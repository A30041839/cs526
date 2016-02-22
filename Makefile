all: 
	g++ main.cpp graph.hpp utility.hpp mongoose.c -std=c++11 -o cs426_graph_server
clean:
	rm cs426_graph_server

all: main.cpp graph.hpp utility.hpp mongoose.c mongoose.h
