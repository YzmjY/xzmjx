//
// Created by 20132 on 2022/3/16.
//
#include <ucontext.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
using namespace std;

int main(){
    ucontext_t context;
    getcontext(&context);
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t,&tm);
    char buf[128];
    strftime(buf,128,"%Y-%m-%d %H:%M:%S",&tm);
    cout<<buf<<endl;
    sleep(1);
    setcontext(&context);
    return 0;
}
