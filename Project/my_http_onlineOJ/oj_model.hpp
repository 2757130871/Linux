//MVC(经典的软件设计方式，20年前就有了
//1现代还有一些更先进的方式比如 MVVM
//M=>model:负责数据存储
//V=>view:负责显示界面
//C=>controller:核心业务逻辑
//基于文件的方式完成题目的存储
//约定每一个题目对应一个目录，目录的名字就是题目的id

//目录里面包含以下几个文件:
//1) header.cpp 代码框架
//2) tail.cpp代码测试用例
//3) desc.txt题目详细描述

//除此之外，再搞一个oj_config.cfg文件，作为一个总的入口文件，
//这个文件是一个行文本文件.
//这个文件的每一行对应一个需要被服务器加载起来的题目
//这一行里面包含以下几个信息:题目的id，题目的名字，题目的难度,题目对应的目录

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>

#include "Util.hpp"

//model这个模块要做的事情就是把刚才在文件中存储的题目信息加载起来,供服务器随时使用

struct Question
{
    std::string id;
    std::string name;
    std::string dir;  //题目对应的目录，目录就包含了题目描述/题目的代码框架/题目的测试用例
    std::string star; //表示难度

    //以下这几个字段要根据 dir字段来获取到
    std::string desc;       //题目的描述
    std::string header_cpp; //题目的代码框架中的代码
    std::string tail_cpp;   // 题目的测试用例代码
};

class OjModel
{
private:
    std::map<std::string, Question> _model;

public:
    //把文件上的数据加载到内存中,加到哈希表中

    bool Load()
    {
        //1．先打开oj_config.cfg
        std::ifstream file("./oj_data/oj_config.cfg");
        if (!file.is_open())
        {
            LOG(ERROR) << "File open error!" << std::endl;
            return false;
        }

        std::string line;
        std::vector<std::string> tmp;
        //2．按行读取oj_config.cfg文件，并且解析
        while (std::getline(file, line))
        {
            tmp.clear();
            StringUtil::Split(line, " ", &tmp);
            if (tmp.size() != 4) //读取出错
            {
                LOG(ERROR) << "oj_config read error check the format" << std::endl;
                return false;
            }

            //3．根据解析结果拼装成Question结构体
            Question q;
            q.id = tmp[0];
            q.name = tmp[1];
            q.star = tmp[2];
            q.dir = tmp[3];

            FileUtil::Read(q.dir + "/desc.txt", &(q.desc));
            FileUtil::Read(q.dir + "/header.cpp", &(q.header_cpp));
            FileUtil::Read(q.dir + "/tail.cpp", &(q.tail_cpp));

            //4．把结构体加入到hash表中
            _model[q.id] = q;
        }

        return true;
    }

    bool GetAllQuestions(std::vector<Question> *questions) const
    {
        questions->clear();
        for (const auto &pr : _model)
            questions->push_back(pr.second);

        return true;
    }

    bool GetQuestion(const std::string &id, Question *q) const
    {
        auto it = _model.find(id);
        if (it == _model.end())
        {
            LOG(ERROR) << "Get Question Error!" << std::endl;
            return false;
        }

        *q = it->second;
        return true;
    }

    void Show()
    {
        std::cout << "--------------------DEBUG-------------------- " << std::endl;
        for (const auto &e : _model)
        {
            std::cout << e.second.id << "," << e.second.name << std::endl;
        }
        std::cout << "--------------------DEBUG-------------------- " << std::endl;
    }
};
