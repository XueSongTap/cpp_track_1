/*
 * @Descripttion: cjson测试
 * @version: 1.0
 * @Author: Darren
 * @Date: 2019-10-29 10:14:51
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-06-09 20:28:49
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "cJSON.h"

static uint64_t getNowTime()
{
    struct timeval tval;
    uint64_t nowTime;

    gettimeofday(&tval, NULL);

    nowTime = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return nowTime;
}

char *CJsonEncode()
{ // 创建json对象
    cJSON *root = cJSON_CreateObject();
    // 添加键值对
    cJSON_AddItemToObject(root, "name", cJSON_CreateString("darren"));
    cJSON_AddItemToObject(root, "age", cJSON_CreateNumber(80));
    // 创建json数组
    cJSON *languages = cJSON_CreateArray();
    cJSON_AddItemToArray(languages, cJSON_CreateString("C++"));
    cJSON_AddItemToArray(languages, cJSON_CreateString("Java"));
    cJSON_AddItemToObject(root, "languages", languages);

    cJSON *phone = cJSON_CreateObject();
    cJSON_AddItemToObject(phone, "number", cJSON_CreateString("18570368134"));
    cJSON_AddItemToObject(phone, "type", cJSON_CreateString("home"));
    cJSON_AddItemToObject(root, "phone", phone);

    cJSON *book0 = cJSON_CreateObject();
    cJSON_AddItemToObject(book0, "name", cJSON_CreateString("Linux kernel development"));
    cJSON_AddItemToObject(book0, "price", cJSON_CreateNumber(7.7));
    cJSON *book1 = cJSON_CreateObject();
    cJSON_AddItemToObject(book1, "name", cJSON_CreateString("Linux server development"));
    cJSON_AddItemToObject(book1, "price", cJSON_CreateNumber(8.0));
    // 创建json数组
    cJSON *books = cJSON_CreateArray();
    cJSON_AddItemToArray(books, book0);
    cJSON_AddItemToArray(books, book1);
    cJSON_AddItemToObject(root, "books", books);

    cJSON_AddItemToObject(root, "vip", cJSON_CreateBool(1));
    cJSON_AddItemToObject(root, "address", cJSON_CreateString("yageguoji")); // NULL值的问题

    // 格式化json对象
    char *text = cJSON_Print(root);
    // size_t len = strlen(text) + 1;
    // char *strJson = malloc(len);
    // memcpy(strJson, text, len);
    // printf("text:%s\n", text);
    // FILE* fp = fopen("root.json", "w");
    // fwrite(text, 1, strlen(text), fp);
    // fclose(fp);
    cJSON_Delete(root);
    return text;
}

void printCJson(cJSON *root)
{
    cJSON *name = cJSON_GetObjectItem(root, "name");
    cJSON *age = cJSON_GetObjectItem(root, "age");
    cJSON *languages = cJSON_GetObjectItem(root, "languages");
    cJSON *phone = cJSON_GetObjectItem(root, "phone");
    cJSON *books = cJSON_GetObjectItem(root, "books");
    cJSON *vip = cJSON_GetObjectItem(root, "vip");
    cJSON *address = cJSON_GetObjectItem(root, "address");

    printf("name: %s\n", name->valuestring);
    printf("age: %d\n", age->valueint);
    printf("languages: ");
    for (int i = 0; i < cJSON_GetArraySize(languages); i++)
    {
        cJSON *lang = cJSON_GetArrayItem(languages, i);
        if (i != 0)
        {
            printf(", ");
        }
        printf("%s", lang->valuestring);
    }
    printf("\n");

    cJSON *number = cJSON_GetObjectItem(phone, "number");
    cJSON *type = cJSON_GetObjectItem(phone, "type");
    printf("phone number: %s, type: %s\n", number->valuestring, type->valuestring);

    for (int i = 0; i < cJSON_GetArraySize(books); i++)
    {
        cJSON *book = cJSON_GetArrayItem(books, i);
        cJSON *name = cJSON_GetObjectItem(book, "name");
        cJSON *price = cJSON_GetObjectItem(book, "price");
        printf("book name: %s, price: %lf\n", name->valuestring, price->valuedouble);
    }

    printf("vip: %d\n", vip->valueint);
    if (address && !cJSON_IsNull(address))
    {
        printf("address: %s\n", address->valuestring);
    }
    else
    {
        printf("address is null\n");
    }
}

int CJsonDecode(const char *strJson)
{
    if (!strJson)
    {
        printf("strJson is null\n");
        return -1;
    }
    cJSON *root = cJSON_Parse(strJson);
    // printCJson(root);
    cJSON_Delete(root);
    return 0;
}

#define TEST_COUNT 100000

int main(void)
{
    char *strJson = NULL;
    strJson = CJsonEncode();
    printf("strJson size:%lu, string:%s\n", strlen(strJson), strJson);
    CJsonDecode(strJson);
    free(strJson);
#if 0   
    uint64_t startTime;
    uint64_t nowTime;
    startTime = getNowTime();
    printf("cjson encode time testing\n");
    for (int i = 0; i < TEST_COUNT; i++)
    {
        strJson = CJsonEncode();
        if (strJson)
        {
            free(strJson);
            strJson = NULL;
        }
    }
    nowTime = getNowTime();
    printf("cjson encode %u time, need time: %lums\n", TEST_COUNT, nowTime - startTime);

    strJson = CJsonEncode();
    startTime = getNowTime();
    printf("\ncjson decode time testing\n");
    for (int i = 0; i < TEST_COUNT; i++)
    {
        CJsonDecode(strJson);
    }
    nowTime = getNowTime();
    printf("cjson decode %u time, need time: %lums\n", TEST_COUNT, nowTime - startTime);
    free(strJson);
#endif
    return 0;
}
