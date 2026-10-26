CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra

todo: todo.cpp
	$(CXX) $(CXXFLAGS) -o todo todo.cpp

clean:
	rm -f todo
