/*
 * @Descripttion: 测试rapidjson的性能
 * @version: 1.0
 * @Author: Darren
 * @Date: 2019-10-30 00:29:10
 * @LastEditors: Darren
 * @LastEditTime: 2019-10-30 16:16:07
 */

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
#define TEST_COUNT 100000

static uint64_t getNowTime()
{
    struct timeval tval;
    uint64_t nowTime;

    gettimeofday(&tval, NULL);

    nowTime = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return nowTime;
}

void example()
{
    // 1. 把 JSON 解析至 DOM。
    const char *json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. 利用 DOM 作出修改。
    Value &s = d["stars"];
    s.SetInt(s.GetInt() + 1);

    // 3. 把 DOM 转换（stringify）成 JSON。
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;
}

/**
 * @brief: 序列化
 * @param count 只是用来在循环测试的时候确认for循环是正常执行
 * @return: 返回说明
 */
bool RapidjsonEncode(std::string &strJson, uint32_t count)
{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);

    writer.StartObject();

    writer.Key("name");
    writer.String("darren");
    writer.Key("age");
    writer.Int(80);

    writer.Key("languages");
    writer.StartArray();
    writer.String("C++");
    writer.String("Java");
    writer.EndArray();

    writer.Key("phone");
    writer.StartObject();
    writer.Key("number");
    writer.String("18570368134");
    writer.Key("type");
    writer.String("home");
    writer.EndObject();

    writer.Key("books");
    writer.StartArray();
    writer.StartObject();
    writer.Key("name");
    writer.String("Linux kernel development");
    writer.Key("price");
    writer.Double(7.7);
    writer.EndObject();
    writer.StartObject();
    writer.Key("name");
    writer.String("Linux server development");
    writer.Key("price");
    writer.Double(8.0);
    writer.EndObject();
    writer.EndArray();

    writer.Key("vip");
    writer.Bool(true);
    writer.Key("address");
    writer.String("yageguoji");
    
    writer.EndObject();
    
    const char *json_content = buf.GetString();
    strJson.clear();
    strJson = json_content;

    if(count == 0 || count == (TEST_COUNT - 1))
        std::cout << "1 size: " << strJson.size() << ", " << strJson << std::endl;

    return true;
}

void printRapidjson(Document &d)
{
    if(d.HasMember("name"))
    {
        std::cout << "name: " << d["name"].GetString() << std::endl;
    }

    if(d.HasMember("age"))
    {
        std::cout << "age: " << d["age"].GetInt() << std::endl;
    }

    if(d.HasMember("languages"))
    {
        Value &m = d["languages"];
        if(m.IsArray())
        {
            std::cout << "languages: ";
            for(uint32_t i = 0; i < m.Size(); i++){
				Value &e = m[i];
				if (i != 0)
                {
                    std::cout << ", ";
                }
				std::cout << e.GetString();
			}
            std::cout << std::endl;
        }
    }
    
    if(d.HasMember("phone"))
    {
        Value &m = d["phone"];
        std::cout << "phone number: " << m["number"].GetString() \
            << ", type: " << m["type"].GetString() << std::endl;
    }

    if(d.HasMember("books"))
    {
        Value &m = d["books"];
        if(m.IsArray())
        {
            std::cout << "books: \n";
            for(uint32_t i = 0; i < m.Size(); i++){
                Value &e = m[i];
				std::cout << "book name: " << e["name"].GetString() \
                    << ", type: " << e["price"].GetFloat() << std::endl;
			}
        }
    }

    if(d.HasMember("vip"))
    {
        std::cout << "vip: " << d["vip"].GetBool() << std::endl;
    }

    if(d.HasMember("address"))
    {
        std::cout << "address: " << d["address"].GetString() << std::endl;
    }
}
bool RapidjsonDecode(std::string strJson)
{
    Document d;
	if(d.Parse(strJson.c_str(), strJson.size()).HasParseError()){
		std::cout << "parse error!\n";
		return false;
	}
    if(!d.IsObject()){
		std::cout << "should be an object!\n";
		return false;
	}

    // printRapidjson(d);
    
    return true;
}

int main()
{
    std::string strJson;
    RapidjsonEncode(strJson, 0);
    
    uint64_t startTime;
    uint64_t nowTime;
    startTime = getNowTime();
    printf("rapidjson encode time testing\n");
    for (int i = 0; i < TEST_COUNT; i++)
    {
        RapidjsonEncode(strJson, i);
    }
    nowTime = getNowTime();
    printf("rapidjson encode %u time, need time: %lums\n", TEST_COUNT, nowTime - startTime);

    startTime = getNowTime();
    printf("rapidjson decode time testing\n");
    for (int i = 0; i < TEST_COUNT; i++)
    {
        RapidjsonDecode(strJson);
    }
    nowTime = getNowTime();
    printf("rapidjson decode %u time, need time: %lums\n", TEST_COUNT, nowTime - startTime);

    return 0;
}