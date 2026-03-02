# PR 4 Verification

This note verifies the requested PR 4 behavior in the current branch state.

## What was verified

- Video quality preset buttons are wired and apply live in the Options window via `BuildVideoPresetState(...)` + `ApplyPresetByName(...)`.
- Defaults button performs INI reset (`ResetINIFile`) and reload.
- Options write/read associated `Oblivion.ini` keys with compatibility mirrors:
  - `Resolution` + `iSize W/H`
  - `Windowed` + `bFull Screen`
  - `VSync` + `iPresentInterval`
  - `AA` + `iMultiSample`
  - `Display.Distant*` + `[LOD].bDisplayLOD*`
- `iniOblivion.cpp` default INI string literal formatting is valid for MSVC (no malformed multiline wide string literals).

## Commands used

- `rg -n "BuildVideoPresetState|ApplyPresetByName|IDC_PRESET_VERYLOW|IDC_PRESET_ULTRA|ResetINIFile\(|WriteINI(Int|String)\(L\"Display\", L\"(Resolution|Windowed|VSync|AA|DistantLand|DistantBuildings|DistantTrees)\"|GetINIInt\(L\"Display\", L\"(bFull Screen|Windowed|iPresentInterval|VSync|iMultiSample|AA)\"|GetINIInt\(L\"LOD\", L\"bDisplayLOD" OptionsWindow.cpp`
- `python` check for malformed wide string literal lines in `iniOblivion.cpp`
- `rg -n "g_DefaultOblivionINI|\[Display\]|bDoHighDynamicRange=0" iniOblivion.cpp`
