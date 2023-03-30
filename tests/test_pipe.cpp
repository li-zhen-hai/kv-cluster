#include <iostream>
#include <string>
#include <map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "star/rpc/serializer.h"

int main(){

    std::map<int,std::string> val;

    int fd[2]={0};

    if(pipe(fd) == -1){
        perror("pipe error");
        exit(-1);
    }

    pid_t pid = fork();

    if(pid == 0){
        star::rpc::Serializer s;
        for(int i=0;i<10;++i)
            val[i] = std::to_string(i);
        close(fd[0]);
        s << val;
        s.reset();
        star::rpc::Serializer s1(s.toString());
        s1 >> val;
        
        // for(auto q : val)
        //     std::cout << q.first << " " << q.second << std::endl;

        // std::cout << s.toString().c_str() << std::endl;

        int size = s.size();
        ssize_t ret = write(fd[1],&size,4);
        ret =  write(fd[1],s.toString().c_str(),s.size());
        (void)ret;
        // std::cout << "write size " << ret << ", size " << s.size() << std::endl;
        close(fd[1]);
    }else{
        close(fd[1]);
        //char buf[65535]={0};
        // ssize_t ret = read(fd[0],buf,65535);
        // std::string tmp(buf,ret);
        // star::rpc::Serializer s(tmp);
        int size=0; 
        std::string tmp="";
        ssize_t n = read(fd[0],&size,4);
        tmp.resize(size);
        std::cout << size << std::endl;
        n = read(fd[0],tmp.data(),size);
        // std::cout << buf << std::endl;
        // if(n>0)
        //     tmp = std::string(buf,size);
        (void)n;
        std::cout << tmp << std::endl;
        star::rpc::Serializer s(tmp);
        s >> val;
        for(auto q : val)
            std::cout << q.first <<" " << q.second << std::endl;
        std::cout << tmp << std::endl;
        close(fd[0]);
        wait(nullptr);
    }
    return 0;

    return 0;
}