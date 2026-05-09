# Copilot instructions for Gomoku_Game_Project

Purpose: quick, repository-specific guidance for GitHub Copilot/CLI sessions.

1. Build, test, lint commands

- Preferred IDE: Visual Studio 2022 (open src\src.sln). Build configuration: Release, Platform: x86.
- Build from CLI (MSBuild):
  msbuild src\src.sln /p:Configuration=Release /p:Platform=x86
- Alternative (devenv):
  devenv src\src.sln /Build "Release|x86"
- Run: Launch the built executable produced by the project (open the Release or Debug output folder under src\). Use Ctrl+F5 in Visual Studio.
- Tests: no automated unit-test framework or CI detected. To exercise behavior, run the application and use in-game flows (PvE/PvP). There is no single-test runner in repo.
- Linting: no repo-level linter configured. Use Visual Studio's built-in static analysis and Code Analysis settings.

2. High-level architecture (big picture)

- Language & platform: C++17 on Windows using Win32 API + GDI+ for rendering.
- Project layout (top-level folders):
  - ApplicationTypes: shared data structures (GameConfig, GameState, constants).
  - GameLogic: core gameplay & AI (BotAI, rules, engine).
  - RenderAPI: rendering primitives, UI components, UIScaler.
  - ScreenModules: screen-specific UI modules (Menu, Play, Settings, etc.).
  - SystemModules: subsystems (Audio, Save/Load, Localization, Config, Time, Stats).
  - Asset/: fonts, audio, language files, sprites, save file slots.
  - main.cpp: Win32 message loop and application entry point.
- AI: three difficulty levels. Hard uses Minimax + Alpha-Beta with Zobrist hashing (transposition table ~1M entries) and move ordering. Default search depth ~5, branching limits applied to keep response time low.
- Save format: binary serialization with versioning (current version referenced in README) and a magic number for file validation.

3. Key conventions and repository-specific patterns

- Architectural style: modular, procedural (not heavy OOP). Prefer module-scoped functions and explicit data structs.
- Naming: several global caches use g\_ prefix (e.g., g_ModelCache, g_BrushCache). Expect global variables in renderer/audio subsystems.
- UI scaling: UIScaler is used; optimal target resolution is 1280x720 or higher.
- Asset locations: localization files under Asset/lang (vi.txt, en.txt). Save slots stored in Asset/save (slot_X.bin).
- Binary saves: include a magic number (0xCA05A1E2) and versioning — be cautious when editing serialization code.
- Build target: project is 32-bit (x86) by design — do not switch platform to x64 without verifying asset/serialization compatibility.

4. AI / Assistant integraton notes present in repo

- GitNexus: this repo includes GitNexus guidance in .claude/ and AGENTS.md/CLAUDE.md. Follow those rules when making code changes:
  - Run impact analysis before editing any symbol (gitnexus_impact).
  - Run gitnexus_detect_changes() before committing.
  - Avoid blind rename/find-replace — use gitnexus_rename if available.

5. Quick troubleshooting pointers for Copilot sessions

- When asked to modify a function/class: run impact analysis first and report blast radius.
- If asked to add tests or CI: note there is no test framework or workflow; propose adding a test harness (googletest or Catch2) if desired.

Files to consult quickly

- README.md (architecture and build notes)
- src/main.cpp (entry point, WndProc loop)
- ApplicationTypes, GameLogic, RenderAPI, SystemModules folders for feature-specific code

If you'd like: configure MCP servers (e.g., code-intel) for deeper analysis. Otherwise, this file is created — want any adjustments or extra coverage (serialization, audio, AI internals)?
