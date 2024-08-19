#pragma once
#include<iostream>
#include<vector>
#include<sys/uio.h> //readv
#include<assert.h>  //readv
#include<unistd.h> //write
#include<atomic>
class buffer
{
private:
    std::vector<char>buffer_;
    

public:
    buffer(int initBuffsize =1024);
    ~buffer()=default;

    size_t
    uint64_t

};

