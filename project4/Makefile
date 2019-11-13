.SUFFIXES: .c .o

CC=gcc

SRCDIR=src/
INC=include/
LIBS=lib/

# SRCS:=$(wildcard src/*.c)
# OBJS:=$(SRCS:.c=.o)

# main source file
TARGET_SRC:=$(SRCDIR)main.c
TARGET_OBJ:=$(SRCDIR)main.o

# Include more files if you write another source file.
SRCS_FOR_LIB:=$(SRCDIR)bpt.c
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.c=.o)
SRCS2_FOR_LIB:=$(SRCDIR)disk_space_manager.c
OBJS2_FOR_LIB:=$(SRCS2_FOR_LIB:.c=.o)
SRCS3_FOR_LIB:=$(SRCDIR)buffer_manager.c
OBJS3_FOR_LIB:=$(SRCS3_FOR_LIB:.c=.o)

CFLAGS+= -g -fPIC -I $(INC)

TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(CC) $(CFLAGS) -o $(OBJS_FOR_LIB) -c $(SRCS_FOR_LIB)
	$(CC) $(CFLAGS) -o $(OBJS2_FOR_LIB) -c $(SRCS2_FOR_LIB)
	$(CC) $(CFLAGS) -o $(OBJS3_FOR_LIB) -c $(SRCS3_FOR_LIB)
	make static_library
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB) $(OBJS2_FOR_LIB) $(OBJS3_FOR_LIB) $(LIBS)*

library:
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB) $(OBJS2_FOR_LIB) $(OBJS3_FOR_LIB)

static_library:
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB) $(OBJS2_FOR_LIB) $(OBJS3_FOR_LIB)
