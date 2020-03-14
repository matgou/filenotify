#!/bin/bash
cd $( dirname $0 ) 
config=$( basename $0 ).config
pid=/tmp/$( basename $0 ).pid
global_rc=0

mkdir /tmp/test-$( basename $0 )-1
mkdir /tmp/test-$( basename $0 )-2

cat > ${config} << EOF
logfile=/tmp/$( basename $0 ).log
loglevel=INFO

watch_directory.0=/tmp/test-$( basename $0 )-1
watch_directory.1=/tmp/test-$( basename $0 )-2

plugins_dir=../../src/plugins/.libs/
plugins.log=libplg_notify_log.so
EOF

../../src/filenotify -c ${config} -i $pid -d

sleep 5

echo "hello world" > /tmp/test-$( basename $0 )-1/test1-hello
echo "hello world" > /tmp/test-$( basename $0 )-2/test2-hello

sleep 5

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


kill $( cat ${pid} )

rm -rvf /tmp/$( basename $0 ).log ${config} /tmp/test-$( basename $0 )-1 /tmp/test-$( basename $0 )-2
exit $global_rc
