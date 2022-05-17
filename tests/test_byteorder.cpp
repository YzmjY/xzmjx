//
// Created by 20132 on 2022/4/17.
//
#include<iostream>
using namespace std;

struct ByteStruct{
    char b1;
    char b2;
    char b3;
    char b4;
};

int main(){
    ByteStruct x;
    x.b1 = 0x1;
    x.b2 = 0x2;
    x.b3 = 0x4;
    x.b4 = 0x5;
    ByteStruct* ptr = &x;
    cout<<ptr<<endl;
    cout<<"b1->"<<*(char*)ptr<<"b2->"<<*((char*)ptr+1)<<"b3->"<<*((char*)ptr+2)<<"b4->"<<*((char*)ptr+3)<<endl;
    return 0;
}
