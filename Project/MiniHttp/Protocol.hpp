#pragma once
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Util.hpp"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/wait.h>

#define HOME_PAGE "index.html"
#define WEB_ROOT "wwwroot"
#define OK 200
#define NOT_FOUND 404
#define HTTP_VERSION "HTTP/1.0"
#define LINE_END "\r\n"

static std::string Code2Desc(int code)
{
	std::string desc;
	switch (code)
	{
	case 200:
		desc = "OK";
		break;
	case 404:
		desc = "Not Found";
		break;
	default:
		break;
	}

	return desc;
}

static std::string Suffix2Desc(const std::string &suffix)
{
	static std::unordered_map<std::string, std::string> suffix2desc = {
		{".html", "text/html"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".jpg", "application/x-jpg"},
		{".xml", "application/xml"},
		{".png", "application/x-png"},
		{".gif", "image/gif"},
	};
	auto iter = suffix2desc.find(suffix);
	if (iter != suffix2desc.end())
		return iter->second;

	return "text/html";
	//return "";
}

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
	std::string suffix;

	bool cgi = false; //CGI
};

class HttpResponse
{
public:
	std::string status_line;
	std::vector<std::string> response_header;
	std::string blank = LINE_END;
	std::string response_body;

	int status_code = OK;
	int fd = -1;  //指向所请求资源的文件描述符
	int size = 0; //想访问资源的大小
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

		for (char &c : http_request.method)
			toupper(c);
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
				//LOG(, "CurString error");
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
			auto iter = header_kv.find("Content-Length");
			if (iter != header_kv.end())
			{
				// std::cerr << "DEBUG iter" << iter->first <<  ":" <<iter->second<< std::endl;
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

			//std::cerr << "DEBUG" << body << std::endl;
		} //end of if
	}

	//返回静态网页
	int ProcessNonCGI(int size)
	{
		http_response.fd = open(http_request.path.c_str(), O_RDONLY);

		if (http_response.fd > 0)
		{
			http_response.status_line = HTTP_VERSION;
			http_response.status_line += " ";
			http_response.status_line += std::to_string(http_response.status_code);
			http_response.status_line += " ";
			http_response.status_line += Code2Desc(http_response.status_code);
			http_response.status_line += LINE_END;
			http_response.size = size;

			std::string header_line = "Content-Length: ";
			header_line += std::to_string(size);
			http_response.response_header.push_back(header_line);
			http_response.response_header.push_back(LINE_END);
			header_line = "Content-Type: ";
			header_line += Suffix2Desc(http_request.suffix);
			http_response.response_header.push_back(header_line);
			http_response.response_header.push_back(LINE_END);

			return OK;
		}
		else
		{
			LOG(ERROR, "open file error");
		}

		return NOT_FOUND;
	}

	//处理CGI
	int ProcessCGI()
	{
		auto method = http_request.method;
		auto query_string = http_request.query_string; //GET
		auto body_text = http_request.request_body;	   //POST
		auto &bin = http_request.path;
		std::string query_string_env;
		std::string method_env;
		int code = OK;

		int input[2];
		int output[2];
		if (pipe(input) < 0)
		{
			LOG(ERROR, "pipe input error");
			return 404;
		}
		if (pipe(output) < 0)
		{
			LOG(ERROR, "pipe output error");
			return 404;
		}

		pid_t pid = fork();
		if (pid == 0) //child
		{
			//exec
			close(input[0]);
			close(output[1]);

			method_env = "METHOD=";
			method_env += method;
			putenv((char *)method_env.c_str());

			if (method == "GET")
			{
				query_string_env = "QUERY_STRING=";
				query_string_env += query_string;
				putenv((char *)query_string_env.c_str());
				std::cout << "add env query " << std::endl;
			}

			//站在子进程角度 子进程只要向fd0读，向fd1写
			dup2(output[0], 0);
			dup2(input[1], 1);

			std::cerr << "DEBUG bin:" << bin << std::endl;
			execl(bin.c_str(), bin.c_str(), nullptr);

			//往后执行说明execl执行错误
			std::cerr << "execl ERROR" << std::endl;
			exit(-1);
		}
		else if (pid < 0) //error
		{
			LOG(ERROR, "fork error!");
			return 404;
		}
		else //parent
		{
			close(input[1]);
			close(output[0]);

			//std::cerr << "!!!" << body_text << std::endl; //DEBUG

			if (method == "POST") //持续写管道
			{
				const char *start = body_text.c_str();
				int total = 0;
				int size = 0;
				while ((size = write(output[1], start + total, body_text.size() - total)) > 0)
				{
					total += size;
				}
			}

			waitpid(pid, nullptr, 0);
			close(input[0]);
			close(output[1]);
		}

		return OK;
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
		int size = 0; //保存请求文件的大小
		size_t found = 0;
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
				http_request.cgi = true;
			}
			else
			{
				http_request.path = http_request.uri;
			}
		}
		else if (http_request.method == "POST")
		{
			//POST
			http_request.cgi = true;
			http_request.path = http_request.uri;
		}
		else
		{
			//Do nothing
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
		//std::cout << "debug: new path:" << http_request.path << std::endl;

		struct stat st;
		//判断是否为文件还是
		if (stat(http_request.path.c_str(), &st) == 0)
		{
			//说明资源是存在的

			//如果是一个dir
			if (S_ISDIR(st.st_mode))
			{
				//is a dir
				http_request.path += "/";
				http_request.path += HOME_PAGE;		  //默认每个文件夹下都有一个默认访问网页 返回默认网页
				stat(http_request.path.c_str(), &st); //重新更新
			}
			//如果为可执行程序
			if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
			{
				//特殊处理... 稍后
				http_request.cgi = true;
			}
			size = st.st_size;
		}
		else //说明资源不存在
		{

			std::string info = http_request.path;
			info += " Not Found !";
			LOG(WARNING, info);
			code = NOT_FOUND;
			goto END;
		}

		//get suffix
		found = http_request.path.rfind(".");
		if (found == std::string::npos)
			http_request.suffix = ".html";
		else
			http_request.suffix = http_request.path.substr(found);

		if (http_request.cgi)
			code = ProcessCGI();
		else
			code = ProcessNonCGI(size); //只用返回静态网页

	END:

		if (code != OK)
		{
			//.....
		}

		return;
	}

	void SendHttpResponse()
	{

		send(sock, http_response.status_line.c_str(), http_response.status_line.size(), 0); //发送响应行
		for (const auto &str : http_response.response_header)								//发送响应报头
			send(sock, str.c_str(), str.size(), 0);

		//测试
		// std::string send_type = "Content-Type: text/html";
		// send_type += LINE_END;
		// send(sock, send_type.c_str(), send_type.size(), 0);

		send(sock, http_response.blank.c_str(), http_response.blank.size(), 0); //发送响应空行
		//DEBUG
		std::cout << "fd " << http_response.fd << "sie:" << http_response.size << std::endl;
		sendfile(sock, http_response.fd, 0, http_response.size);
		close(http_response.fd);
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
