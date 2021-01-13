# define the Cpp compiler to use
CXX = g++

# define any compile-time flags
CXXFLAGS	:= -std=c++17 -Wall -Wextra -g `pkg-config --libs --cflags dbus-1`

# define library paths in addition to /usr/lib
LFLAGS = 

# define output directory
OUTPUT	:= build

# define source directory
SRC		:= {src,include}

# define include directory
INCLUDE	:= {'include','/usr/include/dbus-1.0'}

MAIN	:= mpriscord
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))

# define the C object files 
OBJECTS		:= $(SOURCES:.cpp=.o)

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!