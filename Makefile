CC = tcc # Use Tiny C Compiler by default.
SWITCHES=
SOURCES = tarino.c dos2unix.c program.c
TARGET = tarino
LLIB =

ifeq ($(OS), Windows_NT)
	TARGET = tarino.exe
else
	LLIB = -lm
endif

ifeq ($(CC), gcc) # Use GCC.
	SWITCHES = -s # Strip symbols.
endif

make:
	$(CC) $(SWITCHES) $(LIB) $(SOURCES) -o $(TARGET) $(LLIB)

clean:
	rm -f $(TARGET)
