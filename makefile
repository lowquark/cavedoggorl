
all: mvp wfc

OBJECTS := build/main.o \
					 build/gfx/draw.o \
					 build/gfx/gfx.o \
					 build/game/game.o \
					 build/game/view.o \
					 build/game/level.o \
	         build/game/AStar.o \
					 build/util/Log.o \
					 build/util/serial.o

wfc: wfc2.cpp
	clang++ -std=c++11 -Wall -g -o $@ $<

mvp: $(OBJECTS)
	clang++ -g -Wall -std=c++11 -o $@ $^ -lSDL2 -lGL -lGLEW -lGLU -lSDL2_image -lSDL2_ttf -lluajit-5.1

build/%.o: src/%.cpp
	@mkdir --parents $(@D)
	clang++ -g -Wall -std=c++11 -I src/ -DLOG_LEVEL=INFO -c -o $@ $<

