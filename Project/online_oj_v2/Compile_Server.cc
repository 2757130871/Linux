//#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "./cpp-httplib-0.10.2/httplib.h"
#include <unordered_map>
#include <jsoncpp/json/json.h>
#include "Compile.hpp"
#include "Util.hpp"

int main()
{
  using namespace httplib;
  Server server;

  //路由 通过访问这个路径执行回调函数 注册一个回调函数,调用时机在处理get方法的时候
  server.Post("/Compile", [](const Request &req, Response &res) {
    

    std::unordered_map<std::string, std::string> body_kv;
    UrlUtil::ParseBody(req.body, &body_kv);

    Json::Value req_json;  //从req对象获取
    Json::Value resp_json; // resp_json放到响应中
    
    for(const auto& pr : body_kv)
    {
      req_json[pr.first] = pr.second; 
    }

    Compiler::CompileAndRun(req_json, &resp_json);

    Json::FastWriter writer; //将json对象序列化为字符串

    //需要将对象序列化成字符串返回
    res.set_content(writer.write(resp_json), "text/html");
  });

  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0", 80);

  return 0;
}
