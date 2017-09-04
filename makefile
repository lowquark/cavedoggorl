
all: mvp

OBJECTS := build/main.o \
					 build/rf/game/Game.o \
					 build/rf/game/FOV.o \
					 build/rf/util/Image.o \
					 build/rf/util/load_png.o \
					 build/rf/gfx/gfx.o \
					 build/rf/gfx/draw.o \
					 build/rf/gfx/TileMap.o \
					 build/rf/gfx/gl/Buffer.o \
					 build/rf/gfx/gl/Program.o \
					 build/rf/gfx/gl/Texture.o

wfc: wfc2.cpp
	clang++ -std=c++11 -Wall -g -o $@ $<

mvp: $(OBJECTS)
	clang++ -g -Wall -std=c++11 -o $@ $^ -lSDL2 -lGL -lGLEW -lGLU -lSDL2_image -lSDL2_ttf -lluajit-5.1 -lpng -lz

build/%.o: src/%.cpp
	@mkdir --parents $(@D)
	clang++ -g -Wall -std=c++11 -I src/ -DLOG_LEVEL=DEBUG3 -c -o $@ $<

