 gcc sync_dns_client.c  

不要用g++ , 会有报错

```bash
yxc@yxc-MS-7B89:~/code2/cpp_track_1/6.3async/async$ g++ sync_dns_client.c 
sync_dns_client.c: In function ‘int dns_parse_response(char*, dns_item**)’:
sync_dns_client.c:178:23: error: invalid conversion from ‘char*’ to ‘unsigned char*’ [-fpermissive]
  unsigned char *ptr = buffer;
                       ^~~~~~
sync_dns_client.c:211:42: error: invalid conversion from ‘char*’ to ‘unsigned char*’ [-fpermissive]
   dns_parse_name(buffer, ptr, aname, &len);
                                          ^
sync_dns_client.c:138:13: note:   initializing argument 1 of ‘void dns_parse_name(unsigned char*, unsigned char*, char*, int*)’
 static void dns_parse_name(unsigned char *chunk, unsigned char *ptr, char *out, int *len) {
             ^~~~~~~~~~~~~~
sync_dns_client.c:227:43: error: invalid conversion from ‘char*’ to ‘unsigned char*’ [-fpermissive]
    dns_parse_name(buffer, ptr, cname, &len);
                                           ^
sync_dns_client.c:138:13: note:   initializing argument 1 of ‘void dns_parse_name(unsigned char*, unsigned char*, char*, int*)’
 static void dns_parse_name(unsigned char *chunk, unsigned char *ptr, char *out, int *len) {
             ^~~~~~~~~~~~~~
sync_dns_client.c: At global scope:
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
 };
 ^
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
sync_dns_client.c:350:1: warning: ISO C++ forbids converting a string constant to ‘char*’ [-Wwrite-strings]
yxc@yxc-MS-7B89:~/code2/cpp_track_1/6.3async/async$ 
```

这是因为 C++ 比 C 语言有更严格的类型检查。在 C 中，char* 和 unsigned char* 之间的转换是允许的，而在 C++ 中则不允许。因此，当你使用 g++（C++ 编译器）而不是 gcc（C 编译器）来编译这段代码时，你会得到错误信息。为了解决这个问题，你需要进行显式的类型转换，或者确保你传递给函数的参数类型是正确的。

