//
// Created by 20132 on 2022/3/17.
//
#include <iostream>

using namespace std;
class A{
public:
    A(const A& x){
        a = new int(*x.a);
        cout<<"copy"<<endl;
    }
    A(A&& x){
        this->a = x.a;
        x.a = nullptr;
        cout<<"move"<<endl;
    }
    A(){
        a = new int;
    }
    ~A(){
        delete a;
    }

    int * a;
};
class B{
public:
    B(A x):a(move(x)){

    }
    ~B(){}
    A a;
};

int main(){
    A a = A();
    B b(a);
    cout<<a.a<<endl;
    return 0;

}