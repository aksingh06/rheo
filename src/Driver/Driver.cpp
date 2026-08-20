#include "rheo/Driver/Driver.h"
#include <expected>
#include <llvm-21/llvm/ADT/StringRef.h>
#include <llvm-21/llvm/Support/CommandLine.h>
#include <llvm-21/llvm/Support/ErrorOr.h>
#include <llvm-21/llvm/Support/raw_ostream.h>
#include <string>

namespace rheo {

namespace {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
llvm::cl::opt<std::string> inputFilename(llvm::cl::Positional,
                                         llvm::cl::Required,
                                         llvm::cl::desc("input file"));

} // namespace

std::expected<std::string, std::string>
readFileContents(llvm::StringRef &path) {}

void Driver::run(int argc, const char *const *argv) {
  llvm::cl::ParseCommandLineOptions(argc, argv,
                                    "Rheo programming language compiler\n");
  llvm::cl::SetVersionPrinter(
      [](llvm::raw_ostream &os) { os << "rheoc 0.1.0\n"; });

  llvm::outs() << inputFilename << '\n';
}

} // namespace rheo
