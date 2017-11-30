#!/bin/bash
#
# lnidbtask service script
#
# author: Felipe Bergo <fbergo@gmail.com>
# this should be copied as /etc/rc.d/init.d/lnidbtask and installed with
# chkconfig -add lnidbtask
# $Id: lnidbtask.sh,v 1.3 2011/03/10 15:50:41 bergo Exp $
#
# chkconfig: 345 65 35
#
### BEGIN INIT INFO
# Provides: lnidbtask
# processname: lnidbtask.pl
# Required-Start: postgresql
# Default-Start: 3 4 5
# Default-Stop: 0 1 2 3 4 5 6
# Description: lnidbtask is the backend of the LNIDB system and 
#              processes the queue of background tasks.
### END INIT INFO
#

RETVAL=0;
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin

start() {
    echo "Starting lnidbtask"
    if ps axo comm | grep lnidbtask.pl >/dev/null 2>&1 ; then
	exit $RETVAL
    fi
    if lnidbtask.pl -d -s ; then
	echo ""
    else
	$RETVAL = 1
    fi
}

stop() {
    echo "Stopping lnidbtask"
    if ps axo comm | grep ^lnidbtask.pl >/dev/null 2>&1 ; then
	pid=`ps axo comm,pid | grep ^lnidbtask.pl | awk '{ print $2 }'`
	if [ $pid -gt 0 ]; then
	    kill -s INT $pid
	fi
    fi
}

restart() {
    stop
    start
}

case "$1" in
start)
start
;;
stop)
stop
;;
restart)
restart
;;
*)
echo $"Usage: $0 {start|stop|restart}"
exit 1
esac

exit $RETVAL


