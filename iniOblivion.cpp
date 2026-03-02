// iniOblivion.cpp
// INI routines for Modern Oblivion Launcher.
// Target: %USERPROFILE%\Documents\My Games\Oblivion\Oblivion.ini
// Notes:
//  - The launcher must be robust when iniOblivion.json is missing.
//  - Writes are atomic/safe where possible and directories are created.

#include "iniOblivion.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include <fstream>
#include <vector>

// Include nlohmann/json header (download json.hpp from https://github.com/nlohmann/json)
#include "json.hpp"
using json = nlohmann::json;

#pragma comment(lib, "shlwapi.lib")

// Global INI file path.
std::wstring g_INIPath;

// Global JSON mapping (read from iniOblivion.json). May be empty.
json gIniMapping;

static bool EnsureDirTree(const std::wstring& dir)
{
    if (dir.empty()) return false;
    DWORD attr = GetFileAttributesW(dir.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES)
        return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;

    // Recurse to parent
    std::wstring parent = dir;
    std::vector<wchar_t> buf(parent.begin(), parent.end());
    buf.push_back(L'\0');
    if (!PathRemoveFileSpecW(buf.data()))
        return false;
    parent.assign(buf.data());
    if (!parent.empty() && parent != dir)
        EnsureDirTree(parent);

    if (CreateDirectoryW(dir.c_str(), nullptr))
        return true;
    DWORD e = GetLastError();
    return (e == ERROR_ALREADY_EXISTS);
}

static std::wstring GetExeDir()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    return std::wstring(path);
}

// ----------------------------------------------------------------------
// WideToUtf8
// Converts a std::wstring (UTF-16) to a UTF-8 encoded std::string using the Windows API.
std::string WideToUtf8(const std::wstring& wstr)
{
    if (wstr.empty())
        return std::string();

    // Determine required buffer size for the UTF-8 string.
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0)
        return std::string();

    std::string result(sizeNeeded, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], sizeNeeded, nullptr, nullptr);
    return result;
}

// ----------------------------------------------------------------------
// LoadINIJsonMapping
// Reads the JSON mapping from "iniOblivion.json". This JSON file defines, for each section,
// an array of key names that are allowed in that section.
void LoadINIJsonMapping()
{
    // Prefer iniOblivion.json sitting next to the launcher executable.
    std::wstring p = GetExeDir();
    if (!p.empty() && p.back() != L'\\') p.push_back(L'\\');
    p += L"iniOblivion.json";

    std::ifstream ifs(WideToUtf8(p));
    if (ifs)
    {
        try {
            ifs >> gIniMapping;
        }
        catch (...) {
            // If parsing fails, use an empty JSON object.
            gIniMapping = json::object();
        }
    }
    else
    {
        // If the file is missing, assign an empty object.
        gIniMapping = json::object();
    }
}

// ----------------------------------------------------------------------
// Default INI content for resetting the configuration.
const wchar_t* const g_DefaultOblivionINI =
L"[Display]
"
L"Resolution=2048x1280
"
L"Windowed=0
"
L"VSync=1
"
L"AA=0
"
L"DistantLand=1
"
L"DistantBuildings=1
"
L"DistantTrees=1
"
L"HDR=1
"
L"bAllowScreenShot=1
"
L"iDebugText=1
"
L"bDoHighDynamicRange=1
"
L"bDoBloom=0
"
L"bShadowsOnGrass=0
"
L"bDoCanopyShadowPass=0
"
L"iShadowFilter=1
"
L"bDoSpecularPass=1
"
L"fSpecualrStartMax=500.0000
"
L"bDynamicWindowReflections=0
"
L"uVideoDeviceIdentifierPart1=3539419810
"
L"uVideoDeviceIdentifierPart2=298795840
"
L"uVideoDeviceIdentifierPart3=709264353
"
L"uVideoDeviceIdentifierPart4=903463613
"
L"iSize W=640
"
L"iSize H=480
"
L"fSpecularLOD1=500.0
"
L"fSpecularLOD2=800.0
"
L"iActorShadowCountExt=1
"
L"iActorShadowCountInt=2
"
L"bActorSelfShadowing=0
"
L"iMultiSample=0
"
L"iTexMipMapSkip=1
"
L"fGrassStartFadeDistance=0.0
"
L"fGrassEndDistance=0.0
"
L"bDecalsOnSkinnedGeometry=0
"
L"fDecalLifetime=10.0
"
L"bFullBrightLighting=0
"
L"iMaxLandscapeTextures=0
"
L"bLODPopActors=0
"
L"bLODPopItems=0
"
L"bLODPopObjects=0
"
L"bUseRefractionShader=1
"
L"iAdapter=0
"
L"bFull Screen=0
"
L"iPresentInterval=0
"
L"[Advanced]
"
L"TextureSize=Large
"
L"TreeFade=75
"
L"ActorFade=75
"
L"ItemFade=82
"
L"ObjectFade=75
"
L"[Audio]
"
L"MasterVolume=80
"
L"EffectsVolume=80
"
L"MusicVolume=70
"
L"[Controls]
"
L"MouseSensitivity=50
"
L"InvertMouse=0
"
L"AlwaysRun=1
"
L"AutoSave=1
"
L"fJumpAnimDelay=0.2500
"
L"[MAIN]
"
L"bEnableBorderRegion=1
"
L"[Messages]
"
L"iFileLogging=1
"
L"[General]
"
L"iFPSClamp=0
"
L"bUseEyeEnvMapping=1
"
L"[Menu]
"
L"iConsoleHistorySize=100
"
L"[TerrainManager]
"
L"uGridDistantCount=30
"
L"uGridDistantTreeRange=30
"
L"[Water]
"
L"bUseWaterReflectionsStatics=1
"
L"bUseWaterReflectionsTrees=1
"
L"bUseWaterDepth=1
"
L"iWaterMult=2
"
L"bUseWaterReflections=0
"
L"bUseWaterDisplacements=0
"
L"bUseWaterHiRes=0
"
L"[SpeedTree]
"
L"fLODTreeMipMapLODBias=-0.5000
"
L"fLocalTreeMipMapLODBias=0.0000
"
L"[Decals]
"
L"iMaxDecalsPerFrame=10
"
L"[Trees]
"
L"uGridDistantTreeRange=30
"
L"[LOD]
"
L"bDisplayLODLand=1
"
L"bDisplayLODBuildings=1
"
L"bDisplayLODTrees=0
"
L"fLODMultTrees=0.5
"
L"fLODMultActors=5.0
"
L"fLODMultItems=5.0
"
L"fLODMultObjects=5.0
"
L"[BlurShader]
"
L"bUseBlurShader=1
"
L"[BlurShaderHDR]
"
L"bDoHighDynamicRange=0
";

