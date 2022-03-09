#pragma once
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

class Util
{
public:
	static int ReadLine(int sock, std::string &out)
	{

		char ch = 'X';
		while (ch != '\n')
		{
			ssize_t s = recv(sock, &ch, 1, 0);
			if (s > 0)
			{
				if (ch == '\r')
				{
					recv(sock, &ch, 1, MSG_PEEK); //窥探
					if (ch == '\n')
					{
						recv(sock, &ch, 1, 0);
					}
					else
					{
						ch = '\n';
					}
				}
				out += ch;
			}
			else if (s == 0)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}

		return 0;
	}

	static bool CurString(const std::string &target, std::string &key_out, std::string &value_out,const std::string &sep)
	{
		size_t pos = target.find(sep);
		if (pos != std::string::npos)
		{
			key_out = target.substr(0, pos);
			value_out = target.substr(pos + sep.size());
			return true;
		}
		
		return false;
	}

};
