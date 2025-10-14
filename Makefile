all:
	g++ main.cpp -Wall -Wextra -O2 -I./3rd_lib/spdlog-1.15.3/include -o main.exe
	./main.exe