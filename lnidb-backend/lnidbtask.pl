#!/usr/bin/perl

# $Id: lnidbtask.pl,v 1.98 2016/05/11 00:20:58 bergo Exp $

use DBI;
use File::Basename;
use Cwd;
use POSIX;
use Fcntl qw(LOCK_EX LOCK_NB);

sub sqlsafe;
sub ndate;
sub ntime;
sub app_log;
sub md5_file;
sub qesc;

# general task subs
sub task_die;
sub task_status;

# dicom import subs
sub task_import_dicom;
sub task_post_import;
sub task_perldbr;
sub task_populate_dir;
sub task_populate_file;
sub dbr;
sub dicom_hash;
sub compute_orientations;
sub roundcos;

# basket wrapping
sub task_wrap_basket;
sub task_remove_package;
sub wrap_name_transform;
sub task_purge_old_packages;
sub zlink;

# file attachments
sub task_attach_file;
sub task_remove_attach;

sub night_job_time;
sub consider_night_jobs;
sub delayed_compression;
sub delayed_md5_computation;

# db utilities
sub db_nextval;
sub inarray;
sub logged_cmd;

# needs: dcm2nii, nii2mnc, mincreshape, dicom2scn, splitmri, zip, tar, gzip

my $dicom       = "dicom2scn";
my $import_src  = "/home/lnidb/importacao";
my $import_dest = "/home/lnidb2/dicom";
my $attach_src  = "/home/lnidb/anexos";
my $attach_dest = "/home/lnidb2/attach";
my $logmode     = 0; # stdout
my $servicemode = 0; # do not daemonize
my $logfile     = "/home/lnidb2/lnidbtask.log";
my $basket_base = "/home/lnidb2/basket";
my $user        = "lnidb";

my %import_stats = (
    files_total       => 0,
    files_new         => 0,
    files_dup         => 0,
    files_ignored     => 0,
    studies_new       => 0,
    patients_new      => 0
);

# -d : log to file
# -s : service mode (daemonize)

foreach (@ARGV) {
    $logmode=1 if ($_ eq '-d');
    $servicemode=1 if ($_ eq '-s');
}

if ($servicemode) {
    open(SELFLOCK, "<$0") or die("Couldn't open $0: $!\n");
    flock(SELFLOCK, LOCK_EX | LOCK_NB) or die("Aborting: another lnidbtask.pl is already running\n");
    open(STDOUT, "|-", "logger -t lnidbtask.pl") or die("Couldn't open logger output stream: $!\n");
    open(STDERR, ">&STDOUT") or die("Couldn't redirect STDERR to STDOUT: $!\n");
    $| = 1; # Make output line-buffered so it will be flushed to syslog faster
    chdir('/');
    exit if (fork());
    exit if (fork());
    sleep 1 until getppid() == 1;
}

# if system has just booted, give postgresql 3 minutes to come to its senses

if (open(UPTIME,"/proc/uptime")) {
    $_ = <UPTIME>;
    close(UPTIME);
    @_ = split;
    if ($_[0] < 300) {
	app_log("system has just booted up, waiting 3 minutes until postgresql comes to its senses.");
	sleep(180);
    } else {
	app_log("system has been up for some time, immediate startup");
    }
}

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");

# loop principal: processa tarefas pendentes na tabela tasks do db

if ($> == 0) {
    ($login,$pass,$uid,$gid) = getpwnam($user);
    if ($uid) {
	POSIX::setuid($uid);
	POSIX::setgid($gid);
	app_log("setuid to $user ($uid : $gid)");
    }
}

app_log("starting main loop");
my $timeref = time();

while(1) {
    @now = localtime(time());
    if ($now[2] >= 21 || $now[2] < 7) {
	$sql = "select id, task, src, options, creator from tasks where state = 0 or state = 4 order by started limit 1;";
    } else {
	$sql = "select id, task, src, options, creator from tasks where state = 0 order by started limit 1;";
    }
    $q = $db->prepare($sql);
    if (! $q->execute()) {
	app_log("database error: $DBI::errstr, will retry every 5 seconds");
	$db->disconnect;
	do {
	    sleep(3);
	    $db = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
	} until($db);
	$q = $db->prepare($sql);
	app_log("database connection ok");
	next;
    }

    if ($q->rows == 0) {
	if ( time() - $timeref >= 10) {
	    consider_night_jobs();
	    $timeref = time();
	}
	sleep(1);
	next;
    }

    @row = $q->fetchrow_array;
    $q->finish;

    app_log("processing task id=$row[0] task=${row[1]}($row[2],$row[3])");

    if ($row[1] eq 'import-dicom') {
	task_import_dicom($row[0],$row[1],$row[2],$row[3],$row[4]);
	next;
    }
    if ($row[1] eq 'wrap-basket') {
	task_wrap_basket($row[0],$row[1],$row[2],$row[3],$row[4]);
	next;
    }
    if ($row[1] eq 'remove-package') {
	task_remove_package($row[0],$row[1],$row[2],$row[3],$row[4]);
	next;
    }
    if ($row[1] eq 'purge-old-packages') {
	task_purge_old_packages($row[0],$row[1],$row[2],$row[3],$row[4]);
	next;
    }
    if ($row[1] eq 'attach-file') {
	task_attach_file($row[0],$row[1],$row[2],$row[3],$row[4]);
	next;
    }
    if ($row[1] eq 'remove-attach') {
	task_remove_attach($row[0],$row[1],$row[2],$row[3],$row[4]);
	next;
    }

}

sub app_log {
    my ($msg) = @_;

    if ($logmode == 0) {
	print "$msg\n";
    } else {
	open(LOG,">>$logfile") or return false;
	$now = localtime;
	print LOG "[$now] $msg\n";
	close LOG;
    }
    return true;
}

sub night_job_time {
    my @now = localtime(time());
    if ($now[6]==0 || $now[6]==6) {
      # weekends
      return($now[2] >= 17 || $now[2] <= 6);
    } else {
      # weekdays
      return($now[2] >= 23 || $now[2] <= 6);
    }
}

sub consider_night_jobs {
    delayed_compression() if night_job_time();
    delayed_md5_computation() if night_job_time();
    return true;
}