// ----------------------------------------------------------------------
// GetINIString
// Reads a string value from the INI file for a given section and key.
std::wstring GetINIString(const std::wstring& section, const std::wstring& key, const std::wstring& defaultValue)
{
    wchar_t buffer[512] = { 0 };
    GetPrivateProfileStringW(section.c_str(), key.c_str(), defaultValue.c_str(),
        buffer, sizeof(buffer) / sizeof(wchar_t),
        g_INIPath.c_str());
    return std::wstring(buffer);
}

// ----------------------------------------------------------------------
// GetINIInt
// Reads an integer value from the INI file.
int GetINIInt(const std::wstring& section, const std::wstring& key, int defaultValue)
{
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), defaultValue, g_INIPath.c_str());
}

// ----------------------------------------------------------------------
// ResetINIFile
// Resets the INI file to the default configuration.
void ResetINIFile()
{
    // Ensure parent directories exist.
    std::wstring dir = g_INIPath;
    {
        std::vector<wchar_t> buf(dir.begin(), dir.end());
        buf.push_back(L'\0');
        PathRemoveFileSpecW(buf.data());
        dir.assign(buf.data());
    }
    EnsureDirTree(dir);

    // Oblivion.ini is traditionally ANSI/ASCII. Write UTF-8 without BOM for broad compatibility.
    std::string bytes = WideToUtf8(g_DefaultOblivionINI);
    std::ofstream f(WideToUtf8(g_INIPath), std::ios::binary | std::ios::trunc);
    if (!f.is_open()) return;
    f.write(bytes.data(), (std::streamsize)bytes.size());
    f.flush();
}

// ----------------------------------------------------------------------
// InitializeINI
// Determines the INI file path and creates it with default settings if it doesn't exist.
// Also loads the JSON mapping from the external file.
void InitializeINI()
{
    wchar_t myDocuments[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_MYDOCUMENTS, nullptr, SHGFP_TYPE_CURRENT, myDocuments)))
    {
        std::wstring path(myDocuments);
        path += L"\\My Games\\Oblivion";
        EnsureDirTree(path);
        path += L"\\Oblivion.ini";
        g_INIPath = path;
    }
    else
    {
        g_INIPath = L"Oblivion.ini";
    }

    DWORD attrs = GetFileAttributesW(g_INIPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        ResetINIFile();
    }

    // Load the external JSON mapping.
    LoadINIJsonMapping();
}

// ----------------------------------------------------------------------
// WriteINIString
// Writes a string value to the INI file.
// If iniOblivion.json is present, it can be used to *optionally* filter writes.
// However, missing/empty mapping must NEVER break saving.
void WriteINIString(const std::wstring& section, const std::wstring& key, const std::wstring& value)
{
    // No mapping -> allow all writes.
    if (!gIniMapping.is_object() || gIniMapping.empty())
    {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), g_INIPath.c_str());
        WritePrivateProfileStringW(nullptr, nullptr, nullptr, g_INIPath.c_str());
        return;
    }

    std::string utf8Section = WideToUtf8(section);
    std::string utf8Key = WideToUtf8(key);

    // Missing section -> allow (do not brick saving due to outdated mapping).
    if (!gIniMapping.contains(utf8Section))
    {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), g_INIPath.c_str());
        WritePrivateProfileStringW(nullptr, nullptr, nullptr, g_INIPath.c_str());
        return;
    }

    // Section exists but is not an array -> allow.
    if (!gIniMapping[utf8Section].is_array())
    {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), g_INIPath.c_str());
        WritePrivateProfileStringW(nullptr, nullptr, nullptr, g_INIPath.c_str());
        return;
    }

    // Empty key list -> allow.
    if (gIniMapping[utf8Section].empty())
    {
        WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), g_INIPath.c_str());
        WritePrivateProfileStringW(nullptr, nullptr, nullptr, g_INIPath.c_str());
        return;
    }

    bool allowed = false;
    for (const auto& jKey : gIniMapping[utf8Section])
    {
        if (!jKey.is_string()) continue;
        const std::string s = jKey.get<std::string>();
        if (s == "*") { allowed = true; break; }
        if (s == utf8Key) { allowed = true; break; }
    }

    if (!allowed)
        return;

    WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), g_INIPath.c_str());
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, g_INIPath.c_str());
}

void WriteINIInt(const std::wstring& section, const std::wstring& key, int value)
{
    wchar_t buffer[32] = { 0 };
    swprintf(buffer, 32, L"%d", value);
    WriteINIString(section, key, buffer);
}
