#pragma once

#include "llvm-21/llvm/Support/FormatVariadic.h"
#include <llvm-21/llvm/ADT/StringRef.h>
#include <llvm-21/llvm/ADT/Twine.h>
#include <llvm-21/llvm/Support/SourceMgr.h>
#include <string>

#include <llvm/ADT/SmallVector.h>
#include <vector>

#include "rheo/Diagnostics/diagnostic_kind.h"
#include "rheo/Diagnostics/source_location.h"

namespace rheo::diag {

using ArgList = llvm::SmallVector<std::string, 4>;

struct Diagnostic {
private:
  Kind kind;
  Level level;
  SourceRange range;
  std::string message;
  std::vector<Diagnostic> children;
  llvm::SmallVector<FixIt, 2> fixIts;
  llvm::SmallVector<std::string, 2> fixItDescriptions;

  static std::string formatMessage(llvm::StringRef fmt, const ArgList &args) {
    std::string format = fmt.str();

    switch (args.size()) {
    case 0:
      return format;
    case 1:
      return llvm::formatv(format.c_str(), args[0]).str();
    case 2:
      return llvm::formatv(format.c_str(), args[0], args[1]).str();
    case 3:
      return llvm::formatv(format.c_str(), args[0], args[1], args[2]).str();
    default:
      return llvm::formatv(format.c_str(), args[0], args[1], args[2], args[3])
          .str();
    }
  }

public:
  Diagnostic(Kind kind, SourceRange primaryRange, const ArgList &args = {})
      : kind(kind), level(infoFor(kind).defaultLevel), range(primaryRange),
        message(formatMessage(infoFor(kind).format, args)) {}

  Diagnostic &note(Kind kind, SourceRange range, const ArgList &args = {}) {
    children.emplace_back(kind, range, args);
    return *this;
  };

  Diagnostic &fixIt(SourceRange range, llvm::StringRef replacement,
                    llvm::StringRef description) {
    fixIts.push_back(FixIt{.range = range,
                           .replacement = replacement.str(),
                           .description = description.str()});
    return *this;
  }

  Diagnostic &setLevel(Level lvl) {
    level = lvl;
    return *this;
  }

  Kind getKind() const { return kind; }
  Level getLevel() const { return level; }
  const SourceRange &getRange() const { return range; }
  llvm::StringRef getMessage() const { return message; }
  llvm::ArrayRef<Diagnostic> getChildren() const { return children; }
  llvm::ArrayRef<FixIt> getfixIts() const { return fixIts; }
  llvm::StringRef name() const { return infoFor(kind).name; }
};

} // namespace rheo::diag