sub delayed_compression {
    my $start = time();

    my $sql = "select count(id) from files where compressed = false;";
    my $q = $db->prepare($sql);
    $q->execute();
    my ($n) = $q->fetchrow_array();
    $q->finish();
    return if ($n == 0);

    app_log("starting delayed_compression (night job)");
    $sql = "select id,path from files where compressed = false order by id;";
    $q = $db->prepare($sql);
    $q->execute();
    $n = $q->rows;
    my $i = 1;
    my $bad = 0;
    while( my @row = $q->fetchrow_array ) {
	app_log("delayed_compression ($i of $n) processing file $row[1]");
	my $src = $row[1];
	my $dest = $src . ".gz";
	unless (-f $src) {
	    app_log("** id=$row[0] : source missing, skipping");
	    $i++; $bad++; next;
	}
	if (-f $dest) {
	    app_log("** id=$row[0] : destination exists, skipping");
	    $i++; $bad++; next;
	}
	my $xsrc = qesc($src);
	my $xdest = qesc($dest);
	system("gzip -cq $xsrc >$xdest 2>/dev/null");
	if ($? >> 8 != 0) {
	    app_log("** id=$row[0] : gzip failed");
	    unlink($dest) if (-f $dest);
	    $i++; $bad++; next;
	}
	system("gzip -tq $xdest 2>/dev/null");
	if ($? >> 8 != 0) {
	    app_log("** id=$row[0] : invalid gzip output, skipping");
	    unlink($dest) if (-f $dest);
	    $i++; $bad++; next;
	}
	# success: update db
	my $size = (stat($dest))[7];
	$dest = sqlsafe($dest);
	$db->do("update files set path='$dest',compressed=true,csize=$size,md5=null where id=$row[0];");
	# success: remove uncompressed original
	unlink($src);
	$i++;
	if (time() - $start > 2*3600) {
	    app_log("delayed_compression has been working for over 2 hours, taking a break (after $i of $n with $bad failures).");
	    $q->finish();
	    return true;
	}
    }
    $q->finish();
    my $total = time() - $start;
    app_log("delayed_compression finished, $n files processed in $total seconds, $bad failures.");
    return true;
}

sub delayed_md5_computation {
    my $start = time();

    my $sql = "select count(id) from files where md5 is null;";
    my $q = $db->prepare($sql);
    $q->execute();
    my ($n) = $q->fetchrow_array();
    $q->finish();
    return if ($n == 0);

    app_log("starting delayed_md5_computation (night job)");
    $sql = "select id,path from files where md5 is null;";
    $q = $db->prepare($sql);
    $q->execute();
    $n = $q->rows;
    my $i = 1;
    while( my @row = $q->fetchrow_array ) {
	app_log("delayed_md5_computation ($i of $n) processing file $row[1]");
	my $md5 = sqlsafe(md5_file($row[1]));
	$db->do("update files set md5='$md5' where id=$row[0];");
	$i++;
	if (time() - $start > 2*3600) {
	    app_log("delayed_md5_computation has been working for over 2 hours, taking a break (after $i of $n).");
	    $q->finish();
	    return true;
	}
    }
    $q->finish();
    my $total = time() - $start;
    app_log("delayed_md5_computation finished, $n files processed in $total seconds.");
    return true;
}

sub task_remove_package {
    my ($id,$task,$src,$options,$uid) = @_;

    $sql = "update tasks set state=1 where id=$id;";
    $db->do($sql);

    if ($src eq 'all') {
	$sql = "select path from wrapped where owner=$uid;";
	$q = $db->prepare($sql);
	$q->execute();
	while ( @row = $q->fetchrow_array ) {
	    unlink($row[0]);
	}
	$q->finish();
	$sql = "delete from wrapped where owner=$uid;";
	$db->do($sql);
    } else {
	$sql = "select path from wrapped where owner=$uid and tid=$src;";
	$q = $db->prepare($sql);
	$q->execute();
	while ( @row = $q->fetchrow_array ) {
	    unlink($row[0]);
	}
	$q->finish();
	$sql = "delete from wrapped where owner=$uid and tid=$src;";
	$db->do($sql);
    }

    # claim success
    $sql = "update tasks set state=2, ended='now',message='OK' where id=$id;";
    $db->do($sql);
}

sub task_purge_old_packages {
    my ($id,$task,$src,$options,$uid) = @_;

    $sql = "update tasks set state=1 where id=$id;";
    $db->do($sql);
    
    $sql = "select tid,path from wrapped where now()-wraptime > '10 days';";
    $q = $db->prepare($sql);
    $q->execute();
    $n = 0;
    while( @row = $q->fetchrow_array ) {
	unlink($row[1]);
	$n++;
    }
    $q->finish();
    $sql = "delete from wrapped where now()-wraptime > '10 days';";
    $db->do($sql);

    #claim success
    $sql = "update tasks set state=2, ended='now',message='OK, $n pacote(s) removido(s).' where id=$id;";
    $db->do($sql);
}

sub wrap_name_transform {
    my ($name,$datetime,$series,$id) = @_;
    $series =~ s/[ \t\r\n\.]/_/g;
    $series =~ s/[^A-Za-z0-9_]//g;
    $series =~ s/^_+//;
    $series =~ s/_+$//;
    $id = sprintf("%06d",$id);
    $r = "${datetime}_${series}_${id}_$name";
    $r = $1 if ($r =~ /^(.+)\.gz$/i);
    return $r;
}

# decompress if src is gzip file, link otherwise
sub zlink {
    my ($src, $dest) = @_;

    return 0 if (-f $dest || -d $dest || length($dest) == 0); # fail if dest already exists or has empty name
    return 0 unless (-f $src); # fail if src is not a file

    if ($src =~ /\.gz$/i) {
	my $xsrc = qesc($src);
	my $xdest = qesc($dest);
	system("gzip -cdq $xsrc >$xdest 2>/dev/null");
	return($? >> 8 == 0);
    } else {
	return(link($src,$dest));
    }
}

