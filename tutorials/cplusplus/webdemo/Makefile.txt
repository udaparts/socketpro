all:
	g++ -o webdemo ../../../include/membuffer.cpp ../../../include/aserverw.cpp webpub.cpp -ldl -std=c++11 -lstdc++ -pthread
