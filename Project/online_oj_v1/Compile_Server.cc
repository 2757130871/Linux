//#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "cpp-httplib-master/httplib.h"
#include "Util.hpp"
#include "Compile.hpp"
#include <jsoncpp/json/json.h>

int main()
{
  using namespace httplib;

  Server server;

  server.Get("/", [](const Request &req, Response &res) { //注册一个回调函数,调用时机在处理get方法的时候
    (void)req;

    Json::Value req_json;  //从req对象获取
    Json::Value resp_json; // resp_json放到响应中
    Compiler::CompileAndRun(req_json, &resp_json);

    Json::FastWriter writer; //将json对象序列化为字符串

    //需要将对象序列化成字符串返回
    res.set_content(writer.write(resp_json), "text/html");
  });

  server.listen("0.0.0.0", 80);

  return 0;
}
