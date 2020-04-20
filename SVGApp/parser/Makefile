CFLAGS = -Wall -g -std=c11
BIN = bin/
INC = include/
SRC = src/
XML_PATH = /usr/include/libxml2
LDFLAGS= -L./bin -L.

#export LD_LIBRARY_PATH=LD_LIBRARY_PATH:.:./bin

# Standard common makefile
parser: ../libsvgparse.so

../libsvgparse.so: $(BIN)SVGParser.o $(BIN)LinkedListAPI.o
	gcc -shared -o ../libsvgparse.so $(BIN)SVGParser.o $(BIN)LinkedListAPI.o -lxml2 -lm

$(BIN)SVGParser.o: $(SRC)SVGParser.c $(INC)LinkedListAPI.h $(INC)SVGParser.h
	gcc -c -fpic $(CFLAGS) -I$(XML_PATH) -I$(INC) $(SRC)SVGParser.c -o $(BIN)SVGParser.o

$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	gcc -c -fpic $(CFLAGS) -I$(INC) $(SRC)LinkedListAPI.c -o $(BIN)LinkedListAPI.o

main: $(BIN)main.o $(BIN)libsvgparse.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o main $(BIN)main.o -Iinclude -lsvgparse -lxml2

$(BIN)main.o: $(SRC)main.c
	$(CC) $(CFLAGS) -I$(XML_PATH) -Ibin -Iinclude -c $(SRC)main.c -o $(BIN)main.o -lxml2

xmlExample: $(SRC)/libXmlExample.c
	$(CC) $(CFLAGS) -I$(XML_PATH) $(SRC)/libXmlExample.c -lxml2 -o xmlExample

clean:
	rm $(BIN)*.o $(BIN)*.so main
