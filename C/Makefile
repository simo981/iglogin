CFLAGS = -O3
CC = clang
TARGET = iglogin
OBJ = main.o
UNAME = $(shell uname)
.PHONY = clean
LINKER = -lpthread -lcurl

ifeq ($(UNAME), Linux)
	LINKER += -lrt
	CC = gcc
endif

$(TARGET): $(OBJ)
	$(CC) $^ $(LINKER) -o $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o $(TARGET) *.txt *_state
