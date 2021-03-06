# Copyright 2018 Nico Reißmann <nico.reissmann@gmail.com>
# See COPYING for terms of redistribution.
define HELP_TEXT
clear
echo "Makefile for the llvm-strip tool"
echo "Version 1.0 - 2019-06-18"
endef

.PHONY: help
help:
	@$(HELP_TEXT)
	@$(HELP_TEXT_LLVMSTRIP)

LLVMSTRIP_ROOT ?= .
include $(LLVMSTRIP_ROOT)/Makefile.sub

.PHONY: all
all: llvm-strip

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean
clean: llvm-strip-clean
