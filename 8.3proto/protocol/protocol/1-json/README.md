# json库

git clone https://github.com/DaveGamble/cJSON.git

git clone https://github.com/open-source-parsers/jsoncpp.git

git clone https://gitee.com/mirrors/RapidJSON.git

目前已经将文件集成到测试项目里面，不用再重新编译库文件。

# 性能对比测试
**测试机器:**

处理器：Intel(R) Core(TM) i5-8250U CPU @ 1.60GHz

内存：8G

系统：虚拟机ubuntu 18.04 

## 1 测试10万次序列化

|库|默认|-O1|-O2|序列化后字节 |
|-|-|-|-|-|
|cJSON|488ms|452ms|367ms|297|
|jsoncpp|871ms|709ms|670ms|255|
|rapidjson|701ms|113ms|67ms|239|

## 2 测试10万次反序列化

|库|默认|-O1|-O2|
|-|-|-|-|
|cJSON|284ms|251ms|239ms|
|jsoncpp|786ms|709ms|583ms|
|rapidjson|1288ms|128ms|120ms|

如果开启优化-O1 / -O2 级别, rapidjson提升明显, cjson和jsoncpp提升不明显。
