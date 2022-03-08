#pragma once
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "Util.hpp"
#include <sstream>
#include <vector>
#include <unordered_map>
#include <stdlib.h>
#include <sys/stat.h>

#define HOME_PAGE "index.html"
#define WEB_ROOT "wwwroot"
#define OK 200
#define NOT_FOUND 404

class HttpRequest
{
public:
	std::string request_line;
	std::vector<std::string> request_header;
	std::string blank;
	std::string request_body;

	//after parse result
	std::string method;
	std::string uri;
	std::string version;

	std::unordered_map<std::string, std::string> header_map;
	int content_length = 0;
	std::string path;
	std::string query_string;
};

class HttpResponse
{
public:
	std::string status_line;
	std::vector<std::string> response_header;
	std::string blank;
	std::string response_body;

	int status_code = OK;
};

class EndPoint
{
private:
	int sock;
	HttpRequest http_request;
	HttpResponse http_response;

private:
	void RecvHttpRequestLine()
	{
		Util::ReadLine(sock, http_request.request_line);

		std::string &line = http_request.request_line;
		line.resize(line.size() - 1);
		LOG(INFO, line);
		//std::cout << line << std::endl;
	}

	void RecvHttpRequestHeader()
	{
		std::string line;
		while (1)
		{
			line.clear();
			Util::ReadLine(sock, line);
			if (line == "\n")
			{
				http_request.blank = line;
				break;
			}
			line.resize(line.size() - 1);
			http_request.request_header.push_back(line);
			//LOG(INFO, line);
		}
	}

	void ParseHttpRequestLine()
	{
		std::string &line = http_request.request_line;
		std::stringstream ss(line);
		ss >> http_request.method >> http_request.uri >> http_request.version;

		//LOG(INFO, http_request.method);
		//LOG(INFO, http_request.uri);
		//LOG(INFO, http_request.version);
	}

	void ParseHttpRequestHeader()
	{
		// http_request.header_map
		std::string key_out, value_out;

		for (const std::string &tmp : http_request.request_header)
		{
			if (Util::CurString(tmp, key_out, value_out, ": "))
			{
				http_request.header_map[key_out] = value_out;
			}
			else
			{
				LOG(ERROR, "CurString error");
				exit(-1);
			}
		} //end of for

		// for(const auto& pr : http_request.header_map)
		// {
		// 	std::cout << "key: " << pr.first << " value: " << pr.second << std::endl;
		// }
	}

	bool IsNeedRecvHttpRequestBody()
	{
		auto &method = http_request.method;
		if (method == "POST")
		{
			auto &header_kv = http_request.header_map;
			auto iter = header_kv.find("content-Length");
			if (iter != header_kv.end())
			{
				http_request.content_length = atoi(iter->second.c_str());
				return true;
			}
		}

		return false;
	}

	void RecvHttpRequestBody()
	{
		if (IsNeedRecvHttpRequestBody())
		{
			int content_length = http_request.content_length;
			auto &body = http_request.request_body;
			char ch = 0;
			while (content_length)
			{
				ssize_t s = recv(sock, &ch, 1, 0);
				if (s > 0)
				{
					body.push_back(ch);
					content_length--;
				}
				else
				{
					break;
				}
			}
		}
	}

public:
	EndPoint(int _sock) : sock(_sock)
	{
	}
	~EndPoint()
	{
		close(sock);
	}

	//step1  receive
	void RcvHttpRequest()
	{
		RecvHttpRequestLine();
		RecvHttpRequestHeader();
		ParseHttpRequestLine();
		ParseHttpRequestHeader();
		RecvHttpRequestBody();
	}

	//step2  parse
	void ParseHttpRequest()
	{
		//step1 in
	}

	//step3  Build response
	void BuildHttpResponse()
	{

		std::string tmp_path;
		auto &code = http_response.status_code;
		if (http_request.method != "GET" && http_request.method != "POST")
		{
			//非法请求
			LOG(WARNING, "request method is error");
			code = NOT_FOUND;
			goto END;
		}
		if (http_request.method == "GET")
		{
			size_t pos = http_request.uri.find('?');
			if (pos != std::string::npos)
			{
				Util::CurString(http_request.uri, http_request.path, http_request.query_string, "?");
			}
			else
			{
				http_request.path = http_request.uri;
			}
		}

		tmp_path = http_request.path;
		http_request.path = WEB_ROOT;
		http_request.path += tmp_path;

		//访问默认首页情况
		if (http_request.path[http_request.path.size() - 1] == '/')
		{
			http_request.path += HOME_PAGE;
		}
		//测试
		//  std::cout << "debug: path: " << http_request.path << std::endl;
		//  std::cout << "debug: query_string: " << http_request.query_string << std::endl;
		std::cout << "debug: new path:" << http_request.path << std::endl;

		struct stat st;
		if (stat(http_request.path.c_str(), &st) == 0)
		{
			//说明资源是存在的
			
			//如果是一个dir
			if (S_ISDIR(st.st_mode))
			{
				//is a dir
				http_request.path += "/";
				http_request.path += HOME_PAGE; //默认每个文件夹下都有一个默认访问网页 返回默认网页
			}
			//如果为可执行程序
			if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
			{
				//特殊处理... 稍后
			}

		}
		else
		{
			//说明资源是不存在的
			std::string info = http_request.path;
			info += " Not Found !";
			LOG(WARNING, info);
			code = NOT_FOUND;
			goto END;
		}

	END:
		return;
	}

	void SendHttpResponse()
	{
	}
};

class Entrance
{
public:
	static void *HandlerRequest(void *_sock)
	{

		int sock = *(int *)_sock;
		delete (int *)_sock;
		std::cout << "get a new link ... : " << sock << std::endl;

		EndPoint *ep = new EndPoint(sock);
		ep->RcvHttpRequest();
		ep->ParseHttpRequest();
		ep->BuildHttpResponse();
		ep->SendHttpResponse();

		delete ep;
		close(sock);
		return nullptr;
	}
};
