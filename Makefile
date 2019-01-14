export CXX=xcrun --sdk macosx clang++
#export CXX=g++

export CFLAGS=-O3 -std=c++14
export LDFLAGS=-O3 -flto

all:
	@make -eC ./hybrid_1d/hybrid_1d all

clean:
	@make -eC ./hybrid_1d/hybrid_1d clean
