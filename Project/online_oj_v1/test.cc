#include "Compile.hpp"
#include "Util.hpp"

int main()
{
    //LOG(INFO) << "!!!!!!!!!!!!!!!!!!!!LOG" << std::endl;
    Json::Value val, resp;
    val["code"] = "#include <iostream> \n int main(){ int a = 0,b = 2,c;  std::cout << 11111111111 << std::endl; }";
    val["stdin"] = "";

    Compiler::CompileAndRun(val, &resp);
    Json::FastWriter writer;
    std::string ret = writer.write(resp);

    std::cout << ret << std::endl;
}