/*
 * @Descripttion: 
 * @version: 1.0
 * @Author: Darren
 * @Date: 2019-10-29 15:14:40
 * @LastEditors: Darren
 * @LastEditTime: 2019-10-29 20:26:43
 */
#include <iostream>

class Persion
{
public:
    Persion()
    {
        printf("构造Persion\n");
    }
    ~Persion()
    {
        printf("析构~Persion\n");
    }
    void print()
    {
        printf("darren\n");
    }
};

class Home
{
public:
    Home()
    {
        printf("Home\n");
    }
    ~Home()
    {
        printf("Home\n");
    }
    Persion &get()
    {
        return son;
    }

private:
    Persion son;
};

int main()
{
    printf("1.Home\n");
    Home home;
    printf("2.Home\n");
    Persion &son1 = home.get();
    home.get().print();
    return 0;
}