sub task_wrap_basket {
    my ($id,$task,$src,$options,$uid) = @_;

    my @opts = split(/,/,$options);
    my @sid   = ();
    my %sdesc = ();
    my %sfcount = ();
    my @path  = ();
    my %nii   = ();
    my %scn   = ();
    my %hdr   = ();
    my %minc  = ();

    $sql = "update tasks set state=1 where id=$id;";
    $db->do($sql);

    # create file dir
    $dirname = "${src}-${id}";
    $d1 = $basket_base . '/' . $dirname;
    if (!mkdir($d1)) {
	task_die($id,"Erro ao criar diretório temporário $d1");
	return;
    }

    # link all files
    task_status($id,"Referenciando arquivos");
    $sql = "select f.path,to_char(s.scantime,'YYYYMMDD_HH24MI'),s.series_desc,s.id from files f,wraptmp w,filestudies fs,studies s where f.id=w.fid and w.tid=$id and f.id=fs.fid and fs.sid=s.id;";
    $q = $db->prepare($sql);
    $q->execute();

    $sql = "select a.path from attachments a, wraptmp2 w where a.id=w.aid and w.tid=$id;";
    $q2 = $db->prepare($sql);
    $q2->execute();

    if ($q->rows + $q2->rows == 0) {
	task_die($id,"Nenhum arquivo para empacotar.");
	return;
    }
    $duplicates = 0;
    @refs = ();
    while ( @row = $q->fetchrow_array ) {
	if (! -f $row[0]) {
	    unlink @refs;
	    rmdir $d1;
	    $db->do("delete from wraptmp where tid=$id;");
	    $db->do("delete from wraptmp2 where tid=$id;");
	    task_die($id,"Arquivo não encontrado: $row[0]");
	    return;
	}
	$base = basename($row[0]);
	if ($base =~ /^[A-Za-z]/) {
	    $base = wrap_name_transform($base,$row[1],$row[2],$row[3]);
	} elsif ($base =~ /^(.+)\.gz$/i) {
	    $base = $1;
	}
	#app_log("Referenciando [$row[0]] como [${d1}/${base}]");
	if (-f "${d1}/${base}") {
	    $base = $base . "-DUPLICATE_NAME-$duplicate";
	    $duplicate++;
	}
	if (!zlink($row[0], "${d1}/${base}")) {
	    unlink @refs;
	    rmdir $d1;
	    $db->do("delete from wraptmp where tid=$id;");
	    $db->do("delete from wraptmp2 where tid=$id;");
	    task_die($id,"Erro ao referenciar $row[0] <=> ${d1}/${base}");
	    return;
	}
	push @refs, "${d1}/${base}";
	push @sid, $row[3];
	push @path, "${d1}/${base}";
	if (exists $sfcount{$row[3]}) {
	    $sfcount{$row[3]}++;
	} else {
	    $sfcount{$row[3]}=1;
	}
	$sdesc{$row[3]} = $row[2];
    }
    $q->finish();
    while ( @row = $q2->fetchrow_array ) {
	if (! -f $row[0]) {
	    unlink @refs;
	    rmdir $d1;
	    $db->do("delete from wraptmp where tid=$id;");
	    $db->do("delete from wraptmp2 where tid=$id;");
	    task_die($id,"Arquivo não encontrado: $row[0]");
	    return;
	}
	$base = basename($row[0]);
	if (-f "${d1}/${base}") {
		$base = $base . "-DUPLICATE_NAME-$duplicate";
		$duplicate++;
	}
	if (!link($row[0], "${d1}/${base}")) {
	    unlink @refs;
	    rmdir $d1;
	    $db->do("delete from wraptmp where tid=$id;");
	    $db->do("delete from wraptmp2 where tid=$id;");
	    task_die($id,"Erro ao referenciar $row[0] <=> ${d1}/${base}");
	    return;
	}
	push @refs, "${d1}/${base}";
    }
    $q2->finish();


    # conversions requested, create directory
    $convdir = "${d1}/conv";
    $convlog = "${d1}/0000-conversion.log";
    $cwd = getcwd();
    if (inarray('nii',@opts) || inarray('hdr',@opts) ||
	inarray('mnc',@opts) || inarray('scn',@opts)) {

	task_status($id,"Convertendo arquivos");
	if (!mkdir($convdir)) {
	    task_die($id,"Erro ao criar diretório temporário $convdir");
	    return;
	}

    }    

    # study id list file
    $sidfile = "${d1}/0001-studylist.txt";
    if (open(SIDFILE,">$sidfile")) {
	print SIDFILE "# Lista de Study IDs neste pacote. Copie a linha abaixo no LNIDB\r\n";
	print SIDFILE "# (Cesta => Inserir Lista de Estudos) para recriar o mesmo conjunto.\r\n";
	my %seen = ();
	my @usid = grep { ! $seen{ $_ }++ } @sid;
	foreach (@usid) {
	    print SIDFILE "$_,";
	}
	print SIDFILE "\r\n";
	close SIDFILE;
    }

    foreach $x (@sid) {

	# nii/niigz
	if (inarray('nii',@opts) && $nii{$x} != 1) {
	    # link all files
	    for($i=0;$i<@path;$i++) {
		#logged_cmd("echo x=$x i=$i sid[i]=$sid[$i] path[i]=$path[$i]",$convlog);
		if ($sid[$i] == $x) {
		    $base = basename($path[$i]);
		    link($path[$i], "${convdir}/${base}");
		}
	    }
	    # run conversion

	    $gz = (inarray('niigz',@opts)) ? 'y' : 'n';
	    chdir($convdir);
	    logged_cmd("dcm2nii -f y -g $gz -n y -v y -d n -i n -p n -e n -r n -x n -o $d1 $base",$convlog);
	    chdir($cwd);
	    # clean up conversion directory
	    system("/bin/rm -rf $convdir/*");
	    $nii{$x} = 1;
	}
	# analyze
	if (inarray('hdr',@opts) && $hdr{$x} != 1) {
	    # link all files
	    for($i=0;$i<@path;$i++) {
		if ($sid[$i] == $x) {
		    $base = basename($path[$i]);
		    link($path[$i], "${convdir}/${base}");
		}
	    }
	    # run conversion
	    chdir($convdir);
	    logged_cmd("dcm2nii -f y -n n -s y -v y -d n -i n -p n -e n -r n -x n -o $d1 $base",$convlog);
	    chdir($cwd);
	    # clean up conversion directory
	    system("/bin/rm -rf $convdir/*");
	    $hdr{$x} = 1;
	}
	# minc (new)
	if (inarray('mnc',@opts) && $minc{$x} != 1) {
	    # link all files
	    for($i=0;$i<@path;$i++) {
		#logged_cmd("echo x=$x i=$i sid[i]=$sid[$i] path[i]=$path[$i]",$convlog);
		if ($sid[$i] == $x) {
		    $base = basename($path[$i]);
		    #logged_cmd("echo ln $path[$i] ${convdir}/${base}",$convlog);
		    link($path[$i], "${convdir}/${base}");
		}
	    }
	    # run conversion
	    chdir($convdir);
	    @convfiles = ();
	    if (opendir(CDIR,$convdir)) {
		@convfiles = readdir(CDIR);
		@convfiles = sort(@convfiles);
		closedir(CDIR);
	    }
	    while (! -f $convfiles[0]) {
		shift @convfiles;
	    }
	    if (@convfiles >= 1) {
		$cf = $convfiles[0];
		if (@convfiles > 1) { 
		    app_log("-- study with multiple files, using $cf for conversion."); 
		}

		logged_cmd("dicom2scn $cf first.scn",$convlog);
		logged_cmd("scn2ana first.scn -o first.hdr",$convlog) if (-f 'first.scn');
		logged_cmd("nii2mnc -sagittal first.hdr ${d1}/${cf}-sag.mnc",$convlog)    if (-f 'first.hdr' && inarray('mncsag',@opts));
		logged_cmd("nii2mnc -coronal first.hdr ${d1}/${cf}-cor.mnc",$convlog)     if (-f 'first.hdr' && inarray('mnccor',@opts));
		logged_cmd("nii2mnc -transverse first.hdr ${d1}/${cf}-axi.mnc",$convlog) if (-f 'first.hdr' && inarray('mncaxi',@opts));
	    }
	    chdir($cwd);
	    # clean up conversion directory
	    system("/bin/rm -rf $convdir/*");
	    $minc{$x} = 1;
	}
	# scn
	if (inarray('scn',@opts) && $scn{$x} != 1) {
	    $ucplexical = 0;
	    # link all files
	    for($i=0;$i<@path;$i++) {
		if ($sid[$i] == $x) {
		    $base = basename($path[$i]);
		    link($path[$i], "${convdir}/${base}");
		    $ucplexical = 1 if ($base =~ /IMG\.IMG/ || $base =~ /s\d{2,3}e\d+L/);
		}
	    }
	    # run conversion
	    chdir($convdir);

	    if (inarray('scnneg=shift',@opts)) {
		$neg = "-n2";
	    } elsif (inarray('scnneg=zero',@opts)) {
		$neg = "-n1";
	    } elsif (inarray('scnneg=keep',@opts)) {
		$neg = "-n0";
	    } else {
		$neg = "-n2";
		$neg = "-n1" if ($sdesc{$x} =~ /^COR T2 multi-eco/);
	    }

	    $ucpopt = '';
	    $ucpopt = "-lex -p" if ($ucplexical);

	    logged_cmd("dicom2scn $neg $ucpopt $base ${d1}/${base}.scn",$convlog);

	    chdir($d1);
	    # regular multi-echo with single DICOM file
	    if (inarray('scnme',@opts) && $sdesc{$x} =~ /^COR T2 multi-eco/ && $sfcount{$x} < 6) {
		logged_cmd("splitmri -p 5,1 -o e ${base}.scn",$convlog);
	    }
	    # multi-echo with one DICOM file per slice, dicom2scn interleaves slices differently
	    if (inarray('scnme',@opts) && $sdesc{$x} =~ /^COR T2 multi-eco/ && $sfcount{$x} >= 6) {
		logged_cmd("splitmri -p 6 -o e ${base}.scn",$convlog);
	    }
	    if (inarray('scndif',@opts) && $sdesc{$x} eq 'DIFUSAO') {
		logged_cmd("splitmri -p 2 -o d ${base}.scn", $convlog);
	    }

	    chdir($cwd);
	    # clean up conversion directory
	    system("/bin/rm -rf $convdir/*");
	    $scn{$x} = 1;
	}
    }

    # remove conversion directory
    if (inarray('nii',@opts) || inarray('hdr',@opts) ||
	inarray('mnc',@opts) || inarray('scn',@opts)) {
	$rv = system("/bin/rm -rf $convdir");
	if ($rv != 0) {
	    if ($rv == -1) {
		task_die($id,"Erro ao limpar diretório de conversão.");
		return;
	    }
	    $rv >>= 8;
	    task_die($id,"Erro ao limpar diretório de conversão.");
	    return;
	}

	# remove original DICOM files if requested
	if (inarray('nodicom',@opts)) {
	    unlink @refs;
	}
    }

    task_status($id,"Compactando arquivos");
    
    if (inarray('zip',@opts))  {
	$ext = "zip";
    } elsif (inarray('targz',@opts)) {
	$ext = "tar.gz";
    }

    $dest = "${basket_base}/${src}-${id}.$ext";

    if (inarray('zip',@opts)) {
	$rv = system("zip -jqr4 $dest ${d1}");
	if ($rv != 0) {
	    unlink @refs;
	    rmdir $d1;
	    $db->do("delete from wraptmp where tid=$id;");
	    $db->do("delete from wraptmp2 where tid=$id;");
	    task_die($id,"Erro ao executar zip");
	    return;
	}
    } elsif (inarray('targz',@opts)) {
	$cwd = getcwd;
	chdir $basket_base;
	$rv = system("tar zcf $dest $dirname");
	chdir $cwd;
	if ($rv != 0) {
	    unlink @refs;
	    rmdir $d1;
	    $db->do("delete from wraptmp where tid=$id;");
	    $db->do("delete from wraptmp2 where tid=$id;");
	    task_die($id,"Erro ao executar tar");
	    return;
	}
    }

    if (! -f $dest) {
	unlink @refs;
	rmdir $d1;
	$db->do("delete from wraptmp where tid=$id;");
	$db->do("delete from wraptmp2 where tid=$id;");
	task_die($id,"Erro inesperado, arquivo destino $dest não escontrado.");
	return;
    }

    task_status($id,"Removendo arquivos temporários");
    unlink @refs;

    if (opendir(RDIR,$d1)) {
	@rfiles = readdir(RDIR);
	closedir(RDIR);
	foreach $r (@rfiles) {
	    unless ($r =~ /^\./) {
		app_log("Removing conversion left-over ${d1}/$r");
		unlink("${d1}/$r");
	    } 
	}
    }

    rmdir $d1 || app_log("could not remove directory $d1");

    task_status($id,"Atualizando banco de dados");

    $sql = "update files f set dlcount = dlcount + 1, lastdl = 'now' from wraptmp w where f.id=w.fid and w.tid=$id;";
    $db->do($sql);
    $sql = "update attachments a set dlcount = dlcount + 1, lastdl = 'now' from wraptmp2 w where a.id=w.aid and w.tid=$id;";
    $db->do($sql);

    $sql = "delete from wraptmp where tid=$id;";
    $db->do($sql);
    $sql = "delete from wraptmp2 where tid=$id;";
    $db->do($sql);

    $size = (stat($dest))[7];    
    $sql = "insert into wrapped (tid,owner,path,size,wraptime) values ($id,$uid,'$dest',$size,'now');";
    $db->do($sql);

    # claim success 
    $sql = "update tasks set state=2, ended='now',message='Pacote disponível.' where id=$id;";
    $db->do($sql);
}

