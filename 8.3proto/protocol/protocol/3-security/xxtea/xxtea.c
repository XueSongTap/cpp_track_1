/*
 * @Author: your name
 * @Date: 2020-06-09 16:39:41
 * @LastEditTime: 2020-06-09 17:36:49
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \xxtea\xxtea.c
 */ 
#include <stdio.h>  
#include <stdint.h>  
#define DELTA 0x9e3779b9  
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))  
  
void btea(uint32_t *v, int n, uint32_t const key[4])  
{  
    uint32_t y, z, sum;  
    unsigned p, rounds, e;  
    if (n > 1)            /* Coding Part */  
    {  
        rounds = 6 + 52/n;  
        sum = 0;  
        z = v[n-1];  
        do  
        {  
            sum += DELTA;  
            e = (sum >> 2) & 3;  
            for (p=0; p<n-1; p++)  
            {  
                y = v[p+1];  
                z = v[p] += MX;  
            }  
            y = v[0];  
            z = v[n-1] += MX;  
        }  
        while (--rounds);  
    }  
    else if (n < -1)      /* Decoding Part */  
    {  
        n = -n;  
        rounds = 6 + 52/n;  
        sum = rounds*DELTA;  
        y = v[0];  
        do  
        {  
            e = (sum >> 2) & 3;  
            for (p=n-1; p>0; p--)  
            {  
                z = v[p-1];  
                y = v[p] -= MX;  
            }  
            z = v[n-1];  
            y = v[0] -= MX;  
            sum -= DELTA;  
        }  
        while (--rounds);  
    }  
}  
  
#define N 10  
int main()  
{  
    uint32_t v[N]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};  
    uint32_t const k[4]= {2,2,3,4};  
    int n= N; //n的绝对值表示v的长度，取正表示加密，取负表示解密  
    // v为要加密的数据是n个32位无符号整数  
    // k为加密解密密钥，为4个32位无符号整数，即密钥长度为128位  
    // 1. 原始数据
    printf("加密前原始数据：\n");  
    for(int i=0; i < N; i++) {
        printf("%u ",v[i]);  
    }
    printf("\n");

    // 2. 加密
    btea(v, n, k);  
    printf("\n2. 加密后的数据\n");  
    for(int i=0; i < N; i++) {
        printf("%u ",v[i]);  
    }
    printf("\n");

    // 3. 解密
    btea(v, -n, k);  
    printf("\n3. 解密后的数据:\n");  
    for(int i=0; i < N; i++) {
        printf("%u ",v[i]);  
    }
    printf("\n");
    return 0;  
}