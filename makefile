
all: cavedoggorl

OBJECTS := build/main.o \
					 build/rf/game/Game.o \
					 build/rf/game/World.o \
					 build/rf/game/Level.o \
					 build/rf/game/worldgen.o \
					 build/rf/util/Log.o \
					 build/rf/util/Image.o \
					 build/rf/util/load_png.o \
					 build/rf/util/Dijkstra.o \
					 build/rf/util/FOV.o \
					 build/rf/gfx/gfx.o \
					 build/rf/gfx/draw.o \
					 build/rf/gfx/Scene.o \
					 build/rf/gfx/Tilemap.o \
					 build/rf/gfx/gl/Program.o \
					 build/rf/gfx/gl/Texture.o

wfc/wfc: wfc/wfc2.cpp
	clang++ -std=c++11 -Wall -g -o $@ $<

cavedoggorl: $(OBJECTS)
	clang++ -g -Wall -std=c++11 -o $@ $^ -lSDL2 -lGL -lGLEW -lGLU -lSDL2_image -lSDL2_ttf -lluajit-5.1 -lpng -lz

build/%.o: src/%.cpp
	@mkdir --parents $(@D)
	clang++ -g -Wall -std=c++11 -I src/ -c -o $@ $<

