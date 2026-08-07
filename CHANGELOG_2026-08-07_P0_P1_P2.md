# Changelog - 2026-08-07

## Scope
Stabilization and reliability pass for display/widgets/core.

## P0 - Crash fixes
- `src/displays/widgets/pages.cpp`
  - Removed iterator invalidation in `Page::~Page()`.
  - Added proper owned cleanup for `_widgets` and `_pages`.
  - Added null guards in `removeWidget()` and `removePage()`.
- `src/displays/widgets/widget.h`
  - Initialized all base `Widget` fields in constructor initializer list.
- `src/displays/widgets/textWidget.h`
  - Initialized pointer/state fields to safe defaults (`nullptr`/zero).
- `src/displays/widgets/textWidget.cpp`
  - Added safe destructor cleanup guards.
  - Added re-init cleanup in `init()` to prevent stale pointers and double allocation issues.
- `src/core/display.cpp`
  - Fixed ownership cleanup to avoid double-free by deleting only pager-owned graph in destructor.
- `src/core/audiohandlers.h`
  - Converted header-defined globals to `extern` declarations.
- `src/core/audiohandlers_state.cpp`
  - Added single translation-unit definitions for audiohandler globals.
- `src/main.cpp`
  - AP boot path now clears `player.lockOutput` so metadata/output can resume correctly.
- `src/core/timekeeper.cpp`
  - Replaced VLA weather buffer with heap allocation and proper free paths.

## P1 - Memory/flash durability fixes
- `src/core/netserver.h`
  - Increased `wsBuf` to `4096`.
- `src/core/netserver.cpp`
  - Replaced `sprintf` to bounded `snprintf` for websocket payloads.
  - Replaced unsafe `strcat` assembly with bounded `strlcat` in volume curve payload.
  - Fixed formatting call ordering regressions.
- `src/core/sdmanager.cpp`
  - Replaced VLA (`buff[sectorSize()]`) with bounded heap allocation in `cardPresent()`.
- `src/core/clock_tts.cpp`
  - Replaced blocking `delay(150)` with non-blocking wait state machine before TTS start.
- `src/displays/widgets/scrollWidget.cpp`
  - Fixed re-init leaks for `_spr` and `_sep`.
- `src/displays/widgets/textBoxWidget.cpp`
  - Fixed re-init leaks for `_spr`, `_text`, `_oldtext`.
- `src/displays/widgets/numWidget.cpp`
  - Fixed re-init leaks for `_spr`, `_text`, `_oldtext`.
  - Added allocation safety guards before `memset`.
- `src/displays/widgets/bitrateWidget.cpp`
  - Fixed leak in `refresh()` by deleting sprite object (`delete _spr`) after `deleteSprite()`.
- `src/core/config.h`
  - Added deferred EEPROM commit state (`_eepromDirty`, `_eepromCommitDue`).
  - Updated `saveValue()` templates to debounce commit (2s) for normal writes.
  - Preserved immediate commit behavior for `force=true` writes.
  - Added `loopCommit()` and `forceCommit()` API.
- `src/core/config.cpp`
  - Implemented `Config::loopCommit()` and `Config::forceCommit()`.
- `src/main.cpp`
  - Added `config.loopCommit()` in main loop.

## P2 - Responsiveness/perf
- `src/main.cpp`
  - Replaced blocking backlight fade loop (`delay(12)` in for-loop) with non-blocking fade state machine.
  - Fade update now runs in `loop()` and setup wait loops.
- `src/core/timekeeper.cpp`
  - Moved sync task core pinning to core 1 on multicore targets (core 0 fallback on unicore).
- `src/core/fonts.cpp`
  - Verified font load path checks file existence before open/load.

## Validation
- PlatformIO build: successful.
- PlatformIO upload: successful.

## Runtime impact expected
- Fewer random crashes during page switching.
- Reduced PSRAM leaks on repeated widget/page re-init.
- Lower flash wear during frequent parameter changes.
- Smoother startup and TTS behavior due to non-blocking timing paths.
