#pragma once

#include <ctemplate/template.h>
#include "oj_model.hpp"


class OjView
{
public:
    //通过数据生成 html（render）
    //模板类似于填空题,实现准备好一个html把其中一些需要
    //动态计算的数据挖个空留下来,处理请求过程中，
    //根据计算结果来填这样的空
    static bool RenderAllquestions(const std::vector<Question> &all_questions, std::string *html)
    {
        //1，先创建一个ctemplate的对象，这是一个总的组织数据的

        // 2．循环的往这个对象中添加一些子对象
        // 3．每一个子对象再设置一些键值对(和模板中留下的{{3}/l是要对应的)
        // 4．进行数据的替换，生成最终的html
        ctemplate::TemplateDictionary dict("all_questions");
        for (const auto &question : all_questions)
        {
            ctemplate::TemplateDictionary *table_dict = dict.AddSectionDictionary("question");
            table_dict->SetValue("id", question.id);
            table_dict->SetValue("name", question.name);
            table_dict->SetValue("star", question.star);
        }
        ctemplate::Template *tpl;
        tpl = ctemplate::Template::GetTemplate(
            "./wwwroot/index.html", ctemplate::DO_NOT_STRIP);
        tpl->Expand(html, &dict);

        return true;
    }

    //读取单个对象
    static bool RenderQuestion(const Question &q, std::string *html)
    {
        ctemplate::TemplateDictionary dict("question");

        dict.SetValue("id", q.id);
        dict.SetValue("name", q.name);
        dict.SetValue("star", q.star);
        dict.SetValue("header", q.header_cpp);
        dict.SetValue("desc", q.desc);

        ctemplate::Template *tpl;
        tpl = ctemplate::Template::GetTemplate(
            "./wwwroot/question.html", ctemplate::DO_NOT_STRIP);
        tpl->Expand(html, &dict);

        return true;
    }

    static void RenderResult(const std::string &str_stdout,
                             const std::string &reason, std::string *html)
    {
        ctemplate::TemplateDictionary dict("result");
        dict.SetValue("stdout", str_stdout);
        dict.SetValue("reason", reason);
        ctemplate::Template *tpl;
        tpl = ctemplate::Template::GetTemplate(
            "./wwwroot/result.html", ctemplate::DO_NOT_STRIP);
        tpl->Expand(html, &dict);
    }

 
};
