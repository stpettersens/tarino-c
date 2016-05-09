CC = tcc # Use Tiny C Compiler by default.
SWITCHES=
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
	SWITCHES = -s # Strip symbols.
	INCLUDE =
	LIB =
endif

make:
	$(CC) $(SWITCHES) $(INCLUDE) $(LIB) $(LLIB) $(SOURCES) -o $(TARGET)
