all:
	g++ -std=c++20 -fcoroutines -o clientcpp -O3 ../../../include/membuffer.cpp ../../../include/aclientw.cpp clientcpp.cpp -ldl -pthread -static-libstdc++
	strip -s clientcpp
