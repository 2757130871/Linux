#include <iostream>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <jsoncpp/json/json.h>

#include "oj_model.hpp"
#include "Util.hpp"
#include "oj_view.hpp"
#include "Compile.hpp"

//获取GET 或者 POST传递过来的参数 (GET 环境变量传参) (POST 匿名管道传参)
bool GetQueryString(std::string &query_string)
{
    bool result = false;
    std::string method = getenv("METHOD");

    if (method == "GET")
    {
        query_string = getenv("QUERY_STRING");

        result = true;
    }
    else if (method == "POST")
    {
        //CGI如何得知需要从标准输入读取多少个字节呢
        int content_length = atoi(getenv("CONTENT_LENGTH"));
        std::cerr << "DEBUG content_length :" << content_length << std::endl;
        char c = 0;
        while (content_length)
        {
            read(0, &c, 1);
            query_string.push_back(c);
            content_length--;
        }
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}

//例 in: a=b sp:= out1=a out2=b
void CutString(std::string &in, const std::string &sep, std::string &outl, std::string &out2)
{
    auto pos = in.find(sep);
    if (std::string::npos != pos)
    {
        outl = in.substr(0, pos);
        out2 = in.substr(pos + sep.size());
    }
}

// CGI 程序标准输入和标准输出已经被重定向了 不能使用cout
int main()
{
    std::cerr << "CGI START ------------------------------" << std::endl;

    OjModel model;
    model.Load();

    std::string method = getenv("METHOD");
    std::string compile_id = "";

    if (method == "GET")
    {
        const char *q_ptr = getenv("QUESTION_ID");
        std::string question_id = q_ptr ? q_ptr : "";

        if (question_id.empty()) //默认访问首页
        {
            std::vector<Question> all_questions;
            model.GetAllQuestions(&all_questions);
            std::string index_html;
            OjView::RenderAllquestions(all_questions, &index_html);

            std::cout << index_html;
        }
        else // return a Question page
        {
            Question q;
            model.GetQuestion(question_id, &q);
            std::string question_html;

            OjView::RenderQuestion(q, &question_html);

            std::cout << question_html; //通过pipe 传输
        }
    }
    else //POST  Compile and Run
    {
        compile_id = getenv("COMPILE_ID");
        std::string request_body;
        Json::Value req_json;
        Json::Value resp_json;
        GetQueryString(request_body); //获取request_body

        std::unordered_map<std::string, std::string> body_kv;
        UrlUtil::ParseBody(request_body, &body_kv); //完成了UrlDecode 和 kv映射

        for (const auto &kv : body_kv)
            req_json[kv.first] = kv.second;

        Question q;
        model.GetQuestion(compile_id, &q);
        
        std::cerr << req_json["code"] << std::endl; //DEBUG 

        req_json["code"] = req_json["code"].asString() + q.tail_cpp; //拼接测试代码

        Compiler::CompileAndRun(req_json, &resp_json); //

        std::string result_html; //render result page
        OjView::RenderResult(resp_json["stdout"].asString(), resp_json["reason"].asString(), &result_html);

        std::cout << result_html;
    }

    std::cerr << "CGI END ------------------------------" << std::endl;
}
