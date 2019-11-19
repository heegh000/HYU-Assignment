.SUFFIXES: .cc .o

CXX=g++

SRCDIR=src/
INC=include/
LIBS=lib/

# SRCS:=$(wildcard src/*.cc)
# OBJS:=$(SRCS:.cc=.o)

# main source file
TARGET_SRC:=$(SRCDIR)main.cc
TARGET_OBJ:=$(SRCDIR)main.o

# Include more files if you write another source file.
SRCS_FOR_LIB:=$(SRCDIR)bpt.cc
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.cc=.o)
SRCS2_FOR_LIB:=$(SRCDIR)disk_space_manager.cc
OBJS2_FOR_LIB:=$(SRCS2_FOR_LIB:.cc=.o)
SRCS3_FOR_LIB:=$(SRCDIR)buffer_manager.cc
OBJS3_FOR_LIB:=$(SRCS3_FOR_LIB:.cc=.o)
SRCS4_FOR_LIB:=$(SRCDIR)lock_manager.cc
OBJS4_FOR_LIB:=$(SRCS4_FOR_LIB:.cc=.o)

CXXFLAGS+= -pthread -g -fPIC -I $(INC)

TARGET=main

all: $(TARGET)

$(TARGET): $(TARGET_OBJ)
	$(CXX) $(CXXFLAGS) -o $(OBJS_FOR_LIB) -c $(SRCS_FOR_LIB)
	$(CXX) $(CXXFLAGS) -o $(OBJS2_FOR_LIB) -c $(SRCS2_FOR_LIB)
	$(CXX) $(CXXFLAGS) -o $(OBJS3_FOR_LIB) -c $(SRCS3_FOR_LIB)
	$(CXX) $(CXXFLAGS) -o $(OBJS4_FOR_LIB) -c $(SRCS4_FOR_LIB)
	make static_library
	$(CC) $(CXXFLAGS) -o $@ $^ -L $(LIBS) -lbpt

clean:
	rm $(TARGET) $(TARGET_OBJ) $(OBJS_FOR_LIB) $(OBJS2_FOR_LIB) $(OBJS3_FOR_LIB) $(OBJS4_FOR_LIB) $(LIBS)*

library:
	gcc -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB) $(OBJS2_FOR_LIB) $(OBJS3_FOR_LIB) $(OBJS4_FOR_LIB)

static_library:
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB) $(OBJS2_FOR_LIB) $(OBJS3_FOR_LIB) $(OBJS4_FOR_LIB)
