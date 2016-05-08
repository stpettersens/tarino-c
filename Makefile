CC = tcc # Use Tiny C Compiler.
INCLUDE =
LIB =
SOURCES = tarino.c dos2unix.c program.c
TARGET = tarino
LLIB =

ifeq ($(OS), Windows_NT)
	TARGET = tarino.exe
	INCLUDE = -IC:\Dev\bin\include
	LIB = -LC:\Dev\bin\lib
else
	LLIB = -lm
endif

make:
	$(CC) $(INCLUDE) $(LIB) $(LLIB) $(SOURCES) -o $(TARGET)
