
mvp: main.cpp AStar.cpp draw.cpp game.cpp gfx.cpp Log.cpp
	clang++ -g -Wall -std=c++11 -o $@ $^ -lSDL2 -lGL -lGLEW -lGLU -lSDL2_image -lSDL2_ttf -DLOG_LEVEL=INFO

