export CXX=xcrun --sdk macosx clang++
#export CXX=g++

export CFLAGS=-O3 -std=c++14 -I$(shell pwd)
export LDFLAGS=-O3 -flto
TARGET=hybrid_1d
SRCROOT=../../$(TARGET)

all:
	@make -eC $(SRCROOT) all

install: all
	cp $(SRCROOT)/$(TARGET) ./

clean:
	@make -eC $(SRCROOT) clean
	rm -f $(TARGET)
