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
L"[Display]\n"
L"Resolution=2048x1280\n"
L"Windowed=0\n"
L"VSync=1\n"
L"AA=0\n"
L"DistantLand=1\n"
L"DistantBuildings=1\n"
L"DistantTrees=1\n"
L"HDR=1\n"
L"bAllowScreenShot=1\n"
L"iDebugText=1\n"
L"bDoHighDynamicRange=1\n"
L"bDoBloom=0\n"
L"bShadowsOnGrass=0\n"
L"bDoCanopyShadowPass=0\n"
L"iShadowFilter=1\n"
L"bDoSpecularPass=1\n"
L"fSpecualrStartMax=500.0000\n"
L"bDynamicWindowReflections=0\n"
L"uVideoDeviceIdentifierPart1=3539419810\n"
L"uVideoDeviceIdentifierPart2=298795840\n"
L"uVideoDeviceIdentifierPart3=709264353\n"
L"uVideoDeviceIdentifierPart4=903463613\n"
L"iSize W=640\n"
L"iSize H=480\n"
L"fSpecularLOD1=500.0\n"
L"fSpecularLOD2=800.0\n"
L"iActorShadowCountExt=1\n"
L"iActorShadowCountInt=2\n"
L"bActorSelfShadowing=0\n"
L"iMultiSample=0\n"
L"iTexMipMapSkip=1\n"
L"fGrassStartFadeDistance=0.0\n"
L"fGrassEndDistance=0.0\n"
L"bDecalsOnSkinnedGeometry=0\n"
L"fDecalLifetime=10.0\n"
L"bFullBrightLighting=0\n"
L"iMaxLandscapeTextures=0\n"
L"bLODPopActors=0\n"
L"bLODPopItems=0\n"
L"bLODPopObjects=0\n"
L"bUseRefractionShader=1\n"
L"iAdapter=0\n"
L"bFull Screen=0\n"
L"iPresentInterval=0\n"
L"[Advanced]\n"
L"TextureSize=Large\n"
L"TreeFade=75\n"
L"ActorFade=75\n"
L"ItemFade=82\n"
L"ObjectFade=75\n"
L"[Audio]\n"
L"MasterVolume=80\n"
L"EffectsVolume=80\n"
L"MusicVolume=70\n"
L"[Controls]\n"
L"MouseSensitivity=50\n"
L"InvertMouse=0\n"
L"AlwaysRun=1\n"
L"AutoSave=1\n"
L"fJumpAnimDelay=0.2500\n"
L"[MAIN]\n"
L"bEnableBorderRegion=1\n"
L"[Messages]\n"
L"iFileLogging=1\n"
L"[General]\n"
L"iFPSClamp=0\n"
L"bUseEyeEnvMapping=1\n"
L"[Menu]\n"
L"iConsoleHistorySize=100\n"
L"[TerrainManager]\n"
L"uGridDistantCount=30\n"
L"uGridDistantTreeRange=30\n"
L"[Water]\n"
L"bUseWaterReflectionsStatics=1\n"
L"bUseWaterReflectionsTrees=1\n"
L"bUseWaterDepth=1\n"
L"iWaterMult=2\n"
L"bUseWaterReflections=0\n"
L"bUseWaterDisplacements=0\n"
L"bUseWaterHiRes=0\n"
L"[SpeedTree]\n"
L"fLODTreeMipMapLODBias=-0.5000\n"
L"fLocalTreeMipMapLODBias=0.0000\n"
L"[Decals]\n"
L"iMaxDecalsPerFrame=10\n"
L"[Trees]\n"
L"uGridDistantTreeRange=30\n"
L"[LOD]\n"
L"bDisplayLODLand=1\n"
L"bDisplayLODBuildings=1\n"
L"bDisplayLODTrees=0\n"
L"fLODMultTrees=0.5\n"
L"fLODMultActors=5.0\n"
L"fLODMultItems=5.0\n"
L"fLODMultObjects=5.0\n"
L"[BlurShader]\n"
L"bUseBlurShader=1\n"
L"[BlurShaderHDR]\n"
L"bDoHighDynamicRange=0\n";

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