sub task_remove_attach {
    my ($id,$task,$src,$options,$uid) = @_;

    $sql = "update tasks set state=1 where id=$id;";
    $db->do($sql);

    $n = 0;
    @aids = split(/\s+/,$src);
    foreach $aid (@aids) {
	$sql = "select path from attachments where id=$aid;";
	$q = $db->prepare($sql);
        $q->execute();
	@row = $q->fetchrow_array;
	unlink($row[0]);
	$q->finish;
	$sql = "delete from wraptmp2 where aid=$aid; delete from basket2 where aid=$aid; delete from patattachs where aid=$aid; delete from attachments where id=$aid;";
	$db->do($sql);
	$n++;
    }

    # claim success 
    $sql = "update tasks set state=2, ended='now',message='$n anexo(s) removido(s).' where id=$id;";
    $db->do($sql);
}

sub task_attach_file {
    my ($id,$task,$src,$options,$uid) = @_;

    $sql = "update tasks set state=1 where id=$id;";
    $db->do($sql);
    
    if ($src =~ /^(\d+),(\S+)$/) {
	$pid  = $1;
	$file = $2;
	$text = $options;
    } else {
	task_die($id,"Sintaxe incorreta ou nome de arquivo inválido.");
	return;
    }

    task_die($id,"Arquivo fonte não encontrado.") unless (-f "$attach_src/$file");
    task_status($id,"Copiando arquivo");

    $aid = db_nextval("attach_id");
    $dest = "$attach_dest/$file";
    if (-f $dest) { $dest = "$attach_dest/$aid_$file"; }
    $rv = system("/bin/cp -f $attach_src/$file $dest");
    if ($rv != 0) {
	if ($rv == -1) {
	    task_die($id,"Erro ao copiar arquivo.");
	    return;
	}
	$rv >>= 8;
	task_die($id,"Erro ao copiar arquivo.");
	return;
    }

    unless (-f $dest) {
	task_die($id,"Erro ao copiar arquivo (2).");
	return;
    }
    unlink("$attach_src/$file");

    task_status($id,"Calculando md5sum");
    $md5 = md5_file($dest);

    task_status($id,"Atualizando Banco de Dados");
    $size = (stat($dest))[7];

    $sql = "insert into attachments (id,description,path,bytes,creator,created,md5,dlcount) values ($aid,'$text','$dest',$size,$uid,'now','$md5',0);";
    $db->do($sql);
    $sql = "insert into patattachs (pid,aid) values ($pid, $aid);";
    $db->do($sql);

    # claim success
    $sql = "update tasks set state=2, ended='now',message='OK' where id=$id;";
    $db->do($sql);
}

