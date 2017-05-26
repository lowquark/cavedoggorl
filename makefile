
all: mvp wfc

OBJECTS := build/main.o \
					 build/gfx/gfx.o \
					 build/gfx/draw.o \
					 build/gfx/TileMap.o \
					 build/gfx/gl/Buffer.o \
					 build/gfx/gl/Program.o \
					 build/gfx/gl/Texture.o \
					 build/game/game.o \
					 build/game/core.o \
					 build/game/parts.o \
					 build/game/view.o \
	         build/game/AStar.o \
	         build/game/FOV.o \
					 build/util/Log.o \
					 build/util/serial.o \
					 build/util/Image.o \
					 build/util/load_png.o

wfc: wfc2.cpp
	clang++ -std=c++11 -Wall -g -o $@ $<

mvp: $(OBJECTS)
	clang++ -pg -g -O3 -Wall -std=c++11 -o $@ $^ -lSDL2 -lGL -lGLEW -lGLU -lSDL2_image -lSDL2_ttf -lluajit-5.1 -lpng -lz

build/%.o: src/%.cpp
	@mkdir --parents $(@D)
	clang++ -pg -g -O3 -Wall -std=c++11 -I src/ -DLOG_LEVEL=DEBUG3 -c -o $@ $<

