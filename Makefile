# Copyright 2018 Nico Reißmann <nico.reissmann@gmail.com>
# See COPYING for terms of redistribution.

LLVMCONFIG = llvm-config

CPPFLAGS = -I$(shell $(LLVMCONFIG) --includedir)
CXXFLAGS = -Wall -std=c++14 -Wfatal-errors
LDFLAGS = $(shell $(LLVMCONFIG) --ldflags --system-libs --libs irReader)

LLVMSTRIP_SRC = \
	src/main.cpp

all: llvm-strip

llvm-strip: $(patsubst %.cpp, %.o, $(LLVMSTRIP_SRC))
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

clean:
	find . -name "*.o" | xargs rm -rf
	rm -rf llvm-strip
