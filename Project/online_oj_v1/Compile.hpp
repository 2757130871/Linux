#pragma once

//标准库
#include <string>
#include <atomic> //原子

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

//第三方库
#include <jsoncpp/json/json.h>

//当前项目其他头文件
#include "Util.hpp"

class Compiler
{
public:
    //本.质上此处是使用文件来完成进程间通信
    // 1．源代码文件,name表示当前请求的名字
    //请求和请求之间,name必须是不相同的
    // tmp_1647068905.1.cpp
    // tmp_1647068905.2.cpp
    static std::string SrcPath(const std::string &name)
    {
        return "./temp_files/" + name + ".cpp";
    }

    // 2 编译错误文件 .compile_error
    static std::string CompileErrorPath(const std::string &name)
    {
        return "./temp_files/" + name + ".compile_error";
    }
    // 3．可执行程序文件 .exe
    static std::string ExePath(const std::string &name)
    {
        return "./temp_files/" + name + ".exe";
    }
    // 4．标准输入文件 .stdin
    static std::string StdinPath(const std::string &name)
    {
        return "./temp_files/" + name + ".stdin";
    }
    // 5．标准输出文件 .stdout
    static std::string StdoutPath(const std::string &name)
    {
        return "./temp_files/" + name + ".stdout";
    }
    // 6．标准错误文件 .stderr
    static std::string StderrPath(const std::string &name)
    {
        return "./temp_files/" + name + ".stderr";
    }

    // Value 是jsoncpp的核心类 完成序列化和反序列化 使用起来像map
    static bool CompileAndRun(const Json::Value &req, Json::Value *resp)
    {
        // 1 根据请求对象，生成源代码文件
        if (req["code"].empty())
        {
            (*resp)["error"] = 3;
            LOG(ERROR) << "code empty" << std::endl;
            return false;
        }

        // req["code"]根据key取出value. value类型也是
        // Json::Value.这个类型通过asString() 转成字符串
        const std::string &code = req["code"].asString();

        //通过这个函数完成把代码写到代码文件中的过程．
        std::string file_name = writeTmpFile(code, req["stdin"].asString());
        std::cout << "---------------DEBUG: " << file_name << std::endl;
        // 2．调用g++进行编译(fork + exec / system).
        //生成可执行程序,如果编译出错，需要把编译错误记录下来(重定向到文件中).

        bool ret = Compile(file_name);
        if (ret == false) //编译错误，错误处理
        {

            (*resp)["error"] = 1; //编译错误
            std::string reason;
            FileUtil::Read(CompileErrorPath(file_name), &reason);
            (*resp)["reason"] = reason;
            LOG(ERROR) << "Compile error!" << std::endl;
            return false;
        }

        // 3．调用可执行程房，把标准输入记录到文件中，
        //然后把文件中的内容重定向给可执行程序,可执行程序的标准输出和
        //标准错误内容也要重定问输出记永到文件中
        int sig = Run(file_name);
        if (sig != 0)
        {
            //错误处理
            (*resp)["error"] = 2; //运行错误
            (*resp)["reason"] = "Program exit by signo: " + std::to_string(sig);
            //用户提交的代码出错，提示INFO
            LOG(INFO) << "Program exit by signo: " << std::to_string(sig) << std::endl;
            return false;
        }

        // 4.把程序处理结果返回，构建resp对象

        (*resp)["error"] = 0;
        (*resp)["reason"] = "";
        std::string str_stdout;
        FileUtil::Read(StdoutPath(file_name), &str_stdout);

        (*resp)["stdout"] = str_stdout;
        std::string str_stderr;
        FileUtil::Read(StderrPath(file_name), &str_stderr);
        (*resp)["stderr"] = str_stderr;

        return true;
    }

private:
    // 1．把代码写到文件里
    // 2．给这次请求分配一个唯一的名字，通过返回值返回
    // 分配的名字如 tmp_1647068905.2
    //生成一个唯一的名字 并 写文件
    static std::string writeTmpFile(const std::string &code, const std::string &str_stdin)
    {
        static std::atomic_int id(0);
        id++;
        std::string file_name = "tmp_" + std::to_string(TimeUtil::TimeStamp()) + "." + to_string(id);

        FileUtil::Write(SrcPath(file_name), code);
        FileUtil::Write(StdinPath(file_name), str_stdin);

        return file_name;
    }

    static bool Compile(const std::string &file_name)
    {
        // 1．先构造出编译指令.
        // g++ file_name.cpp -o file_name.exe -std=c++11
        char *command[20] = {0};
        char buf[20][50] = {{0}};
        for (int i = 0; i < 20; ++i)
        {
            command[i] = buf[i];
        }

        // 必须要保证command的指针都是指向有效内存
        sprintf(command[0], "%s", "g++");
        sprintf(command[1], "%s", SrcPath(file_name).c_str());
        sprintf(command[2], "%s", "-o");
        sprintf(command[3], "%s", ExePath(file_name).c_str());
        sprintf(command[4], "%s", "-std=c++11");
        command[5] = NULL;

        // 2 创建子进程
        int ret = fork();
        if (ret > 0)
        {
            // 3 父进程进行进程等待
            waitpid(ret, NULL, 0);
            std::cout << "子进程退出了" << std::endl;
        }
        else
        {
            // 4 子进程进行进程替换
            int fd = open(CompileErrorPath(file_name).c_str(), O_RDWR | O_CREAT, 0666);
            if (fd < 0)
            {
                LOG(ERROR) << "open Compile file error!" << std::endl;
                exit(1);
            }

            dup2(fd, 2);
            execvp(command[0], command);
            LOG(ERROR) << "exec Fatal!!!" << std::endl;
            exit(0);
        }

        //判断可执行文件是否存在来判定是否编译成功
        struct stat st;
        if (stat(ExePath(file_name).c_str(), &st) != 0) //文件不存在
        {
            LOG(ERROR) << "Compile failed file not found: " << file_name << std::endl;
            return false;
        }
        

        LOG(ERROR) << "Compile file: " << file_name << "OK!" << std::endl;

        return true;
    }

    static int Run(const std::string &file_name)
    {
        // 1．创建子进程
        int ret = fork();
        if (ret > 0)
        {
            // 2．父进程进行等待
            int status = 0;
            waitpid(ret, &status, 0);
            return status & 0x7f;
        }
        else
        {
            // 3．进行重定向(标准输入，标准输出，标准错误)
            int fd_stdin = open(StdinPath(file_name).c_str(), O_RDONLY);
            dup2(fd_stdin, 0);
            int fd_stdout = open(StdoutPath(file_name).c_str(), O_WRONLY | O_CREAT, 0666);
            dup2(fd_stdout, 1);
            int fd_stderr = open(StderrPath(file_name).c_str(), O_WRONLY | O_CREAT, 0666);
            dup2(fd_stderr, 2);

            // 4，子进程进行程序替换
            execl(ExePath(file_name).c_str(), ExePath(file_name).c_str(), NULL);
            exit(0);
        }

        return 0;
    }
};
