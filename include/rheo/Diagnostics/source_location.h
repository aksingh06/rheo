#pragma once

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include <cstdint>
#include <llvm-21/llvm/ADT/Twine.h>
#include <llvm-21/llvm/Support/SMLoc.h>
#include <llvm/ADT/StringRef.h>
#include <string>
namespace rheo::diag {

struct SourceLoc {
  std::uint32_t fileId = 0;
  std::uint32_t offset = 0;
  bool isValid() const { return fileId != 0; }
  friend bool operator==(const SourceLoc &, const SourceLoc &) = default;
  friend bool operator<(const SourceLoc &a, const SourceLoc &b) {
    return a.fileId != b.fileId ? a.fileId < b.fileId : a.offset < b.offset;
  }
};

struct SourceRange {
  SourceRange(SourceLoc start, SourceLoc end) : start(start), end(end) {}
  SourceLoc start;
  SourceLoc end;
  bool isValid() const { return start.isValid() && end.isValid(); }
};

struct FixIt {
  SourceRange range;
  std::string replacement;
  std::string description;
};

struct ResolvedLoc {
  llvm::StringRef filename;
  uint32_t line = 0;
  uint32_t column = 0;
};

class SourceManager {
public:
  uint32_t addBuffer(const std::string &filename) {
    auto buf = llvm::MemoryBuffer::getFile(filename);

    if (!buf)
      return 0;

    auto id = llvmMgr.AddNewSourceBuffer(std::move(*buf), llvm::SMLoc());

    return static_cast<uint32_t>(id);
  }

  ResolvedLoc resolve(SourceLoc loc) const {
    if (!loc.isValid()) {
      return {};
    }
    auto smLoc = toSMLoc(loc);
    auto lineCol = llvmMgr.getLineAndColumn(smLoc);
    auto id = llvmMgr.getMemoryBuffer(loc.fileId)->getBufferIdentifier();
    return {.filename = llvm::StringRef(id.begin(), id.size()),
            .line = static_cast<uint32_t>(lineCol.first),
            .column = static_cast<uint32_t>(lineCol.second)};
  }

  llvm::StringRef lineText(SourceLoc loc) const {
    if (!loc.isValid()) {
      return {};
    }
    const llvm::MemoryBuffer *buf = llvmMgr.getMemoryBuffer(loc.fileId);
    llvm::StringRef contents = buf->getBuffer();

    llvm::SMLoc smLoc = toSMLoc(loc);
    const char *lineStart = smLoc.getPointer();
    while (lineStart > contents.begin() && lineStart[-1] != '\n') {
      --lineStart;
    }
    const char *lineEnd = smLoc.getPointer();
    while (lineEnd < contents.end() && *lineEnd != '\n') {
      ++lineEnd;
    }
    return {lineStart, static_cast<size_t>(lineEnd - lineStart)};
  }

  llvm::SourceMgr &llvmSourceMgr() { return llvmMgr; }

private:
  llvm::SMLoc toSMLoc(SourceLoc loc) const {
    const llvm::MemoryBuffer *buf = llvmMgr.getMemoryBuffer(loc.fileId);
    return llvm::SMLoc::getFromPointer(buf->getBufferStart() + loc.offset);
  }
  llvm::SourceMgr llvmMgr;
};

} // namespace rheo::diag
