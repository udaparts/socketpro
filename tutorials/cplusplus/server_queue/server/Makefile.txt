all:
	g++ -o sq_server ../../../../include/membuffer.cpp ../../../../include/aserverw.cpp sq_server.cpp -ldl -std=c++11 -lstdc++ -pthread