sub task_post_import {
    # fix DTI series names
    $sql = "update studies set series_desc='Reg DTI padronizado 1x1x2' where series_desc ~* '^reg.*dti' and series_desc <> 'Reg DTI padronizado 1x1x2' and xspacing=1.0 and yspacing=1.0 and zspacing=2.0 and thickness=2.0 and cols=256 and rows=256 and slices/files=2380;";
    $db->do($sql)
}

sub task_import_dicom {
    my ($id,$task,$src,$options,$uid) = @_;
    @dirs = ();

    %import_stats = (
	files_total       => 0,
	files_new         => 0,
	files_dup         => 0,
	files_ignored     => 0,
	studies_new       => 0,
	patients_new      => 0
    );

    $sql = "update tasks set state=1 where id=$id;";
    $db->do($sql);

    # check if src is a valid dir
    $odir = $import_src . '/' . $src;
    unless (-d $odir && -x $odir) {
	task_die($id,"Diretório inexistente ou com permissões incorretas.") ;
	return;
    }

    $ddir = $import_dest . '/' . $options;
    if (-d $ddir) {
	task_die($id,"Diretório destino já existe (storage repetido).");
	return;
    }
    
    task_status($id,"Copiando arquivos");
    $rv = system("/bin/cp -r $odir $ddir");
    if ($rv != 0) {
	if ($rv == -1) {
	    task_die($id,"Erro ao copiar diretório.");
	    return;
	}
	$rv >>= 8;
	task_die($id,"Erro ao copiar diretório.");
	return;
    }

    task_status($id,"Renomeando arquivos");
    chmod 0755, $ddir;
    return unless (task_perldbr($id,$ddir));

    # import ddir to files, studies and patients tables
    task_status($id,"Atualizando banco de dados");
    return unless (task_populate_dir($id,$ddir,$options,$uid));
    app_log("import-dicom: $import_stats{files_total} files (N/D/I: $import_stats{files_new}/$import_stats{files_dup}/$import_stats{files_ignored}), $import_stats{studies_new} new studies, $import_stats{patients_new} patients");

    # remove src dir on success
    task_status($id,"Limpando diretório de origem");
    $rv = system("/bin/chmod -R 0777 $odir");
    if ($rv != 0) {
	if ($rv == -1) {
	    task_die($id,"Erro ao limpar diretório, passo 1 (importação provavelmente ok).");
	    return;
	}
	$rv >>= 8;
	task_die($id,"Erro ao limpar diretório, passo 1 (importação provavelmente ok).");
	return;
    }
    $rv = system("/bin/rm -rf $odir");
    if ($rv != 0) {
	if ($rv == -1) {
	    task_die($id,"Erro ao limpar diretório, passo 2 (importação provavelmente ok).");
	    return;
	}
	$rv >>= 8;
	task_die($id,"Erro ao limpar diretório, passo 2 (importação provavelmente ok).");
	return;
    }

    # post-import (nomenclature changes)
    task_post_import();

    # claim success
    $sql = "update tasks set state=2, ended='now',message='$import_stats{files_total} arquivos (novos/duplicados/ignorados: $import_stats{files_new}/$import_stats{files_dup}/$import_stats{files_ignored}), $import_stats{studies_new} novos estudos, $import_stats{patients_new} novos pacientes.' where id=$id;";
    $db->do($sql);

}

