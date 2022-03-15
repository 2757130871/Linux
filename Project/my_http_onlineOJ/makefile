all:httpserver cgi

httpserver:main.cc
	g++ main.cc -o httpserver -std=c++11  -pthread
cgi:cgi.cc oj_view.hpp oj_model.hpp 
	g++ cgi.cc -o cgi -std=c++11 -ljsoncpp -lctemplate -pthread -I /root/third_part/include -L /root/third_part/lib

.PHONY:clean
clean:
	rm httpserver ./wwwroot/cgi
# %.o:%.c
#     g++ -c -std=c++11 $< -o $@ -lpthread
# clean:
# rm -rf $(OBJS) $(TARGET)
