CFLAGS=-g
MACFLAGS=-framework GLUT -framework OpenGL -framework Cocoa -framework OpenAL
STATIC = /usr/local/lib/libvorbis.a /usr/local/lib/libogg.a /usr/local/lib/libvorbisfile.a -lpng 
CC=gcc

all: Zombies

Zombies: main.o load_sound.o game.o vector2.o load_png.o sound_list.o
	$(CC) $(CFLAGS) $(STATIC) $(MACFLAGS) vector2.o sound_list.o load_png.o game.o load_sound.o main.o -o Zombies

load_sound.o: load_sound.c load_sound.h
	$(CC) $(CFLAGS) -c load_sound.c

game.o: game.c game.h load_sound.h load_png.h
	$(CC) $(CFLAGS) -c game.c

vector2.o: vector2.c vector2.h
	$(CC) $(CFLAGS) -c vector2.c

sound_list.o: sound_list.c sound_list.h vector2.h
	$(CC) $(CFLAGS) -c sound_list.c

load_png.o: load_png.h load_png.c
	$(CC) $(CFLAGS) -c load_png.c

main.o: main.c game.h
	$(CC) $(CFLAGS) -c main.c

clean:
	\rm *.o
	
