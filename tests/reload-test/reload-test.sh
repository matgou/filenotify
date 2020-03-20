#!/bin/bash
cd $( dirname $0 ) 
config=$( basename $0 ).config
pidf=/tmp/$( dirname $0 ).pid
global_rc=0
plg_ext=$( ls ../../src/plugins/.libs/*notify_log*dll ../../src/plugins/.libs/*notify_log*so 2>/dev/null | head -1  | sed "s/.*notify_log//" )

mkdir /tmp/test-$( basename $0 )-1
mkdir /tmp/test-$( basename $0 )-2
mkdir /tmp/test-$( basename $0 )-3

cat > ${config} << EOF
logfile=/tmp/$( basename $0 ).log
loglevel=DEBUG

watch_directory.0=/tmp/test-$( basename $0 )-1
watch_directory.1=/tmp/test-$( basename $0 )-2

plugins_dir=../../src/plugins/.libs/
plugins.log=libplg_notify_log$plg_ext
EOF

../../src/filenotify -c ${config} -i $pidf -d
sleep 1
pid=$( cat $pidf )

sleep 1
echo "hello world" > /tmp/test-$( basename $0 )-1/test1-hello
echo "hello world" > /tmp/test-$( basename $0 )-2/test2-hello

sleep 1

grep -q test1-hello /tmp/$( basename $0 ).log
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
	echo "ERROR : ne trouve pas test1-hello dans le fichier de log."
	global_rc=1
fi

grep -q test2-hello /tmp/$( basename $0 ).log
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
	echo "ERROR : ne trouve pas test2-hello dans le fichier de log."
	global_rc=1
fi

rm -v /tmp/test-$( basename $0 )-1/* /tmp/test-$( basename $0 )-2/*
sleep 3

# Ajout d'un plugin au fichier de conf et reload
echo "plugins.exec=libplg_notify_exec$plg_ext" >> ${config}
echo "exec.cmd=echo __coucou__ %s %s %s >> /tmp/output-$( basename $0 ).txt" >> ${config}
sleep 1
echo "kill -10 ${pid}"
kill -10 "${pid}"
sleep 1
echo "hello world" > /tmp/test-$( basename $0 )-1/test1-hello
echo "hello world" > /tmp/test-$( basename $0 )-2/test2-hello

sed -i "/^plugins.exec/d" $config
sleep 1
kill -10 "${pid}"
sleep 1
echo "hello world" > /tmp/test-$( basename $0 )-1/test1-hello
echo "hello world" > /tmp/test-$( basename $0 )-2/test2-hello

cat /tmp/output-$( basename $0 ).txt
grep -q __coucou__ /tmp/output-$( basename $0 ).txt
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
        echo "ERROR : ne trouve pas __coucou__ dans le fichier de log."
        global_rc=1
fi
cat $config
echo "watch_directory.3=/tmp/test-$( basename $0 )-3" >> ${config}
sleep 1
kill -10 "${pid}"
sleep 1
echo "hello world" > /tmp/test-$( basename $0 )-3/test3-hello
sleep 1
grep -q test3-hello /tmp/$( basename $0 ).log
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
        echo "ERROR : ne trouve pas test3-hello dans le fichier de log."
        global_rc=1
fi

kill ${pid}
cat /tmp/$( basename $0 ).log
rm -rvf /tmp/$( basename $0 ).log ${config} /tmp/test-$( basename $0 )-1 /tmp/test-$( basename $0 )-2 /tmp/test-$( basename $0 )-3 /tmp/output-$( basename $0 ).txt
exit $global_rc
