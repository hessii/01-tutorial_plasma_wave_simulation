export CXX=xcrun --sdk macosx clang++
#export CXX=g++

export CFLAGS=-O3 -std=c++17 -I$(shell pwd)
export LDFLAGS=-O3 -flto


TARGET=pic_1d
SRCROOT=../../$(TARGET)

all:
	@make -eC $(SRCROOT) all
	cp $(SRCROOT)/$(TARGET) ./
	@make -eC $(SRCROOT) clean

clean:
	@make -eC $(SRCROOT) clean
	rm -f $(TARGET)
