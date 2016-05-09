CC = tcc # Use Tiny C Compiler by default.
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

ifeq ($(CC), gcc) # Use GCC.
	INCLUDE =
	LIB =
endif

make:
	$(CC) $(INCLUDE) $(LIB) $(LLIB) $(SOURCES) -o $(TARGET)
