CXXFLAGS=-std=c++23 -Wall -Wextra -I./3rd_lib/spdlog-1.15.3/include
CXXFLAGS+=-DPASSCHECK
CXXFILES=main.cpp Experiment/*.cpp
CXXOPT=-O3

all:
	g++ $(CXXFILES) $(CXXFLAGS) $(CXXOPT) -o main.exe
	# g++ $(CXXFILES) $(CXXFLAGS) $(CXXOPT) -fopenmp -o main-omp.exe
	# g++ $(CXXFILES) $(CXXFLAGS) -pg -g -o main-pg.exe
	# ./main.exe