sub task_populate_dir {
    my ($id,$ddir,$storage,$uid) = @_;

    #app_log("PARANOID task_populate_dir: ($id,$storage) $ddir");

    unless(opendir(DDIR,$ddir)) {
	task_die($id,"Erro ao acessar diretório copiado.");
	return false;
    }    
    my @files = readdir(DDIR);
    @files = sort(@files);
    closedir(DDIR);

    #app_log("PARANOID subdirs = ( @files )");

    foreach $i (@files) {
	next if ($i eq '.' || $i eq '..');
	my $full = $ddir . '/' . $i;
	#app_log("PARANOID considering $full");
	if (-f $full) {
	    unless (task_populate_file($id,$full,$storage,$uid)) {
		task_die($id,"Erro ao renomear arquivos.");
		return false;
	    }
	} elsif (-d $full) {
	    return false unless(task_populate_dir($id,$full,$storage,$uid));
	} 
    }

    #app_log("PARANOID tpd $id,$storage,$dir ok");
    return true;    
}

sub db_nextval {
    my ($seq) = @_;
    $t = $db->prepare("select nextval('$seq');");
    $t->execute();
    ($val) = $t->fetchrow_array;
    $t->finish();
    return $val;
}

sub task_populate_file {
    my ($id,$path,$storage,$uid) = @_;
    $base = basename($path);

    return false unless (-f $path);
    #app_log("PARANOID task_populate_file: ($id,$storage) $path");
    $import_stats{files_total}++;

    $ref = dicom_hash($path);
    %dicom = %$ref;
    
    $rec{'patient'}       = $dicom{'0010:0010'};
    $rec{'patientid'}     = $dicom{'0010:0020'};
    $rec{'birth'}         = ndate($dicom{'0010:0030'});
    $rec{'scandate'}      = ndate($dicom{'0008:0020'});
    $rec{'scantime'}      = ntime($dicom{'0008:0030'});
    $rec{'gender'}        = $dicom{'0010:0040'};
    $rec{'studyid'}       = $dicom{'0020:000e'};
    $rec{'exam'}          = $dicom{'0008:1030'};
    $rec{'series'}        = $dicom{'0008:103e'};
    $rec{'series'}        = $dicom{'0018:1030'} unless $rec{'series'};
    $rec{'model'}         = $dicom{'0008:1090'};
    $rec{'maker'}         = $dicom{'0008:0070'};
    $rec{'location'}      = $dicom{'0008:0080'};
    $rec{'modality'}      = $dicom{'0008:0060'};
    $rec{'cols'}          = $dicom{'0028:0011'};
    $rec{'rows'}          = $dicom{'0028:0010'};
    $rec{'mfnum'}         = $dicom{'0028:0008'};
    $rec{'mfnum'}         = 1 unless $rec{'mfnum'};
    $rec{'bytes'}         = (stat($path))[7];
    $rec{'thickness'}     = $dicom{'0018:0050'};
    ($rec{'yspacing'},$rec{'xspacing'}) = split(/\\+/,$dicom{'0028:0030'});
    $rec{'zspacing'}      = $dicom{'0018:0088'};
    $rec{'series_order'}  = $dicom{'0020:0012'};

    if (length($rec{'patient'}) == 0 || $rec{'studyid'} == 0) {
	app_log("Ignored: File $base is not DICOM.");
	$import_stats{files_ignored}++;
	return true;
    }
    if (length($rec{'cols'}) == 0) {
	app_log("Ignored: File $base is DICOM, but unlikely to be an image file (columns field (0028:0011) missing).");
	$import_stats{files_ignored}++;
	return true;
    }

    @orient = compute_orientations($ref);

    # tag empty fields
    $rec{'cols'} = -1 unless $rec{'cols'};
    $rec{'rows'} = -1 unless $rec{'rows'};
    $rec{'thickness'} = -1 unless $rec{'thickness'};
    $rec{'xspacing'}  = -1 unless $rec{'xspacing'};
    $rec{'yspacing'}  = -1 unless $rec{'yspacing'};
    $rec{'series_order'}  = -1 unless $rec{'series_order'};

    # elscint files, mostly
    if ($rec{'zspacing'} eq '') {
	app_log("Warning: missing zspacing in $base, marking with -1 for now.");
	$rec{'zspacing'} = -1;
    }

    # compute md5
    #$rec{'md5'} = md5_file($path);

    # compute age
    @d1 = split(/-/,$rec{'birth'});
    @d2 = split(/-/,$rec{'scandate'});
    $rec{'age'} = $d2[0] - $d1[0] - 1;
    $rec{'age'}++ if ($d2[1] > $d1[1] || ($d2[1] == $d1[1] && $d2[2] > $d1[2]));

    # additional fields
    $rec{'storage'} = $storage;
    $rec{'path'}    = $path;
    
    foreach (keys %rec) {
        $rec{$_} = sqlsafe($rec{$_});
    }

    # check if there isn't an exact duplicate
    $q = $db->prepare("select id from files where path='$rec{'path'}';");
    $q->execute();
    if ($q->rows > 0) {
	app_log("Ignored: File $base is already in the database (same full path)");
	$import_stats{files_ignored}++;
	return true;
    }
    $q->finish();

    # check if the study is a duplicate from other storage, ignore if so
    $q = $db->prepare("select id,storage from studies where dicom_studyid='$rec{'studyid'}' and storage <> '$rec{'storage'}';");
    $q->execute();
    if ($q->rows > 0) {
	@dup = ();
	while ( @row = $q->fetchrow_array ) {
	    push @dup, $row[1];
	}
	app_log("Ignored: File $base belongs to a study duplicated in storage(s) [ @dup ], will not add to $rec{'storage'}");
	$import_stats{files_dup}++;
	return true;
    }
    $q->finish();

    # allocate file_id
    $fileid = db_nextval('file_id');

    # insert item in files table
    $sql  = "insert into files (id,path,size,creator,created,md5,dlcount,compressed,csize) values ($fileid,'$rec{'path'}',";
    $sql .= "$rec{'bytes'}, $uid, 'now', NULL, 0, false, 0);";
    $rows = $db->do($sql);
    if ($rows != 1) {
	app_log("Fatal error: SQL statement failed: [$sql]");
	$import_stats{files_ignored}++;
	return false;
    }
    $import_stats{files_new}++;

    # study exists ?
    $q = $db->prepare("select id from studies where dicom_studyid='$rec{'studyid'}';");
    $q->execute();
    
    if ($q->rows == 0) {
	$q->finish();

	# find scanner, allocate+insert if needed
	$q = $db->prepare("select id from scanners where maker='$rec{'maker'}' and model='$rec{'model'}' and location='$rec{'location'}';");
	$q->execute();
	if ($q->rows == 0) {
	    $scannerid = db_nextval('scanner_id');
	    $rows = $db->do("insert into scanners (id,maker,model,location) values ($scannerid,'$rec{'maker'}','$rec{'model'}','$rec{'location'}');");
	    if ($rows != 1) {
		app_log("Fatal error: SQL statement failed: [$sql]");
		return false;
	    }
	} else {
	    ($scannerid) = $q->fetchrow_array;
	}
	$q->finish();

	# new study
	# allocate id
	$studyid = db_nextval('study_id');

	$sql  = "insert into studies (id,scantime,dicom_studyid,exam_desc,series_desc,scanner,";
	$sql .= "modality,cols,rows,slices,bytes,files,thickness,xspacing,";
	$sql .= "yspacing,zspacing,series_order,storage,oaxi,osag,ocor) values ($studyid,'$rec{'scandate'} $rec{'scantime'}',";
	$sql .= "'$rec{'studyid'}','$rec{'exam'}','$rec{'series'}',$scannerid,";
	$sql .= "'$rec{'modality'}',$rec{'cols'},$rec{'rows'},$rec{'mfnum'},";
	$sql .= "$rec{'bytes'},1,$rec{'thickness'},$rec{'xspacing'},$rec{'yspacing'},$rec{'zspacing'},";
	$sql .= "$rec{'series_order'},'$rec{'storage'}',$orient[0],$orient[1],$orient[2]);";
	$rows = $db->do($sql);
	if ($rows != 1) {
	    app_log("Fatal error: SQL statement failed: [$sql]");
	    return false;
	}
	$import_stats{studies_new}++;

    } else {
	# existing study
	($studyid) = $q->fetchrow_array;
	$q->finish();
	my $oc = '';
	if ($orient[0]) { $oc .= ", oaxi=1"; } 
	if ($orient[1]) { $oc .= ", osag=1"; } 
	if ($orient[2]) { $oc .= ", ocor=1"; } 
	$sql = "update studies set files=files+1, bytes=bytes+$rec{'bytes'}, slices = slices + $rec{'mfnum'} $oc where id=$studyid;";
	$rows = $db->do($sql);
	if ($rows != 1) {
	    app_log("Fatal error: SQL statement failed: [$sql]");
	    return false;
	}
    }

    # relate file/study
    $sql = "insert into filestudies (fid,sid) values ($fileid,$studyid);";
    $rows = $db->do($sql);
    if ($rows != 1) {
	app_log("Fatal error: SQL statement failed: [$sql]");
	return false;
    }

    # new patient ?
    $sql  = "select id from patients where name='$rec{'patient'}' and patcode='$rec{'patientid'}' and ";
    $sql .= "age = $rec{'age'} and gender = '$rec{'gender'}';";
    $q = $db->prepare($sql);
    $q->execute();

    if ($q->rows == 0) {
	# new patient
	$q->finish();
	# allocate id
	$patientid = db_nextval('patient_id');

	$sql  = "insert into patients (id,name,patcode,birth,age,gender) values (";
	$sql .= "$patientid,'$rec{'patient'}','$rec{'patientid'}','$rec{'birth'}',$rec{'age'},";
	$sql .= "'$rec{'gender'}');";
	$rows = $db->do($sql);
	if ($rows != 1) {
	    app_log("Fatal error: SQL statement failed: [$sql]");
	    return false;
	}
	$import_stats{patients_new}++;
    } else {
	# existing patient
	($patientid) = $q->fetchrow_array;
	$q->finish();	
    }

    # relate patient/study 
    $rows = $db->do("select pid,sid from patstudies where pid=$patientid and sid=$studyid;");
    if ($rows == 0) {
	$rows = $db->do("insert into patstudies (pid,sid) values ($patientid,$studyid);");
	if ($rows != 1) {
	    app_log("Fatal error: SQL statement failed: [$sql]");
	    return false;
	}
    }

    # clear patperiod cache for this patient
    $db->do("delete from patperiod where pid=$patientid;");

    # add patient/scanner pair if absent
    $rows = $db->do("select pid from patscanners where pid=$patientid and sid=$scannerid;");
    if ($rows == 0) {
	$r2 = $db->do("insert into patscanners (pid,sid) values ($patientid, $scannerid);");
	if ($r2 != 1) {
	    app_log("Fatal error: SQL statement failed: [$sql]");
	    return false;
	}
    }
    
    return true;
}

