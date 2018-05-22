CC = gcc
CFLAGS = -Wall -g

TARGET = ircbot

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

