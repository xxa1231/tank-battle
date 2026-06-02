# Tank Battle - Makefile
CC      = D:/ASoftware/C/Dev-Cpp/MinGW64/bin/gcc.exe
CFLAGS  = -std=c99 -Wall -Wextra -O2
TARGET  = tank-battle.exe
SRC     = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	del /f $(TARGET) 2>nul || rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
