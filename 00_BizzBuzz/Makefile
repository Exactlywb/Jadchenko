CC=gcc
CFLAGS=-c -Wall -Wextra

SOURCES=main.c BizzBuzz.c
EXECUTABLE=BizzBuzz

OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o main

main.o: main.c
	$(CC) $(FLAGS) main.c

BizzBuzz.o: BizzBuzz.c
	$(CC) $(CFLAGS) BizzBuzz.c

clean:
	rm -rf *.o BizzBuzz

