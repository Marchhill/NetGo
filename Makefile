FILES = main.c board.c history.c display.c gui.c net.c utils.c glad.c lodepng.c
LIBS = -lm -lcrypto -ldl -lglfw -lpthread
TARGET = bin/go
INCLUDE = include

all: build
build:
	gcc $(FILES) $(LIBS) -I$(INCLUDE) -o $(TARGET)