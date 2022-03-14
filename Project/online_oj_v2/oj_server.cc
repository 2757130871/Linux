#include "cpp-httplib-0.10.2/httplib.h"

#include <jsoncpp/json/json.h>

#include "oj_model.hpp"
#include "Util.hpp"
#include "oj_view.hpp"
#include "Compile.hpp"

// controller 作为服务器核心逻辑，需要创建好对应的服务器框架代码
//在这个框架中来组织逻辑

int main()
{
    OjModel model;
    model.Load(); //只需要初始化一次

    using namespace httplib;

    Server server;

    //[] 内部可以引用外部对象使用 变量捕获
    server.Get("/all_questions", [&model](const Request &req, Response &resp) {
        (void)req;
        std::vector<Question> all_questions;
        model.GetAllQuestions(&all_questions); //获得所有question对象

        // all_questions -->> HTML View层做的事情
        std::string html;
        OjView::RenderAllquestions(all_questions, &html);
        resp.set_content(html, "text/html");
    });

    //R("") 原始字符串，字符串内部的转义字符被忽略。
    server.Get(R"(/question/(\d+))", [&model](const Request &req, Response &resp) {
        std::string id = req.matches[1].str(); //下标1取出id
        Question q;
        model.GetQuestion(id, &q);

        // all_questions -->> HTML View层做的事情
        std::string html;
        OjView::RenderQuestion(q, &html);
        resp.set_content(html, "text/html");
    });

    server.Post("/Compile", [&model](const Request &req, Response &resp) {
        Json::Value req_json;
        Json::Value resp_json;

        std::unordered_map<std::string, std::string> body_kv;
        UrlUtil::ParseBody(req.body, &body_kv);

        for (const auto &var : body_kv)
        {
            req_json[var.first] = var.second;
        }

        Question q;
        model.GetQuestion(req_json["id"].asString(), &q);
        req_json["code"] = req_json["code"].asString() + q.tail_cpp;

        Compiler::CompileAndRun(req_json, &resp_json);

        std::string html;
        OjView::RenderResult(req_json["stdout"].asString(), req_json["reason"].asString(), &html);
        resp.set_content(html, "text/html");
    });

    // server.Get("/css/index.css", [](const Request &req, Response &resp) {
    //     std::string url = "." + req.path;
    //     std::string css;
    //     FileUtil::Read(url, &css);
    //     std::cout << "DEBUG: " << url << std::endl;
    //     std::cout << "css: -----" << css << std::endl;

    //     resp.set_content(css, "text/css");
    // });

    // server.Get("/css/bootstrap.min.css", [](const Request &req, Response &resp) {
    //     std::string url = "." + req.path;
    //     std::string css;
    //     FileUtil::Read(url, &css);
    //     resp.set_content(css, "text/css");
    // });

    // server.Get(R"(/css/.css(\$))", [](const Request &req, Response &resp) {
    //     std::string url = "." + req.path;
    //     std::string css;
    //     FileUtil::Read(url, &css);

    //     resp.set_content(css, "text/css");
    // });

    // server.Get("/images/01.png", [](const Request &req, Response &resp) {
    //     std::string url = "./css" + req.path;
    //     std::string img;
    //     FileUtil::Read(url, &img);
    //     std::cout << "DEBUG: " << url << std::endl;
    //     resp.set_content(img, "image/png");
    // });

    server.set_base_dir("./wwwroot");
    server.listen("0.0.0.0", 80);
}