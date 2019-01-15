export CXX=xcrun --sdk macosx clang++
#export CXX=g++

EXECUTABLE=hybrid_1d


CFLAGS=-O3 -std=c++14
LDFLAGS=-O3 -flto

SOURCES=$(shell find . -type f -name "*.cc")

OBJECTS=$(SOURCES:.cc=.o)


all: $(EXECUTABLE)

%.o : %.cc
	$(CXX) $(CFLAGS) -c -o $@  $<

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