sub task_perldbr {
    my ($id,$ddir) = @_;

    unless(opendir(DDIR,$ddir)) {
	task_die($id,"Erro ao acessar diretório copiado.");
	return false;
    }    
    my @files = readdir(DDIR);
    @files = sort(@files);
    closedir(DDIR);

    foreach $i (@files) {
	next if ($i eq '.' || $i eq '..');
	my $full = $ddir . '/' . $i;
	if (-f $full) {
	    chmod 0644, $full;
	    unless (dbr($full)) {
		task_die($id,"Erro ao renomear arquivos.");
		return false;
	    }
	} elsif (-d $full) {
	    chmod 0755, $full;
	    return false unless(task_perldbr($id,$full));
	} 
    }

    return true;
}

sub task_status {
    my ($id,$msg) = @_;
    $msg = sqlsafe($msg);
    $sql = "update tasks set message='$msg' where id=$id;";
    $db->do($sql);
}

sub task_die {
    my ($id, $msg) = @_;
    $msg = sqlsafe($msg);
    $sql = "update tasks set state=3, ended='now', message='$msg' where id=$id;";
    $db->do($sql);
    app_log("Task failed with message: [$msg]");
}

sub md5_file {
    my ($file) = @_;
    $x = '';
    if (open(MD5,"md5sum $file|")) {
	$_ = <MD5>;
	$x = $1 if (/^([0-9a-f]+)/);
	close MD5;
    }
    return $x;
}

