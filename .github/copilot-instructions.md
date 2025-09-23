# Copilot Instructions for Cataclysm: Dark Days Ahead

This guide is for AI coding agents working on the Cataclysm-DDA codebase. It summarizes essential project knowledge, workflows, and conventions to maximize productivity and code quality.

## Project Architecture
- **Core Engine:** C++ code in `src/` implements game logic, world simulation, UI, and systems (e.g., inventory, combat, mapgen).
- **Game Data:** Most content (items, monsters, recipes, etc.) is defined in JSON under `data/json/` and `data/mods/`.
- **Build System:** Uses a top-level `Makefile` supporting many platforms and build types (see below).
- **Testing:** C++ tests live in `tests/` and use the Catch2 framework. Test files end with `_test.cpp`.

## Build & Test Workflows
- **Standard build (Linux):**
  - Curses: `make -j$(nproc) RELEASE=1`
  - Tiles: `make -j$(nproc) RELEASE=1 TILES=1`
  - Add `SOUND=1` for sound support (requires `TILES=1`).
  - Use `CLANG=1` to build with Clang, `CCACHE=1` for ccache.
- **Run tests:**
  - `make tests` then run `./tests/cata_test [test_filter]` from the root.
  - Or: `make check` in `tests/` for batch test runs.
- **Style checks:**
  - C++: `make astyle` (uses astyle 3.0.1, see `.astylerc` and `doc/CODE_STYLE.md`).
  - JSON: `make style-json` (whitelisted) or `make style-all-json` (all files).

## Coding & Content Conventions
- **C++:**
  - Follow `doc/CODE_STYLE.md` and run astyle before submitting changes.
  - Use `int` for most integers; avoid `long` and unsigned types unless necessary.
  - Doxygen comments are encouraged for new/changed classes and functions.
- **JSON:**
  - Validate with in-game debug tools or `tools/format/json_formatter.exe`.
  - Place new content in the appropriate subdirectory of `data/json/` or `data/mods/`.
- **PRs:**
  - All PRs must include a `#### Summary` section (see `doc/CONTRIBUTING.md`).
  - Use a new branch per feature/fix; keep `master` clean.

## Integration & External Dependencies
- **SDL2** for tiles/sound builds; **ncurses** for terminal builds.
- **gettext** for localization (optional, skip with `LOCALIZE=0`).
- **Transifex** for translations (see `doc/TRANSLATING.md`).

## Key Files & References
- `README.md`: Project overview, links to build docs.
- `doc/COMPILING/COMPILING.md`: Detailed build instructions for all platforms.
- `doc/CONTRIBUTING.md`: Contribution and PR guidelines.
- `doc/CODE_STYLE.md`: C++ style guide.
- `Makefile`: All build/test/style targets and options.
- `tests/Makefile`: Test build and run logic.

## Examples
- Build a release with tiles and sound: `make -j$(nproc) RELEASE=1 TILES=1 SOUND=1`
- Run all tests: `cd tests && make check`
- Style all C++: `make astyle`

---
For any unclear workflow or convention, check the referenced docs or ask for clarification in your PR.
