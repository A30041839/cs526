#Macro definitions
CXXFLAGS = -O1 -g -Wall -std=c++11
OBJ = main.o log.o graph.o mongoose.o
TARGET = cs426_graph_server

#Rules
all: $(TARGET)
$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ)
clean:
	rm -f $(OBJ) $(TARGET)

#Dependencies
main.o: main.cpp graph.hpp utility.hpp log.hpp types.hpp
log.o: log.hpp log.cpp graph.hpp types.hpp debug.hpp
graph.o: graph.hpp graph.cpp
mongoose.o: mongoose.c mongoose.h
