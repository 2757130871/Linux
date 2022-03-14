oj_server:oj_server.cc oj_model.hpp oj_view.hpp
	g++ oj_server.cc -std=c++11 -o oj_server -ljsoncpp -lctemplate -pthread -I /root/third_part/include -L /root/third_part/lib
PHONY:clean
clean:
	rm oj_server
#test:test.cc
#	g++ test.cc -o test  -std=c++11   -ljsoncpp -lpthread