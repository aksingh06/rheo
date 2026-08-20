#pragma once
#include "rheo/Diagnostics/diagnostic.h"
#include "rheo/Diagnostics/diagnostic_kind.h"
#include "rheo/Diagnostics/source_location.h"
#include <cstdint>
#include <cstdlib>
#include <llvm-21/llvm/ADT/StringRef.h>
#include <llvm-21/llvm/Support/raw_ostream.h>
#include <string>

namespace rheo::diag {

inline llvm::StringRef levelTag(Level lvl) {
  switch (lvl) {
  case Level::kNote:
    return "note";
  case Level::kRemark:
    return "remark";
  case Level::kWarning:
    return "warning";
  case Level::kError:
    return "error";
  case Level::kFatal:
    return "fatal error";
  }
  return "?";
}

enum class Theme : std::uint8_t {
  kDefault,
  kKingfisher,
  kGenZ,
};

struct RGB {
  uint8_t r, g, b;
};

struct Palette {
  RGB note;
  RGB remark;
  RGB warning;
  RGB error;
  RGB gutter;
  RGB fixit;
  RGB dim;
  RGB heading;
};

inline const Palette &paletteFor(Theme theme) {
  static const Palette kDefault{
      .note = {.r = 86, .g = 182, .b = 194},
      .remark = {.r = 198, .g = 120, .b = 221},
      .warning = {.r = 209, .g = 154, .b = 102},
      .error = {.r = 224, .g = 108, .b = 117},
      .gutter = {.r = 86, .g = 182, .b = 194},
      .fixit = {.r = 152, .g = 195, .b = 121},
      .dim = {.r = 92, .g = 99, .b = 112},
      .heading = {.r = 220, .g = 223, .b = 228},
  };

  static const Palette kKingfisher{
      .note = {.r = 42, .g = 169, .b = 160},
      .remark = {.r = 110, .g = 180, .b = 175},
      .warning = {.r = 217, .g = 164, .b = 65},
      .error = {.r = 232, .g = 130, .b = 60},
      .gutter = {.r = 42, .g = 169, .b = 160},
      .fixit = {.r = 232, .g = 130, .b = 60},
      .dim = {.r = 90, .g = 105, .b = 104},
      .heading = {.r = 230, .g = 236, .b = 235},
  };

  static const Palette kGenZ{
      .note = {.r = 0, .g = 224, .b = 255},
      .remark = {.r = 178, .g = 102, .b = 255},
      .warning = {.r = 255, .g = 214, .b = 10},
      .error = {.r = 255, .g = 45, .b = 149},
      .gutter = {.r = 178, .g = 102, .b = 255},
      .fixit = {.r = 57, .g = 255, .b = 20},
      .dim = {.r = 140, .g = 140, .b = 160},
      .heading = {.r = 255, .g = 255, .b = 255},
  };

  switch (theme) {
  case Theme::kKingfisher:
    return kKingfisher;
  case Theme::kGenZ:
    return kGenZ;
  case Theme::kDefault:
  default:
    return kDefault;
  }
}

inline Theme themeFromEnv() {
  if (const char *env = std::getenv("RHEO_DIAG_THEME")) {
    llvm::StringRef v(env);
    if (v.equals_insensitive("kingfisher")) {
      return Theme::kKingfisher;
    }
    if (v.equals_insensitive("genz") || v.equals_insensitive("gen-z")) {
      return Theme::kGenZ;
    }
  }
  return Theme::kDefault;
}

inline const RGB &levelColor(Level lvl, const Palette &pal) {
  switch (lvl) {
  case Level::kNote:
    return pal.note;
  case Level::kRemark:
    return pal.remark;
  case Level::kWarning:
    return pal.warning;
  case Level::kError:
  case Level::kFatal:
    return pal.error;
  }
  return pal.heading;
}

inline bool levelIsBoldCaret(Level lvl) {
  return lvl == Level::kError || lvl == Level::kFatal || lvl == Level::kWarning;
}

inline void setFg(llvm::raw_ostream &out, const RGB &c, bool bold = false) {
  out << "\x1b[" << (bold ? "1;" : "") << "38;2;" << (int)c.r << ";" << (int)c.g
      << ";" << (int)c.b << "m";
}

inline void resetFg(llvm::raw_ostream &out) { out << "\x1b[0m"; }

class TerminalRenderer {
private:
  SourceManager *sm;
  llvm::raw_ostream *out;
  Theme theme;
  const Palette *pal;

  uint32_t numErrors = 0;
  uint32_t numWarnings = 0;

  void tallyLevel(Level lvl) {
    if (lvl == Level::kError || lvl == Level::kFatal) {
      ++numErrors;
    } else if (lvl == Level::kWarning) {
      ++numWarnings;
    }
  }

