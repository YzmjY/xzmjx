//
// Created by 20132 on 2022/3/11.
//
#include <memory>
#include <iostream>
#include <functional>
using namespace std;

class A{
public:
    typedef std::shared_ptr<A> ptr;
    A(){
        cout<<"A()"<<endl;
    }


    ~A(){
        cout<<"~A()"<<endl;
    }
};

void del(A* a){
    delete a;
   cout<<"del()"<<endl;
}

class test:public enable_shared_from_this<test>{
public:
    test() = default;
    ~test() = default;

    void A(){
        shared_from_this();
    }
};
int main(){
/*    {
        A::ptr a(new A,std::bind(del,std::placeholders::_1));
    }*/
/*    {
        A s;
        A::ptr x(&s);
    }*/

    //test* t = new test;
    //t->A();
    std::weak_ptr<test> r;
    try{
        std::shared_ptr<test> p = r.lock();
        if(p == nullptr){
            std::cout<<"error but return"<<std::endl;
        }
    }catch (...){
        std::cout<<"error throw and catch"<<std::endl;
    }
    return 0;
}


