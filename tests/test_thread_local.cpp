#include <pthread.h>
#include <vector>
#include <iostream>
#include <list>
#include <atomic>

// 用一个combiner去搜集各个线程里面的局部变量
// 最后merge，bvar的基础理念，尽可能快的写
class ThreadLocal{
public:
    ThreadLocal(){
    }
    ~ThreadLocal(){
    }
    static int* getData() { return a_; }
    static void setData(int a){
        if(a_ == nullptr){
            a_ = new int();
        }
        *a_ = a;
    }
private:
    static __thread int* a_;
};
__thread int* ThreadLocal::a_ = nullptr;


class Combiner{
public:
    Combiner(){}
    ~Combiner(){}
    void Push(int* p){
        combiner_.push_back(p);
    }
    void Print() const{
        for(auto&& it:combiner_){
            std::cout<<*it<<" ";
        }
        std::cout<<std::endl;
    }
private:
    std::list<int* > combiner_;
};

Combiner c;
std::atomic<int> i = 0;
void* thread_cb(void* arg){
    if(nullptr == ThreadLocal::getData()){
        ThreadLocal::setData(i++);
    }
    c.Push(ThreadLocal::getData());
    return nullptr;
}


int main(){
    std::vector<pthread_t> v;
    for(int i = 0;i<10;i++){
        pthread_t tid;
        pthread_create(&tid,NULL,thread_cb,nullptr);
        v.push_back(tid);
    }
    for(auto&& i:v){
        pthread_join(i,nullptr);
    }
    c.Print();
    return 0;
}