  void renderImpl(const Diagnostic &d, int depth) {
    std::string indent(static_cast<std::size_t>(depth) * 2, ' ');
    const auto &r = d.getRange();
    auto loc = sm->resolve(r.start);
    auto lvl = d.getLevel();
    llvm::StringRef filename(loc.filename.data(), loc.filename.size());

    if (depth == 0) {
      tallyLevel(lvl);
    }

    *out << indent;
    setFg(*out, pal->heading, /*bold=*/true);
    *out << filename << ":" << loc.line << ":" << loc.column << ":";
    resetFg(*out);
    *out << " ";
    setFg(*out, levelColor(lvl, *pal), /*bold=*/true);
    *out << levelTag(lvl) << ":";
    resetFg(*out);
    *out << " ";
    setFg(*out, pal->heading, /*bold=*/true);
    *out << d.getMessage();
    resetFg(*out);

    *out << " ";
    setFg(*out, pal->dim, /*bold=*/false);
    *out << "[" << d.name() << "]";
    resetFg(*out);
    *out << "\n";

    auto text = sm->lineText(r.start);
    auto lineNoStr = std::to_string(loc.line);
    std::string gutter(lineNoStr.size(), ' ');

    *out << indent;
    setFg(*out, pal->gutter, /*bold=*/false);
    *out << " " << lineNoStr << " | ";
    resetFg(*out);
    *out << llvm::StringRef(text.data(), text.size()) << "\n";

    *out << indent;
    setFg(*out, pal->gutter, /*bold=*/false);
    *out << " " << gutter << " | ";
    resetFg(*out);
    for (uint32_t c = 1; c < loc.column; ++c) {
      *out << ' ';
    }

    bool bold = levelIsBoldCaret(lvl);
    setFg(*out, levelColor(lvl, *pal), bold);
    auto endLoc = sm->resolve(r.end);
    auto endCol = (r.end.isValid() && endLoc.line == loc.line &&
                   endLoc.column > loc.column)
                      ? endLoc.column
                      : loc.column + 1;
    *out << '^';
    for (auto c = loc.column + 1; c < endCol; ++c) {
      *out << '~';
    }
    resetFg(*out);
    *out << '\n';

    bool spanIsSingleLine = endLoc.line == loc.line;
    for (const FixIt &fx : d.getfixIts()) {
      if (fx.replacement.empty()) {
        continue;
      }
      bool replacementIsMultiLine =
          fx.replacement.find('\n') != std::string::npos;
      if (!spanIsSingleLine || replacementIsMultiLine) {
        continue;
      }

      *out << indent;
      setFg(*out, pal->gutter, /*bold=*/false);
      *out << " " << gutter << " | ";
      resetFg(*out);
      for (uint32_t c = 1; c < loc.column; ++c) {
        *out << ' ';
      }
      setFg(*out, pal->fixit, /*bold=*/true);
      *out << fx.replacement;
      resetFg(*out);
      *out << '\n';
    }

    for (const FixIt &fx : d.getfixIts()) {
      *out << indent;
      setFg(*out, pal->fixit, /*bold=*/true);
      *out << "  help: ";
      resetFg(*out);
      setFg(*out, pal->fixit, /*bold=*/false);
      *out << fx.description;
      resetFg(*out);
      *out << "\n";

      bool replacementIsMultiLine =
          fx.replacement.find('\n') != std::string::npos;
      bool alreadyShownInline = spanIsSingleLine && !replacementIsMultiLine;
      if (!fx.replacement.empty() && !alreadyShownInline) {
        *out << indent;
        setFg(*out, pal->fixit, /*bold=*/true);
        *out << "      + " << fx.replacement;
        resetFg(*out);
        *out << "\n";
      }
    }

    for (const Diagnostic &child : d.getChildren()) {
      renderImpl(child, depth + 1);
    }

    if (depth == 0) {
      *out << "\n";
    }
  }

public:
  TerminalRenderer(SourceManager &sm, llvm::raw_ostream &out = llvm::errs(),
                   Theme theme = themeFromEnv())
      : sm(&sm), out(&out), theme(theme), pal(&paletteFor(theme)) {}

  void render(const Diagnostic &d) { renderImpl(d, 0); }

  void renderSummary() {
    if (numErrors == 0 && numWarnings == 0) {
      return;
    }

    bool first = true;
    if (numErrors > 0) {
      setFg(*out, pal->error, /*bold=*/true);
      *out << numErrors << (numErrors == 1 ? " error" : " errors");
      resetFg(*out);
      first = false;
    }
    if (numWarnings > 0) {
      if (!first) {
        *out << ", ";
      }
      setFg(*out, pal->warning, /*bold=*/true);
      *out << numWarnings << (numWarnings == 1 ? " warning" : " warnings");
      resetFg(*out);
    }
    *out << " generated\n";
  }
};

} // namespace rheo::diag
