/*
 * Copyright 2018 Nico Reißmann <nico.reissmann@gmail.com>
 * See COPYING for terms of redistribution.
 */

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/CallSite.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/SourceMgr.h>

#include <iostream>

class cmdline_options {
public:
	cmdline_options()
	: ifile("")
	, ofile("")
	{}

	std::string ifile;
	std::string ofile;
};

static void
parse_cmdline(int argc, char ** argv, cmdline_options & flags)
{
	using namespace llvm;

	/*
		FIXME: The command line parser setup is currently redone
		for every invocation of parse_cmdline. We should be able
		to do it only once and then reset the parser on every
		invocation of parse_cmdline.
	*/

	cl::TopLevelSubCommand->reset();

	cl::opt<bool> show_help(
	  "help"
	, cl::ValueDisallowed
	, cl::desc("Display available options."));

	cl::opt<std::string> ifile(
	  cl::Positional
	, cl::desc("<input>"));

	cl::opt<std::string> ofilepath(
	  "o"
	, cl::desc("Write output to <file>.")
	, cl::value_desc("file"));

	cl::ParseCommandLineOptions(argc, argv);

	if (show_help) {
		cl::PrintHelpMessage();
		exit(EXIT_SUCCESS);
	}

	if (ifile.empty()) {
		std::cerr << "No input file given.\n";
		exit(EXIT_FAILURE);
	}

	flags.ifile = ifile;
	if (!ofilepath.empty())
		flags.ofile = ofilepath;
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
	cmdline_options options;
	parse_cmdline(argc, argv, options);

	llvm::LLVMContext ctx;
	llvm::SMDiagnostic smd;
	auto module = llvm::parseIRFile(options.ifile, smd, ctx);
	if (!module) {
		smd.print(argv[0], llvm::errs());
		exit(EXIT_FAILURE);
	}

	strip(*module);

	if (options.ofile.empty()) {
		llvm::raw_os_ostream os(std::cout);
		module->print(os, nullptr);
	} else {
		std::error_code ec;
		llvm::raw_fd_ostream os(options.ofile, ec);
		module->print(os, nullptr);
	}

	return 0;
}
