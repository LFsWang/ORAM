CXXFLAGS=-std=c++23 -Wall -Wextra -O2 -I./3rd_lib/spdlog-1.15.3/include
CXXFLAGS+=-DPASSCHECK

all:
	g++ main.cpp $(CXXFLAGS) -o main.exe
	g++ main.cpp $(CXXFLAGS) -fopenmp -o main-omp.exe
	# ./main.exe