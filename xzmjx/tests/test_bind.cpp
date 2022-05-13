//
// Created by 20132 on 2022/4/18.
//
#include <iostream>
#include <functional>
using namespace std;
class Father{
public:
    Father() = default;
    virtual ~Father() = default;

    virtual void printMessage(){
        cout<<"Father:"<<m_data<<endl;
    }
    void setData(int v){
        m_data = v;
    }

protected:
    int m_data = 0;
};

class Child : public Father{
public:
    void printMessage() override{
        cout<<"Child:"<<m_data<<endl;
    }
};

int main(){
    Father* pa = new Father();
    pa->setData(10);
    function<void()> fa = bind(&Father::printMessage,pa);
    fa();

    cout<<"--------------------------"<<endl;
    Father* pb = new Child();
    pb->setData(20);
    function<void()> fb = bind(&Father::printMessage,pb);
    fb();
    return 0;
}