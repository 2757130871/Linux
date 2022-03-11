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

#define LINE_END "\r\n"
#define HOME_PAGE "index.html"
#define WEB_ROOT "wwwroot"

#define HTTP_VERSION "HTTP/1.0"

#define PAGE_404 "404.html"
#define PAGE_400 "400.html"
#define PAGE_500 "500.html"

#define OK 200
#define NOT_FOUND 404
#define BAD_REQUEST 400
#define SERVER_ERROR 500

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
	case 400:
		desc = "Bad Request";
		break;
	case 500:
		desc = "Server Error";
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
	int size;
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
	bool stop = false;
	HttpRequest http_request;
	HttpResponse http_response;

private:
	bool RecvHttpRequestLine()
	{
		if (Util::ReadLine(sock, http_request.request_line))
		{
			std::string &line = http_request.request_line;
			line.resize(line.size() - 1);
			LOG(INFO, line);
		}
		else
		{
			stop = true;
		}

		return stop;
	}

	bool RecvHttpRequestHeader()
	{
		std::string line;
		while (1)
		{
			line.clear();
			if (Util::ReadLine(sock, line) <= 0)
			{
				stop = true;
				break;
			}

			if (line == "\n")
			{
				http_request.blank = line;
				break;
			}
			line.resize(line.size() - 1);
			http_request.request_header.push_back(line);
			//LOG(INFO, line);
		}
		return stop;
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

	bool RecvHttpRequestBody()
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
					stop = true;
					break;
				}
			}

			//std::cerr << "DEBUG" << body << std::endl;
		} //end of if
		return stop;
	}

	//返回静态网页
	int ProcessNonCGI()
	{
		http_response.fd = open(http_request.path.c_str(), O_RDONLY);

		if (http_response.fd > 0)
			return OK;

		LOG(ERROR, "open file error");
		return NOT_FOUND;
	}

	//处理CGI
	int ProcessCGI()
	{
		int code = OK;
		auto method = http_request.method;
		auto query_string = http_request.query_string; //GET
		auto body_text = http_request.request_body;	   //POST
		auto &bin = http_request.path;
		auto &response_body = http_response.response_body;

		std::string query_string_env;
		std::string method_env;
		std::string content_length_env;

		int input[2];
		int output[2];
		if (pipe(input) < 0)
		{
			LOG(ERROR, "pipe input error");
			return code = SERVER_ERROR;
		}
		if (pipe(output) < 0)
		{
			LOG(ERROR, "pipe output error");
			return code = SERVER_ERROR;
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

			content_length_env = "CONTENT_LENGTH=";
			content_length_env += std::to_string(http_request.content_length);
			putenv((char *)content_length_env.c_str());

			if (method == "GET") //添加参数
			{
				query_string_env = "QUERY_STRING=";
				query_string_env += query_string;
				putenv((char *)query_string_env.c_str());
				//std::cout << "add env query " << std::endl;
			}

			//站在子进程角度 子进程只要向fd0读，向fd1写
			dup2(output[0], 0);
			dup2(input[1], 1);

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
			int size = 0;
			if (method == "POST") //持续写管道
			{
				const char *start = body_text.c_str();
				int total = 0;

				while ((size = write(output[1], start + total, body_text.size() - total)) > 0)
				{
					total += size;
				}
			}

			//接收CGI处理返回
			char ch = 0;
			while (read(input[0], &ch, 1) > 0)
				response_body.push_back(ch);

			std::cerr << "CGI Parent DEBUG: CGI 回传 = " << response_body << std::endl;

			int status = 0;
			pid_t ret = waitpid(pid, &status, 0);
			if (ret == pid)
			{
				if (WIFEXITED(status))
				{
					if (WEXITSTATUS(status) == 0) //正常退出
						code = OK;
					else
						code = BAD_REQUEST;
				}
				else
				{
					code = SERVER_ERROR;
				}
			}

			close(input[0]);
			close(output[1]);
		}

		return code;
	}

