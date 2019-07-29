CXX=xcrun --sdk macosx clang++
#CXX=g++

CFLAGS=-O3 -std=c++17 -I.
LDFLAGS=-O3 -flto


EXECUTABLE=pic_1d

SOURCES=$(shell find . -type f -name "*.cc")

OBJECTS=$(SOURCES:.cc=.o)


all: $(EXECUTABLE)

%.o : %.cc
	$(CXX) $(CFLAGS) -c -o $@  $<

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
