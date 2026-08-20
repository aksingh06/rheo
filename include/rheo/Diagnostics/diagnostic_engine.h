#pragma once
#include "diagnostic.h"
#include "renderer.h"
#include "llvm/ADT/DenseMap.h"

namespace rheo::diag {

class DiagnosticEngine {
public:
  explicit DiagnosticEngine(TerminalRenderer renderer)
      : renderer(std::move(renderer)) {}

  void setLevelOverride(Kind kind, Level lvl) {
    overrides[static_cast<uint16_t>(kind)] = lvl;
  }

  void setWarningsAsErrors(bool enabled) { warnings_as_errors = enabled; }

  void emit(Diagnostic d) {
    Level effective = resolveLevel(d.getKind(), d.getLevel());
    d.setLevel(effective);

    switch (effective) {
    case Level::kError:
    case Level::kFatal:
      ++error_count;
      break;
    case Level::kWarning:
      ++warning_count;
      break;
    default:
      break;
    }

    renderer.render(d);

    if (effective == Level::kFatal) {
      fatal_hit = true;
    }
  }

  bool hasErrors() const { return error_count > 0; }
  bool hasFatal() const { return fatal_hit; }
  unsigned errorCount() const { return error_count; }
  unsigned warningCount() const { return warning_count; }

private:
  Level resolveLevel(Kind kind, Level original) const {
    auto it = overrides.find(static_cast<uint16_t>(kind));
    if (it != overrides.end()) {
      return it->second;
    }
    if (warnings_as_errors && original == Level::kWarning) {
      return Level::kError;
    }
    return original;
  }

  TerminalRenderer renderer;
  llvm::DenseMap<uint16_t, Level> overrides;
  bool warnings_as_errors = false;
  unsigned error_count = 0;
  unsigned warning_count = 0;
  bool fatal_hit = false;
};

} // namespace rheo::diag
