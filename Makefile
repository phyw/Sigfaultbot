CFLAGS = -O0 -pipe

CFLAGS += -Wall -pedantic -std=c99 `pkg-config --cflags gthread-2.0 glib-2.0 loudmouth-1.0`
LDFLAGS += `pkg-config --libs gthread-2.0 glib-2.0 loudmouth-1.0` #-pthread
FUNCFLAGS = -D_XOPEN_SOURCE=500
DEBUG = -DDEBUG -g
OBJ = main.o xmpp.o util.o commands.o
WIPE_OBJ = wipe.o util.o xmpp.o commands.o

all: losbot

losbot: $(OBJ)
	gcc -o beatbot $(OBJ) $(LDFLAGS) $(DEBUG)  
main.o: main.c
	gcc -c main.c -o main.o $(CFLAGS) $(DEBUG) $(FUNCFLAGS)
xmpp.o: xmpp.c
	gcc -c xmpp.c -o xmpp.o $(CFLAGS) $(DEBUG) $(FUNCFLAGS)
util.o: util.c
	gcc -c util.c -o util.o $(CFLAGS) $(DEBUG) $(FUNCFLAGS)
commands.o: commands.c
	gcc -c commands.c -o commands.o $(CFLAGS) $(DEBUG) $(FUNCFLAGS)

wipe: $(WIPE_OBJ)
	gcc -o wipe $(WIPE_OBJ) $(LDFLAGS) $(DEBUG)
wipe.o: wipe.c
	gcc -c wipe.c -o wipe.o $(CFLAGS) $(DEBUG) $(FUNCFLAGS)

clean:
	rm -vf losbot $(OBJ)
	rm -vf wipe $(WIPE_OBJ)

