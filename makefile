
mvp: main.cpp AStar.cpp draw.cpp
	clang++ -g -Wall -std=c++11 -o $@ $^ -lSDL2 -lGL -lGLEW -lGLU

