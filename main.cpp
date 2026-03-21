#include "include/void_any.hpp"
#include <string>
#include <iostream>

int main()
{
    void_any a;
    void_any b(123124);
    a.set(std::string("hello world!"));

    //失败返回默认构造
    //Return default constructor on failure
    std::cout << b.get<int>() << std::endl;


    auto p=a.get_ptr<std::string>();
    if(p==nullptr)
    {
        std::cout << "get_ptr failed" << std::endl;
    }
    else
    {
        std::cout << *p<< std::endl;
    }


    std::cout << b.get_type_id() << std::endl;
    std::cout << a.get_type_id() << std::endl;

}