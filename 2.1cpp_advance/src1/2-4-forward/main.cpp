//2-4-forward
#include <iostream>

using namespace std;

template <class T>
void Print(T &t)
{
    cout << "L" << t << endl;
}
template <class T>
void Print(T &&t)
{
    cout << "R" << t << endl;
}
template <class T>
void func(T &&t) // 左值右值都只能走这个
{
    cout << "func(T &&t)" << endl;
    Print(t);  // L
    Print(std::move(t));       // 肯定调用"R"
    Print(std::forward<T>(t)); // 不确定L  R  ,如果要完美往下层函数转发加forward
}
//template <class T>
//void func(T &t)
//{
//    cout << "func(T &t)" << endl;
//    Print(t);
//    Print(std::move(t));
//    Print(std::forward<T>(t));
//}
int main()
{
    cout << "-- func(1)" << endl;
    func(1);  //  1是R
    int x = 10;
    int y = 20;
    cout << "-- func(x)" << endl;
    func(x);  // x本身是左值
    cout << "-- func(std::forward<int>(y))" << endl;
    func(std::forward<int>(y)); // std::forward<int>(y)变成右值
//    cout << "-- std::move(y)" << endl;


//    func(std::move(y)); // 这种情况又怎么样呢？
//     func("std::move(y)");
//     func(std::forward<string>("std::move(y)"));
    return 0;
}








