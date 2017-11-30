-- branch test 2 (main)

drop sequence user_id cascade;
create sequence user_id start 10;

drop table users cascade;

-- admlevel: -2 disabled, -1=guest (not stored), 0 regular, 1 can import data, 2 can add/remove users
create table users (
       id        integer primary key,
       username  text unique,
       fullname  text,
       password  text,
       created   timestamp,
       lastlogin timestamp,
       admlevel  integer
);

drop table loginlog cascade;
create table loginlog (
       username text,
       time     timestamp,
       ipaddr   inet,
       agent    text,
       success  boolean
);

insert into users values ( nextval('user_id'), 'admin', 'Administrador', 'senha', 'now', 'now', 2 );
-- ids < 10 sao ocultados pela interface web, e nao sao aceitos para login grafico
insert into users values ( 0, 'lnidb', 'Sistema LNIDB', '', 'now', 'now', 0 );

drop sequence task_id cascade;
create sequence task_id;

drop table tasks cascade;

-- state: 0: pending, 1: executing, 2: finished, success, 3: finished, error, 4: pending, do as night job
create table tasks (
       id      integer primary key,
       state   integer,
       message text,
       task    text,
       src     text,
       options text,
       creator integer references users(id),
       started timestamp,
       ended   timestamp
);

drop sequence file_id cascade;
create sequence file_id;

-- for study files only, not attachments
-- compressed, csize: new fields added in dec 2014, for gzip compression
drop table files cascade;
create table files (
       id          integer primary key,
       path        text,
       size        bigint,
       creator     integer references users(id),
       created     timestamp,
       md5         text,
       lastdl	   timestamp,
       dlcount	   integer,
       compressed  boolean default false,
       csize       bigint default 0
);

drop index file_created_index cascade;
create index file_created_index on files(created);
drop index file_path_index cascade;
create index file_path_index on files(path);

drop sequence scanner_id cascade;
create sequence scanner_id;

drop table scanners cascade;
create table scanners (
       id    integer primary key,
       maker text,
       model text,
       location text,
       unique(maker,model,location)
);

drop sequence study_id cascade;
create sequence study_id;

-- artifact: 0 unknown, 1 none, 2 minor, 3 major
drop table studies cascade;
create table studies (
       id               integer primary key,
       scantime         timestamp,
       dicom_studyid    text,
       exam_desc        text,
       series_desc      text,
       scanner          integer references scanners(id),
       modality         text,
       cols             integer,
       rows             integer,
       slices           integer,
       bytes            bigint,       
       files            integer,
       thickness        real,
       xspacing         real,
       yspacing         real,
       zspacing         real,
       series_order     integer,
       storage          text,
       oaxi		integer,
       osag		integer,
       ocor		integer,
       artifact		integer default 0
);

drop index study_scantime_index cascade;
create index study_scantime_index on studies(scantime);
drop index study_dicom_studyid_index cascade;
create index study_dicom_studyid_index on studies(dicom_studyid);

drop sequence attach_id cascade;
create sequence attach_id;

drop table attachments cascade;
create table attachments (
       id          integer primary key,
       description text,
       path        text,
       bytes       bigint,
       creator	   integer references users(id),
       created	   timestamp,       
       md5	   text,
       lastdl	   timestamp,
       dlcount	   integer
);

drop sequence patient_id cascade;
create sequence patient_id;

drop table patients cascade;
create table patients (
       id       integer primary key,
       name     text,
       patcode  text,
       birth    date,
       age	integer,
       gender	text
);

-- new: period cache
drop table patperiod cascade;
create table patperiod (
       pid   integer references patients(id),
       first timestamp,
       last  timestamp,
       primary key (pid)
);

drop index patients_name;
create index patients_name on patients(name);
drop index patients_patcode;
create index patients_patcode on patients(patcode);

drop table patscanners cascade;
create table patscanners (
       pid   integer references patients(id),
       sid   integer references scanners(id),
       primary key (pid,sid)
);

drop table patstudies cascade;
create table patstudies (
       pid integer references patients(id),
       sid integer references studies(id),
       primary key (pid,sid)
);

drop table patattachs cascade;
create table patattachs (
       pid integer references patients(id),
       aid integer references attachments(id),
       primary key (pid,aid)
);

drop table filestudies cascade;
create table filestudies (
       fid integer references files(id),
       sid integer references studies(id),
       primary key (fid,sid)
);

drop index filestudies_sid cascade;
create index filestudies_sid on filestudies(sid);

drop sequence comment_id cascade;
create sequence comment_id;

-- comments in patients

drop table comments cascade;
drop table cvisible cascade;

create table comments (
       id       integer primary key,
       pid      integer references patients(id),
       text     text,
       owner    integer references users(id),
       public   integer,
       created  timestamp
);

create table cvisible (
       cid   integer references comments(id),
       uid   integer references users(id),
       primary key (cid,uid)
);

-- user file basket, for study files
drop table basket cascade;
create table basket (
       uid   integer references users(id),
       fid   integer references files(id),
       primary key(uid,fid)
);

-- user file basket, for attachments
drop table basket2 cascade;
create table basket2 (
       uid   integer references users(id),
       aid   integer references attachments(id),
       primary key(uid,aid)
);

-- finished baskets (tid is the task id that generated it)
drop table wrapped cascade;
create table wrapped (
       tid      integer primary key,
       owner    integer references users(id),
       path     text,
       size     bigint,
       wraptime timestamp       
);

drop table wraptmp cascade;
create table wraptmp (
       tid   integer,
       fid   integer references files(id),
       primary key(tid,fid)
);

drop table wraptmp2 cascade;
create table wraptmp2 (
       tid   integer,
       aid   integer references attachments(id),
       primary key(tid,aid)
);

drop table editlog cascade;
create table editlog (
       uid     integer references users(id),
       edited  timestamp,
       message text,
       primary key(uid,edited)
);

drop table statcache cascade;
create table statcache (
       d1 text,
       d2 text,
       what integer,
       val bigint,
       primary key(d1,d2,what)
);

drop sequence group_id cascade;
create sequence group_id;

-- public: 0: private, 1: all can view, owner edits, 2: all can view/edit
drop table groups cascade;
create table groups (
       id      integer primary key,
       title       text,
       description text,
       public  integer,
       owner   integer references users(id),
       created timestamp       
);

drop table gvisible;
create table gvisible (
       gid   integer references groups(id),
       uid   integer references users(id),
       change integer,
       primary key (gid,uid)
);

drop table groupstudies;
create table groupstudies (
       gid   integer references groups(id),
       sid   integer references studies(id),
       primary key (gid,sid)
);
