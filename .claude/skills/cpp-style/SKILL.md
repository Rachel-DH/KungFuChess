---
name: cpp-style
description: The C++17 coding conventions and senior quality bar for the Kung-Fu-Chess engine. Load BEFORE writing or reviewing any C++ code — covers naming (member vars end with a trailing underscore, NOT m_), formatting, comment style, and the correctness/ownership bar. Keeps code concise, clear, and debuggable.
---

# Kung-Fu-Chess C++ style & quality

Write code that reads like the code already in `Kung-Fu-Chess/src` and `include`.
Concise, clear, debuggable. No cleverness for its own sake.

## Part A — Senior quality bar (correctness first)

- **RAII & ownership**: resources managed by their owning object; ownership is obvious.
- **const-correctness**: `const` on methods that don't mutate, on locals that don't
  change, on reference params you only read (`const Board&`). See Part E for the exact
  rules and the pre-finish checklist.
- **Exception safety**: throw for exceptional paths only (e.g. off-board → `std::out_of_range`;
  parse errors → `ParseError`). Leave objects in a valid state on throw.
- **Zero UB**: bounds-checked access, no dangling refs/iterators, no signed overflow reliance.
- **Values over sentinels**: `std::optional<Cell>` for "maybe a piece", `enum class` for
  fixed sets — never magic ints or bool flags where an enum reads clearer.
- **No needless copies**: pass big types by `const&`; `std::move` when transferring.
- **Small pure functions**: movement rules etc. are pure functions of (args, board).
- **Single Responsibility**: one thing per function, one reason to change per class. See
  Part F for the exact rules and the pre-finish checklist.
- **Function length**: as short as the logic honestly allows, but never at the cost of
  clarity — don't fragment a function into unlabeled pieces just to hit a line count, and
  don't cram unrelated steps into one function just to avoid a second one. One function,
  one job, one level of abstraction: if you're reaching for a comment to mark off a
  section (`// now validate`, `// now apply`), that's usually a sign it should be its own
  well-named function instead. Extract only when the extracted piece is independently
  understandable (has a name that fully explains it) — extracting a fragment that only
  makes sense read inline just moves confusion around.
- **Zero compiler warnings.**

## Part B — Naming & formatting (match existing code exactly)

- **Member variables**: **trailing underscore** — `board_`, `clock_ms_`, `selected_`,
  `pending_moves_`. NOT `m_`. (Plain data structs like `Board`'s `width`/`height`/`cells`
  stay bare — follow the neighbor.)
- **Functions / methods / locals / params**: `snake_case` — `is_available_move`,
  `settle_arrived_moves`, `start_x`, `dest_cell`. No `tmp`, `x2`, `foo` — names say intent.
- **Types / classes / structs**: `PascalCase` — `GameEngine`, `PieceFactory`, `PendingMove`.
- **Constants**: `UPPER_SNAKE_CASE` on `static constexpr` — `DEFAULT_MOVE_MS_PER_CELL`, `JUMP_DURATION_MS`.
- **Enums**: `enum class`, terse enumerators matching text tokens — `Color { w, b }`,
  `PieceType { K, Q, R, B, N, P }`.
- **Getters**: Board uses `get_`-prefix (`get_at`, `get_width`); GameEngine uses bare
  nouns (`clock_ms()`, `game_over()`). Follow the class you're in.
- **Files**: `PascalCase` = primary class. Headers in `include/`, sources in `src/`.
- **Formatting**: 4 spaces, no tabs. K&R braces (opening brace on the same line). Braces
  even around single-statement `if` bodies. Braced-init with inner spaces: `Cell{ Color::w, PieceType::P }`.
- **`#include` order**: own header → blank → `<std>` headers → blank → project headers.
  File-local helpers go in an anonymous `namespace { ... } // namespace`.

## Part C — Parameter Passing

- **By value** — for small, cheap-to-copy types: `int`, `bool`, `double`, `enum`s, pointers, and small structs (e.g. a `{int, int}` position). Copying these is cheaper than the indirection of a reference.
- **By `const&`** — for large or expensive-to-copy types you only need to read: `std::string`, `std::vector`, `std::map`, or any non-trivial struct/class. Avoids an unnecessary copy on every call.
- **By `&` (non-const)** — only when the function needs to mutate the caller's variable in place. Rare in modern C++; prefer returning a new value (RVO/NRVO makes this free) over an output parameter.
- **By value + `std::move`** — when the function will store or take ownership of the value regardless. Take the parameter by value, then `std::move` it into the member/field. This lets the caller choose: pass an rvalue/`std::move`d value for a zero-copy transfer, or pass an lvalue and pay for exactly one copy — never more.

> **Rule of thumb:** if `sizeof(type)` is roughly ≤ 2 pointers (16 bytes on 64-bit) and it owns no heap resource → pass by value. Otherwise → `const&`, unless the callee is going to store it, in which case → value + `std::move`.

## Part D — Comments

`//` line comments only (no `/* */`, no Doxygen). Comment the **why / contract**, not a
line-by-line narration. Every public method gets a short contract comment (behavior +
edge cases). Add an inline note only to justify a non-obvious line
(`// white moves up (-y), black moves down (+y)`).

## Part E — No magic numbers & const-correctness (explicit rules)

- **No magic numbers/strings**: every constant value (numeric or string literal) used in
  class logic must be extracted into a named constant declared at the top of the class
  (`static constexpr` / `static const`), named `UPPER_SNAKE_CASE` and descriptive
  (`JUMP_DURATION_MS`, not `DURATION`). Exception: a trivial `0`/`1` used purely as
  loop/index arithmetic (`i + 1`, `size() - 1`) — not a business value — doesn't need
  extracting.
- **Const-correctness on methods**: any method that doesn't modify the object's state
  must be marked `const`.
- **Const parameters — objects only**: any object parameter (custom class, STL
  container, string — passed by reference or pointer) that isn't modified in the
  function body must be `const&` / `const*`. Primitives passed by value (`int`, `float`,
  `bool`, `char`, etc.) are exempt — don't force `const` onto those.
- **Const locals**: any local variable whose value doesn't change after initialization
  must be declared `const`.
- **Workflow**: before finishing any function, check every object parameter and local
  variable for a missing `const`. Before finishing any method, check whether it can be
  `const`. If existing code you touch violates these rules, flag it and propose a
  refactor rather than leaving it as-is.

## Part F — Single Responsibility Principle (explicit rules)

- **Functions — one thing, one level of abstraction**: if a function's name needs "and"
  to describe it (`validate_and_save`, `parse_and_render`), split it. A function mixing
  I/O, business logic, and formatting in one body must be broken into smaller functions,
  each isolating one concern.
- **Classes — one reason to change**: before finishing a class, state its single
  responsibility in one sentence with no "and"/"or". If the sentence needs one, split the
  class. Watch for the common violations: a class that both fetches/persists data AND
  contains business logic; a class that both computes a result AND formats/logs/prints
  it; a "manager"/"utils"/"helper" class accumulating unrelated methods over time.
- **Size is a signal, not a rule**: a long function or class is often — not always — a
  sign of mixed responsibilities. Use length as a prompt to re-check cohesion, not as a
  hard limit to enforce on its own.
- **Workflow**: before finishing any new function or class, and before extending an
  existing one with a new capability, explicitly check whether the addition fits the
  existing single responsibility or is a second one. If it's a second responsibility,
  propose extracting a new function/class instead of appending to the existing one —
  don't silently bolt it on. When reviewing or refactoring existing code that violates
  SRP, flag it and propose a split rather than leaving it as-is.

