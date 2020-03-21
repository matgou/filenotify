#!/bin/bash
cd $( dirname $0 ) 
config=$( basename $0 ).config
pidf=/tmp/$( basename $0 ).pid
global_rc=0
plg_ext=$( ls ../../src/plugins/.libs/*notify_log*dll ../../src/plugins/.libs/*notify_log*so 2>/dev/null | head -1  | sed "s/.*notify_log//" )

mkdir /tmp/test-$( basename $0 )-1
mkdir /tmp/test-$( basename $0 )-2

cat > ${config} << EOF
logfile=/tmp/$( basename $0 ).log
loglevel=DEBUG

watch_directory.0=/tmp/test-$( basename $0 )-1
watch_directory.0.extra_post_data=metadata=test1
watch_directory.1=/tmp/test-$( basename $0 )-2
watch_directory.1.extra_post_data=metadata=test2

plugins_dir=../../src/plugins/.libs/
plugins.log=libplg_notify_log$plg_ext
plugins.exec=libplg_notify_exec$plg_ext

exec.cmd=echo "{{ dirname }},{{ filename }},{{ extra_post_data }},{{ event_type_int }}" >> /tmp/$( basename $0 )-result.csv
EOF

../../src/filenotify -c ${config} -i $pidf -d
sleep 2
pid=$( cat $pidf )

sleep 2
echo "hello world" > /tmp/test-$( basename $0 )-1/test1-hello
echo "hello world" > /tmp/test-$( basename $0 )-2/test2-hello

sleep 2

grep -q "/tmp/test-extraarg-test.sh-1,test1-hello,metadata=test1,1" /tmp/$( basename $0 )-result.csv
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
	echo "ERROR : ne trouve pas '/tmp/test-extraarg-test.sh-1,test1-hello,metadata=test1,1' dans le fichier csv."
	global_rc=1
fi

grep -q "/tmp/test-extraarg-test.sh-2,test2-hello,metadata=test2,1" /tmp/$( basename $0 )-result.csv
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
        echo "ERROR : ne trouve pas /tmp/test-extraarg-test.sh-2,test2-hello,metadata=test2,1 dans le fichier csv."
        global_rc=1
fi


rm /tmp/test-$( basename $0 )-1/test1-hello /tmp/test-$( basename $0 )-2/test2-hello
sleep 2
grep -q "/tmp/test-extraarg-test.sh-1,test1-hello,metadata=test1,0" /tmp/$( basename $0 )-result.csv
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
        echo "ERROR : ne trouve pas /tmp/test-extraarg-test.sh-1,test1-hello,metadata=test1,0 dans le fichier csv."
        global_rc=1
fi

grep -q "/tmp/test-extraarg-test.sh-2,test2-hello,metadata=test2,0" /tmp/$( basename $0 )-result.csv
grep_rc=$?
if [ "$grep_rc" != "0" ]
then
        echo "ERROR : ne trouve pas /tmp/test-extraarg-test.sh-2,test2-hello,metadata=test2,0 dans le fichier csv."
        global_rc=1
fi

kill ${pid}

cat /tmp/$( basename $0 ).log
cat /tmp/$( basename $0 )-result.csv

rm -rvf /tmp/$( basename $0 ).log ${config} /tmp/test-$( basename $0 )-1 /tmp/test-$( basename $0 )-2 /tmp/$( basename $0 )-result.csv
exit $global_rc
