CC = gcc
CFLAGS = $(shell pkg-config --cflags gtk+-3.0) -I.
LIBS = $(shell pkg-config --libs gtk+-3.0) -lSDL2 -lm

all: app.exe

app.exe: main.c animation.c
	$(CC) main.c animation.c -o app.exe $(CFLAGS) $(LIBS)

clean:
	rm -f app.exe
