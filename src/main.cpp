/*
 * Copyright 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/CallSite.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/SourceMgr.h>

#include <iostream>

static void
print_help(const char * app)
{
	std::cerr << "Usage: " << app << " FILE\n";
}

static void
strip_metadata(llvm::GlobalObject & go)
{
	llvm::SmallVector<std::pair<unsigned, llvm::MDNode*>, 32> mdvector;
	go.getAllMetadata(mdvector);
	for (const auto & md : mdvector)
		go.setMetadata(md.first, nullptr);
}

static void
strip_metadata(llvm::Instruction & i)
{
	llvm::SmallVector<std::pair<unsigned, llvm::MDNode*>, 32> mdvector;
	i.getAllMetadata(mdvector);
	for (const auto & md : mdvector)
		i.setMetadata(md.first, nullptr);
}

static void
strip(llvm::Module & module)
{
	for (auto & function : module) {
		/* strip function, parameter, and return value attributes */
		function.setAttributes({});

		strip_metadata(function);

		for (auto & bb : function) {
			for (auto & instruction : bb) {
				strip_metadata(instruction);

				if (auto cs = llvm::dyn_cast<llvm::CallInst>(&instruction))
					cs->setAttributes({});
			}
		}
	}

	for (auto & gv : module.getGlobalList())
		strip_metadata(gv);
}

int
main(int argc, char * argv[])
{
	if (argc < 2) {
		print_help(argv[0]);
		exit(1);
	}

	llvm::LLVMContext ctx;
	llvm::SMDiagnostic smd;
	auto module = llvm::parseIRFile(argv[1], smd, ctx);
	if (!module) {
		smd.print(argv[0], llvm::errs());
		exit(1);
	}

	strip(*module);
	module->dump();

	return 0;
}
