#pragma once

#include <string>
#include "json.hpp"

// JSON mapping loaded from iniOblivion.json (optional). If empty/missing, the launcher will still
// read/write settings without filtering.
extern nlohmann::json gIniMapping;

// Global INI file path accessible to all modules.
extern std::wstring g_INIPath;

// Default INI content for resetting the configuration.
// The extra 'const' after the pointer ensures the declaration matches the definition.
extern const wchar_t* const g_DefaultOblivionINI;

// Retrieves a string value from the INI file.
// @param section The section name in the INI file.
// @param key The key name in the section.
// @param defaultValue Value to return if the key is not found.
// @return The string value from the INI.
std::wstring GetINIString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue);

// Retrieves an integer value from the INI file.
// @param section The section name.
// @param key The key name.
// @param defaultValue Default value if the key is not found.
// @return The integer value.
int GetINIInt(const std::wstring& section, const std::wstring& key, int defaultValue);

// Writes a string value to the INI file, but only if that key exists in the JSON mapping.
// @param section The section name.
// @param key The key name.
// @param value The new value to write.
void WriteINIString(const std::wstring& section, const std::wstring& key, const std::wstring& value);

// Writes an integer value to the INI file by converting it to a string.
// Only writes if the key exists in the JSON mapping.
// @param section The section name.
// @param key The key name.
// @param value The integer value.
void WriteINIInt(const std::wstring& section, const std::wstring& key, int value);

// Resets the INI file to its default settings.
void ResetINIFile();

// Initializes the INI file.
// Sets g_INIPath to "My Documents\My Games\Oblivion\Oblivion.ini", creates the file with default settings if needed,
// and loads the JSON mapping.
void InitializeINI();
