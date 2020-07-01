all:
	g++ -o sq_client ../../../../include/membuffer.cpp ../../../../include/aclientw.cpp sq_client.cpp -ldl -std=c++11 -lstdc++ -pthread
