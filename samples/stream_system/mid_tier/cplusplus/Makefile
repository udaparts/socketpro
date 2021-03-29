all:
	g++ -o midtier -O3 -DNO_SHARED_SENDING_BUFFER ../../../../include/*.cpp *.cpp -ldl -std=c++11 -lstdc++ -pthread
	strip -s midtier
