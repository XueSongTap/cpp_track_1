最新的下载地址：http://gitlab.0voice.com/liaoqingfu/libzmq-test
# 编译
使用cmake，先进到该目录
cd libzmq-test
mkdir build
cd build
cmake ..
make
在build目录下则可以看到对应.c文件生成的对应的bin执行文件。

# 代码说明
1.REQ/REP模型
hwserver.c 		REP
hwclient.c 		REQ

2.PUB/SUB模型
wuserver.c	 	PUB
wuclient.c   	SUB

3.PUSH/PULL模型
taskwork.c		PULL-PUSH  可以多个work	
taskvent.c		PUSH 
tasksink.c		PULL

4.ROUTER/DEALER 
rrclient.c		REQ
rrbroker.c		ROUTER/DEALER
rrworker.c		REP