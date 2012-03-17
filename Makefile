CFLAGS=-g
MACFLAGS=-framework GLUT -framework OpenGL -framework Cocoa -framework OpenAL
STATIC = -lsqlite3 -lvorbis -logg -lvorbisfile -lpng -lfreetype `sdl-config --cflags --libs` 

CC=gcc

all: Zombies CTags

Zombies: freetype_imp.o data_interface.o main.o physics.o load_sound.o game.o vector2.o load_png.o sound_list.o stream_sound.o
	$(CC) $(CFLAGS) $(STATIC) $(MACFLAGS) data_interface.o freetype_imp.o physics.o stream_sound.o vector2.o sound_list.o load_png.o game.o load_sound.o main.o -o Zombies

CTags: main.o
	ctags *.c *.h

load_sound.o: load_sound.c load_sound.h
	$(CC) $(CFLAGS) -c load_sound.c

game.o: game.c game.h vector2.h physics.h load_sound.h load_png.h
	$(CC) $(CFLAGS) -c game.c

vector2.o: vector2.c vector2.h
	$(CC) $(CFLAGS) -c vector2.c

freetype_imp.o: freetype_imp.c freetype_imp.h
	$(CC) $(CFLAGS) -c freetype_imp.c

data_interface.o: data_interface.c data_interface.h freetype_imp.h
	$(CC) $(CFLAGS) -c data_interface.c

physics.o: physics.c physics.h vector2.h
	$(CC) $(CFLAGS) -c physics.c 

sound_list.o: sound_list.c sound_list.h vector2.h
	$(CC) $(CFLAGS) -c sound_list.c

stream_sound.o: stream_sound.c stream_sound.h
	$(CC) $(CFLAGS) -c stream_sound.c

load_png.o: load_png.h load_png.c
	$(CC) $(CFLAGS) -c load_png.c

main.o: main.c game.h
	$(CC) $(CFLAGS) -c main.c

clean:
	\rm *.o
	
