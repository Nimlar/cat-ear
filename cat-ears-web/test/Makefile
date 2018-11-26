CXX_CFLAGS=-std=c++11 -Wall -Wextra

ifdef MOVE
   CXX_CFLAGS += -DMOVE=$(MOVE)
endif

all:gcc clang
gcc:
	g++ $(CXX_CFLAGS) emul.cpp -o emul-$@

clang:
	clang++ $(CXX_CFLAGS) -Weverything -Wno-missing-prototypes  -Wno-c99-extensions -Wno-c++98-compat emul.cpp -o emul-$@

