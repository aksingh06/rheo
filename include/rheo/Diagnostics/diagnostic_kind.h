#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <llvm/ADT/StringRef.h>

namespace rheo::diag {

enum class Level : std::uint8_t {
  kNote,
  kRemark,
  kWarning,
  kError,
  kFatal,
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define RHEO_DIAGNOSTIC_KINDS(X)                                               \
  /* --- lexer --- */                                                          \
  X(kUnterminatedStringLiteral, kError, "unterminated string literal")         \
  X(kInvalidEscapeSequence, kError, "invalid escape sequence '\\{0}'")         \
                                                                               \
  /* --- parser --- */                                                         \
  X(kExpectedToken, kError, "expected '{0}', found '{1}'")                     \
  X(kUnexpectedIndirectOnEnumCase, kError,                                     \
    "'indirect' applies to the whole enum, not individual cases")              \
                                                                               \
  /* --- genref / borrow checking (RheoLivenessPass) --- */                    \
  X(kUseAfterInvalidation, kError, "use of value after it was invalidated")    \
  X(kBorrowNoteOrigin, kNote, "value borrowed here")                           \
  X(kBorrowNoteInvalidated, kNote,                                             \
    "invalidated here, generation advances from {0} to {1}")                   \
  X(kBorrowNoteFallbackToRuntimeCheck, kRemark,                                \
    "escape analysis could not prove liveness statically; "                    \
    "falling back to runtime generation check")                                \
  X(kUnboundedGenerationChurnInLoop, kWarning,                                 \
    "allocation in a tight loop may exhaust the 32-bit generation "            \
    "counter; consider routing through ArenaAllocator")                        \
                                                                               \
  /* --- type system --- */                                                    \
  X(kClassRequiredForSharedMutableGraph, kError,                               \
    "'{0}' has value semantics (struct) but is captured by multiple "          \
    "owners; use 'class' for shared, identity-bearing state")                  \
  X(kConceptConformanceMissing, kError,                                        \
    "type '{0}' does not conform to concept '{1}'")                            \
  X(kConceptConformanceMissingMethod, kNote, "missing method '{0}'")           \
                                                                               \
  /* --- concurrency --- */                                                    \
  X(kActorIsolationViolation, kError,                                          \
    "actor-isolated property '{0}' can only be accessed from within "          \
    "the actor")                                                               \
  X(kActorIsolationNoteSuggestAwait, kNote,                                    \
    "wrap this access in 'await' to dispatch across the isolation "            \
    "boundary")                                                                \
  X(kActorReentrancyWarning, kWarning,                                         \
    "actor state may have changed across this suspension point")
// NOLINTEND(cppcoreguidelines-macro-usage)

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
enum class Kind : std::uint8_t {
#define X(name, level, fmt) name,
  RHEO_DIAGNOSTIC_KINDS(X)
#undef X
      kNumKinds,
};
// NOLINTEND(cppcoreguidelines-macro-usage)

struct KindInfo {
  llvm::StringRef name{};
  Level defaultLevel{Level::kError};
  llvm::StringRef format{};
};

inline const KindInfo &infoFor(Kind kind) {
  static constexpr std::array<KindInfo,
                              static_cast<std::size_t>(Kind::kNumKinds)>
      kKindInfos = {{
#define X(name, level, fmt)                                                    \
  {llvm::StringRef(#name), Level::level, llvm::StringRef(fmt)},
          RHEO_DIAGNOSTIC_KINDS(X)
#undef X
      }};

  return kKindInfos.at(static_cast<std::size_t>(kind));
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#undef RHEO_DIAGNOSTIC_KINDS
// NOLINTEND(cppcoreguidelines-macro-usage)

} // namespace rheo::diag
