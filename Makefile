# variables
CC=gcc
CFLAGS=-std=c11 -Wall -Werror -g
LDLIBS=
MODULES=lexer.o interpret.o generator.o
TARGET=interpreter

# targets
all: $(TARGET)

clean:
	clear && rm -rf $(TARGET) $(MODULES) main.o

$(TARGET): $(MODULES) main.o
	make cppcheck
	$(CC) $(CFLAGS) $(MODULES) main.o $(LDLIBS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cppcheck:
	clear
	#cppcheck --enable=performance --error-exitcode=1 *.c

run:
	make all
	clear
	./$(TARGET)