public:
	EndPoint(int _sock) : sock(_sock)
	{
	}
	~EndPoint()
	{
		close(sock);
	}

	bool Stop()
	{
		return stop;
	}
	void HandlerError(const std::string &page) //返回404页面
	{
		http_request.cgi = false; //只要出错就设置CGI false

		http_response.fd = open(page.c_str(), O_RDONLY);
		if (http_response.fd > 0)
		{
			struct stat st;
			stat(page.c_str(), &st);
			http_request.size = st.st_size;

			std::string line = "Content-Type: text/html";
			line += LINE_END;
			http_response.response_header.push_back(line);

			line = " Content-Length: ";
			line += std::to_string(st.st_size);
			line += LINE_END;
			http_response.response_header.push_back(line);
		}
	}

	void BuildOkResponse() //构建200 OK 响应报头
	{
		std::string line = "Content-Type: ";
		line += Suffix2Desc(http_request.suffix);
		line += LINE_END;
		http_response.response_header.push_back(line);
		line = "Content-Length: ";
		if (http_request.cgi)
			line += std::to_string(http_response.response_body.size());
		else
			line += std::to_string(http_request.size); //Get ???疑问

		line += LINE_END;
		http_response.response_header.push_back(line);
	}

	void BuildHttpResponseHelper() //返回错误页面
	{
		//http_request;//http_response;
		int code = http_response.status_code;
		//构建状态行
		auto &status_line = http_response.status_line;
		status_line += HTTP_VERSION;
		status_line += " ";
		status_line += std::to_string(code);
		status_line += " ";
		status_line += Code2Desc(code);
		status_line += LINE_END;

		std::cerr << "DEBUG: " << status_line << "  code: " << code << std::endl;

		std::string path = WEB_ROOT;
		path += "/";
		//构建响应正文
		switch (code)
		{
		case 200:
			BuildOkResponse();
			break;
		case 404:
			path += PAGE_404;
			HandlerError(path);
			break;
		case 400:
			path += PAGE_400;
			HandlerError(path);
			break;
		case 500:
			path += PAGE_500;
			HandlerError(path);
			break;
		default:
			break;
		}
	}

	//step1  receive and   parse
	void RcvHttpRequest()
	{
		//读取请求行和请求头都成功才用进行后续处理
		if ((!RecvHttpRequestLine()) && (!RecvHttpRequestHeader()))
		{
			ParseHttpRequestLine();
			ParseHttpRequestHeader();
			RecvHttpRequestBody();
		}
		else
		{
			LOG(WARNING, "+++++ client request read error ! +++++");
		}
	}

	//step2  Build response
	void BuildHttpResponse()
	{

		std::string tmp_path;
		int size = 0; //保存请求文件的大小
		size_t found = 0;
		auto &code = http_response.status_code;
		if (http_request.method != "GET" && http_request.method != "POST")
		{
			//非法请求
			LOG(WARNING, "request method != GET && != POST");
			code = BAD_REQUEST;
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
		if (stat(http_request.path.c_str(), &st) == 0) //进入说明资源是存在的
		{

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
			//http_response.size = size;
			http_request.size = size;
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
			code = ProcessNonCGI(); //只用返回静态网页

	END:

		BuildHttpResponseHelper(); //处理页面
	}
	//step3 send data
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
		std::cout << "fd " << http_response.fd << "send size:" << http_response.size << std::endl;

		if (http_request.cgi)
		{
			auto &response_body = http_response.response_body;
			size_t size = 0;
			size_t total = 0;
			const char *start = response_body.c_str();

			while (total < response_body.size() //发送响应正文
				   && (size = send(sock, start + total, response_body.size() - total, 0)) > 0)
				total += size;
		}
		else
		{
			sendfile(sock, http_response.fd, 0, http_request.size);
		}
		close(http_response.fd);
	}
};

class CallBack
{
public:
	void operator()(int sock)
	{
		HandlerRequest(sock);
	}

	static void HandlerRequest(int sock)
	{

		// std::cout << "get a new link ... : " << sock << std::endl;

		EndPoint *ep = new EndPoint(sock);
		ep->RcvHttpRequest();
		if (!ep->Stop())
		{
			ep->BuildHttpResponse();
			ep->SendHttpResponse();
		}
		else
		{
			LOG(WARNING, "Entrance DEBUG :RECV Error");
		}

		delete ep;
	}
};
	