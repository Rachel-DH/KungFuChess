# Kung-Fu-Chess

Real-time chess variant engine (C++17, MSVC/Visual Studio). Pure logic core driven by a
text protocol on stdin/stdout — no graphics. Engine: `Kung-Fu-Chess/` (headers in
`include/`, sources in `src/`). Tests: `Kung-Fu-Chess.Tests/` (doctest).

## Agent team

The main conversation is the **team lead** (reports to Rachel, who is the project
manager). It orchestrates three subagents per feature and **gates every commit**: agents
never commit — after each phase (develop / test / fix / refactor) the team lead reports
the result and asks Rachel to commit, and only then proceeds. It orchestrates:

1. **architect** (Bigtan) — slices the feature into tiny steps, picks the design pattern,
   and defines the focused test list up front. Asks Rachel about real decisions.
2. **implementer** (Teresh) — Senior C++ dev; executes one approved step via the commit
   cycle.
3. **reviewer** (Israel) — reviews the step's diff for bugs, right-sized design, and
   conventions.

## Where the detail lives (load on demand)

- Coding conventions + quality bar → skill **cpp-style**
- Test writing → skill **doctest-tests**
- Small-step workflow + commit cycle + build/test commands → skill **feature-workflow**
- Choosing patterns → skill **design-patterns**

## Build & test

Visual Studio solution `Kung-Fu-Chess.sln` (x64/Debug). See the **feature-workflow** skill
for the exact MSBuild + test-runner commands.