sub roundcos {
    my ($x) = @_;
    my $pi = 3.1415926535;
    if (abs($x) < cos(45.5 * $pi / 180.0)) { return 0; }
    if (abs($x) > cos(44.5 * $pi / 180.0)) {
	if ($x < 0.0) { return -1; } else { return 1; }
    }
    return 2;
}

sub compute_orientations {
    my ($ref) = @_;
    my %ldicom = %$ref;
    my $i;
    my ($axi,$sag,$cor) = (0,0,0);
    my $x = '';

    foreach (keys %ldicom) {
	if (/^0020:0037/) {
	    my @val = split( /\\/, $ldicom{$_} );
	    next if (@val != 6);
	    my @rval;
	    for($i=0;$i<6;$i++) {
		$rval[$i] = roundcos($val[$i]);
	    }
	    $axi = 1 if ($rval[0]==0 && $rval[2]==0 && $rval[4]==0 && $rval[5]==0); # ap/rl
	    $axi = 1 if ($rval[1]==0 && $rval[2]==0 && $rval[3]==0 && $rval[5]==0); # rl/ap
	    $sag = 1 if ($rval[0]==0 && $rval[2]==0 && $rval[3]==0 && $rval[4]==0); # ap/fh
	    $sag = 1 if ($rval[0]==0 && $rval[1]==0 && $rval[3]==0 && $rval[5]==0); # fh/ap	    
	    $cor = 1 if ($rval[0]==0 && $rval[1]==0 && $rval[4]==0 && $rval[5]==0); # fh/rl
	    $cor = 1 if ($rval[1]==0 && $rval[2]==0 && $rval[3]==0 && $rval[4]==0); # rl/fh
	}
    }

    return($axi,$sag,$cor);
}

sub dicom_hash {
    my ($file) = @_;
    my %dicom = ();
    open(DCM,"$dicom -d \"$file\"|") or return {};
    while($_ = <DCM>) {
        if (/^(\w{4}:\w{4})\s+\(\d+\)\s+\[(.*?)\s*\]/) {	    
	    my $k = $1;
	    my $kk = $k;
	    my $n = 0;
	    my $val = $2;
	    next if ($dicom{$k} && $k ne '0020:0037');
	    while($dicom{$kk}) {
		$n++;
		$kk = "$k$n";
	    }
            $dicom{$kk} = $val;
        }
    }
    close(DCM);
    return \%dicom;
}

sub sqlsafe {
    my ($x) = @_;
    $x =~ s/\n//g;
    $x =~ s/\r//g;
    $x =~ s/\'/\'\'/g;
    return $x;
}

sub ndate {
    my ($x) = @_;    
    if ($x =~ /^(\d{4})(\d{2})(\d{2})/) { # philips
        $x = $1 .'-' . $2 . '-' . $3;
        return $x;
    } elsif ($x =~ /^(\d{4})\.(\d{2})\.(\d{2})/) { # elscint
        $x = $1 .'-' . $2 . '-' . $3;
        return $x;
    } else {
        app_log("Warning: bad date format $x");
        return '1900-01-01';
    }
}

sub ntime {
    my ($x) = @_;
    if ($x =~ /^(\d{2})(\d{2})(\d{2})/) { # philips
        $x = $1 .':' . $2 . ':' . $3;
        return $x;
    } elsif ($x =~ /^(\d{2}):(\d{2}):(\d{2})/) { # elscint
        $x = $1 .':' . $2 . ':' . $3;
        return $x;
    } else {
        app_log("Warning: bad time format $x");
        return '12:00:00';
    }
}

# returns the new file name if successfully renamed or not dicom
# returns false on error
sub dbr {
    my ($item) = @_;
    $dir  = dirname($item);
    $file = basename($item);

    return false unless (-f $item);
    $ref = dicom_hash($item);
    %dicom = %$ref;

    $patient = $dicom{'0010:0010'};
    $seq1    = $dicom{'0008:103e'};    
    $seq1    = $dicom{'0008:1030'} unless $dicom{'0008:103e'};
    $date    = $dicom{'0008:0020'};
    $time    = $dicom{'0008:0030'};
    $order   = $dicom{'0020:0013'};
    $mf      = $dicom{'0028:0008'};

    if (length($patient)==0 || length($seq1)==0 || length($date)==0 || length($time)==0) {
	return $item;
    }

    # canonize
    $date =~ s/[^0-9]//g;
    if (length($date)==8) {
	$date = substr($date,0,4) . '-' . substr($date,4,2) . '-' . substr($date,6,2);
    }

    $time =~ s/[^0-9]//g;
    if (length($time)==6) {
	$time = substr($time,0,4);
    }
    if (length($time)==4) {
	$time = substr($time,0,2) . '-' . substr($time,2,2);
    }

    $patient =~ s/[ \t\r\n\.]/_/g;
    $patient =~ s/[^A-Za-z0-9_]//g;
    $patient =~ s/^_+//;
    $patient =~ s/_+$//;

    $seq1 =~ s/[ \t\r\n\.]/_/g;
    $seq1 =~ s/[^A-Za-z0-9_]//g;
    $seq1 =~ s/^_+//;
    $seq1 =~ s/_+$//;

    $newname = "$date-$time-$patient-$seq1";
    $newname .= "-ENH" if ($mf > 1);

    $pos = '';
    if (length($order) > 0) {
	$pos = sprintf("%04d",$order);
    }

    $dest = $dir . '/' . $newname . '-' . $pos;
    return $item if (basename($dest) eq basename($item));

    $i = 0;
    while (-f $dest && basename($dest) ne basename($item)) {
	$i++;
	$dest = $dir . '/' . "$newname-S$i-$pos";
    }

    return $item if (basename($item) eq basename($dest));

    unless (rename($item,$dest)) {
	return false;
    }

    return $dest;
}

sub inarray {
    my @x = @_;
    $y = shift @x;
    foreach $z (@x) {
	return 1 if ($z eq $y);
    }
    return 0;
}

sub logged_cmd {
    my ($cmd,$log) = @_;
    $now = localtime;
    if (open(CLOG,">>$log")) {
	print CLOG "### [$now] $cmd\n";
	close CLOG;
    }
    system("$cmd >>$log 2>&1");
    if (open(CLOG,">>$log")) {
	print CLOG "### [$now] concluido\n";
	close CLOG;
    }
}

sub qesc {
    my ($s) = @_;
    $s =~ s/'/\\'/g;
    return $s;
}
