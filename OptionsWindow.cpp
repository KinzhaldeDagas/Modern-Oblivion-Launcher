// OptionsWindow.cpp (production)

// Win32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlwapi.h>

// STL
#include <string>
#include <vector>
#include <map>
#include <cwctype>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <climits>

#include "OptionsWindow.h"
#include "iniOblivion.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

// ----------------------------------------------------------------------------
// Forward decls
// ----------------------------------------------------------------------------
static int  GetEditInt(HWND h, int fallback);
static void SetEditInt(HWND h, int v);


static void SetCtrlFont(HWND h);

static HWND CreateComboBox(HWND parent, int id, int x, int y, int w, int h);
static void AddComboItemWithData(HWND hCombo, const wchar_t* text, LPARAM data);
static void AddComboItem(HWND hCombo, const wchar_t* text, LPARAM data);

static HWND CreateTrackBar(HWND parent, int id, int x, int y, int w, int h, int minV, int maxV);
static HWND CreateValueLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h);
static HWND CreateEditBox(HWND parent, int id, int x, int y, int w, int h);

static std::wstring PresetPathFromName(const std::wstring& presetName);
static std::map<std::wstring, std::wstring> CaptureCurrentState();

// ----------------------------------------------------------------------------
// Globals
// ----------------------------------------------------------------------------
static HFONT gFont = nullptr;
static HWND  gWnd = nullptr;

static HWND  gScrollHost = nullptr;
static HWND  gContent = nullptr;
static int   gScrollPos = 0;
static int   gContentHeight = 0;

static HWND  gTitleText = nullptr;
static HWND  gPresetLabel = nullptr;

static HWND gPresetCombo = nullptr;
static std::vector<std::wstring> gPresetPaths;
static std::wstring gPendingPresetPath;

static HWND gBtnApply = nullptr;
static HWND gBtnCancel = nullptr;
static HWND gBtnDefaults = nullptr;
static HWND gBtnSavePreset = nullptr;
static HWND gBtnPresetVeryLow = nullptr;
static HWND gBtnPresetLow = nullptr;
static HWND gBtnPresetMedium = nullptr;
static HWND gBtnPresetHigh = nullptr;
static HWND gBtnPresetUltra = nullptr;

static HWND gAdapterCombo = nullptr;
static HWND gWindowedRadio = nullptr;
static HWND gFullscreenRadio = nullptr;
static HWND gEffectsNoneRadio = nullptr;
static HWND gAutoSaveCheck = nullptr;
static HWND gUseEyeEnvMappingCheck = nullptr;
static HWND gUseBlurShaderCheck = nullptr;
static HWND gWaterHiResCheck = nullptr;
static HWND gActorSelfShadowingCheck = nullptr;
static HWND gCloseLauncherOnLaunchCheck = nullptr;

static HWND gResCombo = nullptr;
static HWND gWinModeCombo = nullptr;
static HWND gVsyncCheck = nullptr;

static HWND gHdrCheck = nullptr;
static HWND gAaCombo = nullptr;
static HWND gBloomCheck = nullptr;
static HWND gShadowsOnGrassCheck = nullptr;
static HWND gTreeCanopyShadowsCheck = nullptr;
static HWND gShadowFilterCombo = nullptr;
static HWND gSpecularTrack = nullptr;
static HWND gSpecularVal = nullptr;

static HWND gWaterDetailCombo = nullptr;
static HWND gWaterReflectionsCheck = nullptr;
static HWND gWaterRipplesCheck = nullptr;
static HWND gWindowReflectionsCheck = nullptr;
static HWND gWaterDepthCheck = nullptr;
static HWND gBloodDecalsCombo = nullptr;

static HWND gGridDistantCountEdit = nullptr;
static HWND gGridDistantTreeRangeEdit = nullptr;
static HWND gWaterReflStaticsCheck = nullptr;
static HWND gWaterReflTreesCheck = nullptr;

static HWND gDistantLandCheck = nullptr;
static HWND gDistantBuildingsCheck = nullptr;
static HWND gDistantTreesCheck = nullptr;

static HWND gTexSizeCombo = nullptr;
static HWND gTreeFadeTrack = nullptr;
static HWND gActorFadeTrack = nullptr;
static HWND gItemFadeTrack = nullptr;
static HWND gObjectFadeTrack = nullptr;
static HWND gTreeFadeVal = nullptr;
static HWND gActorFadeVal = nullptr;
static HWND gItemFadeVal = nullptr;
static HWND gObjectFadeVal = nullptr;
static HWND gJumpDelayTrack = nullptr;
static HWND gJumpDelayVal = nullptr;
static HWND gLodTreeBiasTrack = nullptr;
static HWND gLodTreeBiasVal = nullptr;
static HWND gLodLocalTreeBiasTrack = nullptr;
static HWND gLodLocalTreeBiasVal = nullptr;

static HWND gMasterVolTrack = nullptr;
static HWND gEffectsVolTrack = nullptr;
static HWND gMusicVolTrack = nullptr;
static HWND gMasterVolVal = nullptr;
static HWND gEffectsVolVal = nullptr;
static HWND gMusicVolVal = nullptr;

static HWND gMouseSensTrack = nullptr;
static HWND gMouseSensVal = nullptr;
static HWND gInvertMouseCheck = nullptr;
static HWND gAlwaysRunCheck = nullptr;

static HWND gDisableBorderRegionsCheck = nullptr;
static HWND gEnableConstructionSetButtonCheck = nullptr;

static HWND gDevAllowScreenshotCheck = nullptr;
static HWND gDevFileLoggingCheck = nullptr;
static HWND gDevDebugTextCheck = nullptr;
static HWND gDevFpsClampEdit = nullptr;
static HWND gDevConsoleHistoryEdit = nullptr;

// ----------------------------------------------------------------------------
// IDs
// ----------------------------------------------------------------------------
static const int IDC_PRESET_COMBO = 1100;
static const int IDC_APPLY = 1101;
static const int IDC_CANCEL = 1102;
static const int IDC_DEFAULTS = 1103;
static const int IDC_SAVE_PRESET = 1104;
static const int IDC_PRESET_VERYLOW = 1105;
static const int IDC_PRESET_LOW = 1106;
static const int IDC_PRESET_MEDIUM = 1107;
static const int IDC_PRESET_HIGH = 1108;
static const int IDC_PRESET_ULTRA = 1109;

static const int IDC_RES_COMBO = 1200;
static const int IDC_WINMODE_COMBO = 1201;
static const int IDC_VSYNC = 1202;
static const int IDC_MODE_WINDOWED = 1203;
static const int IDC_MODE_FULLSCREEN = 1204;
static const int IDC_EFFECTS_NONE = 1205;
static const int IDC_ADAPTER_COMBO = 1206;
static const int IDC_AUTOSAVE = 1207;
static const int IDC_EYE_ENVMAP = 1208;
static const int IDC_BLUR_SHADER = 1209;
static const int IDC_WATER_HIRES = 1210;
static const int IDC_ACTOR_SELF_SHADOW = 1211;

static const int IDC_HDR = 1300;
static const int IDC_AA_COMBO = 1301;
static const int IDC_DIST_LAND = 1302;
static const int IDC_DIST_BUILD = 1303;
static const int IDC_DIST_TREES = 1304;

static const int IDC_BLOOM = 1305;
static const int IDC_SHADOWS_GRASS = 1306;
static const int IDC_CANOPY_SHADOWS = 1307;
static const int IDC_SHADOW_FILTER = 1308;
static const int IDC_SPECULAR = 1309;
static const int IDC_WATER_DETAIL = 1310;
static const int IDC_WATER_REFLECT = 1311;
static const int IDC_WATER_RIPPLES = 1312;
static const int IDC_WINDOW_REFLECT = 1313;
static const int IDC_BLOOD_DECALS = 1314;
static const int IDC_WATER_DEPTH = 1315;
static const int IDC_WATER_REFLECT_STATICS = 1316;
static const int IDC_WATER_REFLECT_TREES = 1317;

static const int IDC_TEXSIZE_COMBO = 1400;
static const int IDC_TREEFADE = 1401;
static const int IDC_ACTORFADE = 1402;
static const int IDC_ITEMFADE = 1403;
static const int IDC_OBJECTFADE = 1404;
static const int IDC_UGRID_DISTANT_COUNT = 1405;
static const int IDC_UGRID_DISTANT_TREE_RANGE = 1406;
static const int IDC_JUMP_DELAY = 1407;
static const int IDC_LOD_TREE_BIAS = 1408;
static const int IDC_LOCAL_TREE_BIAS = 1409;

static const int IDC_MASTERVOL = 1501;
static const int IDC_EFFECTSVOL = 1502;
static const int IDC_MUSICVOL = 1503;

static const int IDC_MOUSESENS = 1601;
static const int IDC_INVERTMOUSE = 1602;
static const int IDC_ALWAYSRUN = 1603;

static const int IDC_DISABLE_BORDER = 1700;
static const int IDC_ENABLE_CS_BUTTON = 1701;
static const int IDC_CLOSE_ON_LAUNCH = 1702;

static const int IDC_DEV_ALLOWSCREENSHOT = 1900;
static const int IDC_DEV_FILELOGGING = 1901;
static const int IDC_DEV_DEBUGTEXT = 1902;
static const int IDC_DEV_FPSCLAMP = 1903;
static const int IDC_DEV_CONSOLEHIST = 1904;

// ----------------------------------------------------------------------------
// Utility
// ----------------------------------------------------------------------------
static std::wstring Trim(const std::wstring& s)
{
    size_t a = 0;
    while (a < s.size() && iswspace(s[a])) a++;
    size_t b = s.size();
    while (b > a && iswspace(s[b - 1])) b--;
    return s.substr(a, b - a);
}

static std::wstring FloatToWString(double v, int decimals = 4)
{
    wchar_t buf[64] = {};
    if (decimals < 0) decimals = 0;
    if (decimals > 8) decimals = 8;
    swprintf_s(buf, L"%.*f", decimals, v);
    return std::wstring(buf);
}

static std::string WideToUtf8(const std::wstring& ws)
{
    if (ws.empty()) return std::string();
    int needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return std::string();
    std::string out;
    out.resize((size_t)needed);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), out.data(), needed, nullptr, nullptr);
    return out;
}

static bool WriteFileUtf8Atomic(const std::wstring& path, const std::string& utf8)
{
    std::wstring dir = path;
    std::vector<wchar_t> buf(dir.begin(), dir.end());
    buf.push_back(L'\0');
    PathRemoveFileSpecW(buf.data());
    std::wstring parent = buf.data();
    if (!parent.empty())
        SHCreateDirectoryExW(nullptr, parent.c_str(), nullptr);

    std::wstring tmp = path + L".tmp";

    HANDLE h = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    BOOL ok = TRUE;
    if (!utf8.empty())
        ok = WriteFile(h, utf8.data(), (DWORD)utf8.size(), &written, nullptr);

    FlushFileBuffers(h);
    CloseHandle(h);

    if (!ok)
    {
        DeleteFileW(tmp.c_str());
        return false;
    }

    if (!MoveFileExW(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        DeleteFileW(tmp.c_str());
        return false;
    }
    return true;
}

static std::wstring GetExeDir()
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    return std::wstring(path);
}

static std::wstring PresetsDir()
{
    wchar_t docs[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, docs)))
    {
        std::wstring dir = docs;
        dir += L"\\My Games\\Oblivion\\LauncherPresets";
        SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
        return dir;
    }

    std::wstring fallback = GetExeDir();
    fallback += L"\\presets";
    CreateDirectoryW(fallback.c_str(), nullptr);
    return fallback;
}

static std::wstring PresetPathFromName(const std::wstring& presetName)
{
    std::wstring fn = Trim(presetName);
    if (fn.empty()) fn = L"Preset";
    if (fn.size() > 64) fn.resize(64);

    for (auto& ch : fn)
    {
        if (ch < 32) { ch = L'_'; continue; }
        if (ch == L'/' || ch == L'\\' || ch == L':' || ch == L'*' || ch == L'?' || ch == L'"' || ch == L'<' || ch == L'>' || ch == L'|')
            ch = L'_';
    }

    while (!fn.empty() && (fn.back() == L'.' || fn.back() == L' ')) fn.pop_back();
    if (fn.empty()) fn = L"Preset";

    std::wstring p = PresetsDir();
    p += L"\\";
    p += fn;
    p += L".txt";
    return p;
}

static std::wstring MakeUniquePresetPath(const std::wstring& baseName, std::wstring* outDisplayName)
{
    std::wstring name = Trim(baseName);
    if (name.empty()) name = L"Custom";

    std::wstring candidate = PresetPathFromName(name);
    DWORD attr = GetFileAttributesW(candidate.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        if (outDisplayName) *outDisplayName = name;
        return candidate;
    }

    for (int i = 2; i < 1000; i++)
    {
        std::wstring n = name + L" (" + std::to_wstring(i) + L")";
        candidate = PresetPathFromName(n);
        attr = GetFileAttributesW(candidate.c_str());
        if (attr == INVALID_FILE_ATTRIBUTES)
        {
            if (outDisplayName) *outDisplayName = n;
            return candidate;
        }
    }

    if (outDisplayName) *outDisplayName = name;
    return PresetPathFromName(name);
}

static void SetCtrlFont(HWND h)
{
    if (h && gFont) SendMessageW(h, WM_SETFONT, (WPARAM)gFont, TRUE);
}

static void SetTextInt(HWND h, int v)
{
    wchar_t b[32] = {};
    wsprintfW(b, L"%d", v);
    SetWindowTextW(h, b);
}

static void SetTextFloat(HWND h, double v, int decimals = 3)
{
    if (!h) return;
    std::wstring text = FloatToWString(v, decimals);
    SetWindowTextW(h, text.c_str());
}

static int GetTrackPos(HWND h, int fallback)
{
    if (!h) return fallback;
    return (int)SendMessageW(h, TBM_GETPOS, 0, 0);
}

static void SetTrackPos(HWND h, int v)
{
    if (!h) return;
    SendMessageW(h, TBM_SETPOS, TRUE, (LPARAM)v);
}

// ----------------------------------------------------------------------------
// Control factories
// ----------------------------------------------------------------------------
static HWND CreateComboBox(HWND parent, int id, int x, int y, int w, int h)
{
    HWND c = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(c);
    return c;
}

static void AddComboItem(HWND hCombo, const wchar_t* text, LPARAM data)
{
    int idx = (int)SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)text);
    if (idx >= 0) SendMessageW(hCombo, CB_SETITEMDATA, idx, data);
}

static void AddComboItemWithData(HWND hCombo, const wchar_t* text, LPARAM data)
{
    AddComboItem(hCombo, text, data);
}

static HWND CreateTrackBar(HWND parent, int id, int x, int y, int w, int h, int minV, int maxV)
{
    HWND t = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(t);
    SendMessageW(t, TBM_SETRANGE, TRUE, MAKELONG(minV, maxV));
    SendMessageW(t, TBM_SETTICFREQ, 10, 0);
    return t;
}

static HWND CreateValueLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h)
{
    HWND s = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
        x, y, w, h, parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(s);
    return s;
}

static HWND CreateEditBox(HWND parent, int id, int x, int y, int w, int h)
{
    HWND e = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(e);
    return e;
}

static int GetEditInt(HWND h, int fallback)
{
    if (!h) return fallback;
    wchar_t b[64] = {};
    GetWindowTextW(h, b, 63);
    if (!b[0]) return fallback;
    return _wtoi(b);
}

static void SetEditInt(HWND h, int v)
{
    if (!h) return;
    wchar_t b[64] = {};
    wsprintfW(b, L"%d", v);
    SetWindowTextW(h, b);
}

static void SetComboByData(HWND hCombo, LPARAM data)
{
    if (!hCombo) return;
    int count = (int)SendMessageW(hCombo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++)
    {
        if (SendMessageW(hCombo, CB_GETITEMDATA, i, 0) == data)
        {
            SendMessageW(hCombo, CB_SETCURSEL, i, 0);
            return;
        }
    }
    if (count > 0) SendMessageW(hCombo, CB_SETCURSEL, 0, 0);
}

static LPARAM GetComboData(HWND hCombo, LPARAM fallback)
{
    if (!hCombo) return fallback;
    int sel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
    if (sel < 0) return fallback;
    return SendMessageW(hCombo, CB_GETITEMDATA, sel, 0);
}

static std::wstring GetComboText(HWND hCombo)
{
    if (!hCombo) return L"";
    int sel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
    if (sel < 0) return L"";
    wchar_t b[256] = {};
    SendMessageW(hCombo, CB_GETLBTEXT, sel, (LPARAM)b);
    return std::wstring(b);
}

// ----------------------------------------------------------------------------
// Scrolling host subclass (for mouse wheel + scroll routing)
// ----------------------------------------------------------------------------
static LRESULT CALLBACK ScrollHostSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
    switch (msg)
    {
    case WM_VSCROLL:
        if (gWnd) return SendMessageW(gWnd, WM_VSCROLL, wParam, (LPARAM)hwnd);
        return 0;
    case WM_MOUSEWHEEL:
        if (gWnd) return SendMessageW(gWnd, WM_MOUSEWHEEL, wParam, lParam);
        return 0;
    default:
        break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK ContentSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
    switch (msg)
    {
    case WM_COMMAND:
    case WM_HSCROLL:
    case WM_VSCROLL:
        if (gWnd) return SendMessageW(gWnd, msg, wParam, lParam);
        return 0;
    default:
        break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// Preset file IO
// ----------------------------------------------------------------------------
static std::map<std::wstring, std::wstring> LoadPresetFile(const std::wstring& path)
{
    std::map<std::wstring, std::wstring> out;

    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) return out;

    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return out;

    LARGE_INTEGER sz = {};
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart <= 0 || sz.QuadPart > (LONGLONG)(8 * 1024 * 1024))
    {
        CloseHandle(h);
        return out;
    }

    std::string bytes;
    bytes.resize((size_t)sz.QuadPart);
    DWORD rd = 0;
    BOOL ok = ReadFile(h, bytes.data(), (DWORD)bytes.size(), &rd, nullptr);
    CloseHandle(h);
    if (!ok || rd != (DWORD)bytes.size()) return out;

    if (bytes.size() >= 3 && (unsigned char)bytes[0] == 0xEF && (unsigned char)bytes[1] == 0xBB && (unsigned char)bytes[2] == 0xBF)
        bytes.erase(0, 3);

    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data(), (int)bytes.size(), nullptr, 0);
    if (wlen <= 0) return out;

    std::wstring text;
    text.resize((size_t)wlen);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data(), (int)bytes.size(), text.data(), wlen);

    std::wistringstream iss(text);
    std::wstring line;
    std::wstring section;

    while (std::getline(iss, line))
    {
        if (line.size() > 4096) continue;
        if (!line.empty() && line.back() == L'\r') line.pop_back();

        line = Trim(line);
        if (line.empty()) continue;
        if (line[0] == L';' || line[0] == L'#') continue;

        if (line.size() >= 3 && line.front() == L'[' && line.back() == L']')
        {
            section = Trim(line.substr(1, line.size() - 2));
            if (section.size() > 64) section.resize(64);
            continue;
        }

        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;

        std::wstring k = Trim(line.substr(0, eq));
        std::wstring v = Trim(line.substr(eq + 1));
        if (k.empty()) continue;
        if (k.size() > 128) k.resize(128);
        if (v.size() > 2048) v.resize(2048);

        if (!section.empty())
            k = section + L"." + k;

        out[k] = v;
    }

    return out;
}

static bool SavePresetFile(const std::wstring& path, const std::map<std::wstring, std::wstring>& kv)
{
    if (kv.empty()) return false;

    std::vector<std::wstring> sections;
    sections.reserve(16);

    for (auto& it : kv)
    {
        const std::wstring& full = it.first;
        size_t dot = full.find(L'.');
        if (dot == std::wstring::npos || dot == 0) continue;
        std::wstring sec = full.substr(0, dot);
        if (sec.empty()) continue;

        bool exists = false;
        for (auto& s : sections)
        {
            if (_wcsicmp(s.c_str(), sec.c_str()) == 0) { exists = true; break; }
        }
        if (!exists) sections.push_back(sec);
    }

    std::sort(sections.begin(), sections.end(), [&](const std::wstring& a, const std::wstring& b) {
        auto prio = [](const std::wstring& s) {
            if (_wcsicmp(s.c_str(), L"Display") == 0) return 0;
            if (_wcsicmp(s.c_str(), L"Advanced") == 0) return 1;
            if (_wcsicmp(s.c_str(), L"Audio") == 0) return 2;
            if (_wcsicmp(s.c_str(), L"Controls") == 0) return 3;
            if (_wcsicmp(s.c_str(), L"MAIN") == 0) return 4;
            if (_wcsicmp(s.c_str(), L"Messages") == 0) return 5;
            if (_wcsicmp(s.c_str(), L"General") == 0) return 6;
            if (_wcsicmp(s.c_str(), L"Menu") == 0) return 7;
            return 100;
            };
        int pa = prio(a), pb = prio(b);
        if (pa != pb) return pa < pb;
        return _wcsicmp(a.c_str(), b.c_str()) < 0;
        });

    std::wstring out;
    out.reserve(8192);

    for (auto& sec : sections)
    {
        out += L"[";
        out += sec;
        out += L"]\r\n";

        for (auto& it : kv)
        {
            const std::wstring& full = it.first;
            if (full.size() <= sec.size() + 1) continue;
            if (_wcsnicmp(full.c_str(), sec.c_str(), sec.size()) != 0) continue;
            if (full[sec.size()] != L'.') continue;

            std::wstring key = full.substr(sec.size() + 1);
            if (key.empty()) continue;

            out += key;
            out += L"=";
            out += it.second;
            out += L"\r\n";
        }
        out += L"\r\n";
    }

    std::string utf8 = WideToUtf8(out);
    return WriteFileUtf8Atomic(path, utf8);
}

// ----------------------------------------------------------------------------
// Preset allowlist + validation
// ----------------------------------------------------------------------------
static bool IsManagedKey(const std::wstring& fullKey)
{
    static const wchar_t* const kAllowed[] = {
        L"Display.iSize W",
        L"Display.iSize H",
        L"Display.bFull Screen",
        L"Display.iPresentInterval",
        L"Display.HDR",
        L"Display.iMultiSample",
        L"Display.iAdapter",
        L"Display.DistantLand",
        L"Display.DistantBuildings",
        L"Display.DistantTrees",
        L"Display.bDoBloom",
        L"Display.bShadowsOnGrass",
        L"Display.bDoCanopyShadowPass",
        L"Display.iShadowFilter",
        L"Display.fSpecualrStartMax",
        L"Display.bDynamicWindowReflections",
        L"Water.iWaterMult",
        L"Water.bUseWaterReflections",
        L"Water.bUseWaterReflectionsStatics",
        L"Water.bUseWaterReflectionsTrees",
        L"Water.bUseWaterDisplacements",
        L"Water.bUseWaterDepth",
        L"Decals.iMaxDecalsPerFrame",
        L"TerrainManager.uGridDistantCount",
        L"TerrainManager.uGridDistantTreeRange",
        L"Trees.uGridDistantTreeRange",
        L"Display.iTexMipMapSkip",
        L"LOD.fLODMultTrees",
        L"LOD.fLODMultActors",
        L"LOD.fLODMultItems",
        L"LOD.fLODMultObjects",
        L"Audio.MasterVolume",
        L"Audio.EffectsVolume",
        L"Audio.MusicVolume",
        L"Controls.MouseSensitivity",
        L"Controls.InvertMouse",
        L"Controls.AlwaysRun",
        L"Controls.AutoSave",
        L"General.bUseEyeEnvMapping",
        L"BlurShader.bUseBlurShader",
        L"Water.bUseWaterHiRes",
        L"Display.bActorSelfShadowing",
        L"Controls.fJumpAnimDelay",
        L"SpeedTree.fLODTreeMipMapLODBias",
        L"SpeedTree.fLocalTreeMipMapLODBias",
        L"MAIN.bEnableBorderRegion",
        L"Display.bAllowScreenShot",
        L"Messages.iFileLogging",
        L"Display.iDebugText",
        L"General.iFPSClamp",
        L"Menu.iConsoleHistorySize",
    };

    for (auto* k : kAllowed)
        if (_wcsicmp(fullKey.c_str(), k) == 0) return true;
    return false;
}

static bool TryParseBoolNormalized(const std::wstring& in, std::wstring& out01)
{
    std::wstring s = in;
    std::transform(s.begin(), s.end(), s.begin(), ::towlower);
    s = Trim(s);
    if (s == L"1" || s == L"true" || s == L"yes" || s == L"on") { out01 = L"1"; return true; }
    if (s == L"0" || s == L"false" || s == L"no" || s == L"off") { out01 = L"0"; return true; }
    return false;
}

static bool ComboContainsText(HWND combo, const std::wstring& value)
{
    if (!combo) return false;
    int count = (int)SendMessageW(combo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++)
    {
        wchar_t b[256] = {};
        SendMessageW(combo, CB_GETLBTEXT, i, (LPARAM)b);
        if (_wcsicmp(b, value.c_str()) == 0) return true;
    }
    return false;
}

static bool ComboContainsData(HWND combo, int data)
{
    if (!combo) return false;
    int count = (int)SendMessageW(combo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++)
        if ((int)SendMessageW(combo, CB_GETITEMDATA, i, 0) == data) return true;
    return false;
}

static bool ValidateAndNormalizePresetValue(const std::wstring& fullKey, const std::wstring& inValue, std::wstring& outValue)
{
    std::wstring v = Trim(inValue);
    if (v.size() > 2048) v.resize(2048);

    if (_wcsicmp(fullKey.c_str(), L"Display.iSize W") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.iSize H") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 320 || n > 16384) return false;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Display.iTexMipMapSkip") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 0) n = 0;
        if (n > 2) n = 2;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Display.iMultiSample") == 0)
    {
        int aa = _wtoi(v.c_str());
        if (!ComboContainsData(gAaCombo, aa)) return false;
        outValue = std::to_wstring(aa);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Display.iAdapter") == 0)
    {
        int adapter = _wtoi(v.c_str());
        if (!ComboContainsData(gAdapterCombo, adapter)) return false;
        outValue = std::to_wstring(adapter);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Display.bFull Screen") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.iPresentInterval") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.HDR") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.DistantLand") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.DistantBuildings") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.DistantTrees") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.bDoBloom") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.bShadowsOnGrass") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.bDoCanopyShadowPass") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.bDynamicWindowReflections") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Water.bUseWaterReflections") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Water.bUseWaterReflectionsStatics") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Water.bUseWaterReflectionsTrees") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Water.bUseWaterDisplacements") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Water.bUseWaterDepth") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Controls.InvertMouse") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Controls.AlwaysRun") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Controls.AutoSave") == 0 ||
        _wcsicmp(fullKey.c_str(), L"General.bUseEyeEnvMapping") == 0 ||
        _wcsicmp(fullKey.c_str(), L"BlurShader.bUseBlurShader") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Water.bUseWaterHiRes") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.bActorSelfShadowing") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.bAllowScreenShot") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Messages.iFileLogging") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Display.iDebugText") == 0)
    {
        std::wstring b;
        if (!TryParseBoolNormalized(v, b)) return false;
        outValue = b;
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"MAIN.bEnableBorderRegion") == 0)
    {
        std::wstring b;
        if (!TryParseBoolNormalized(v, b)) return false;
        outValue = b;
        return true;
    }

    auto clamp0100 = [](int n) {
        if (n < 0) n = 0;
        if (n > 100) n = 100;
        return n;
        };

    if (_wcsicmp(fullKey.c_str(), L"Audio.MasterVolume") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Audio.EffectsVolume") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Audio.MusicVolume") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Controls.MouseSensitivity") == 0)
    {
        int n = _wtoi(v.c_str());
        outValue = std::to_wstring(clamp0100(n));
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"General.iFPSClamp") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 0) n = 0;
        if (n > 300) n = 300;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Menu.iConsoleHistorySize") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 0) n = 0;
        if (n > 5000) n = 5000;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"LOD.fLODMultTrees") == 0 ||
        _wcsicmp(fullKey.c_str(), L"LOD.fLODMultActors") == 0 ||
        _wcsicmp(fullKey.c_str(), L"LOD.fLODMultItems") == 0 ||
        _wcsicmp(fullKey.c_str(), L"LOD.fLODMultObjects") == 0)
    {
        wchar_t* end = nullptr;
        double d = wcstod(v.c_str(), &end);
        if (end == v.c_str()) return false;
        if (d < 0.0) d = 0.0;
        if (d > 15.0) d = 15.0;
        outValue = FloatToWString(d, 4);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Display.iShadowFilter") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 0) n = 0;
        if (n > 3) n = 3;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Water.iWaterMult") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 1) n = 1;
        if (n > 3) n = 3;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Decals.iMaxDecalsPerFrame") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 0) n = 0;
        if (n > 50) n = 50;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"TerrainManager.uGridDistantCount") == 0 ||
        _wcsicmp(fullKey.c_str(), L"TerrainManager.uGridDistantTreeRange") == 0 ||
        _wcsicmp(fullKey.c_str(), L"Trees.uGridDistantTreeRange") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 0) n = 0;
        if (n > 64) n = 64;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Display.fSpecualrStartMax") == 0)
    {
        int n = _wtoi(v.c_str());
        if (n < 0) n = 0;
        if (n > 100) n = 100;
        outValue = std::to_wstring(n);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"Controls.fJumpAnimDelay") == 0)
    {
        double f = _wtof(v.c_str());
        if (f < 0.0) f = 0.0;
        if (f > 1.0) f = 1.0;
        outValue = FloatToWString(f, 4);
        return true;
    }

    if (_wcsicmp(fullKey.c_str(), L"SpeedTree.fLODTreeMipMapLODBias") == 0 ||
        _wcsicmp(fullKey.c_str(), L"SpeedTree.fLocalTreeMipMapLODBias") == 0)
    {
        double f = _wtof(v.c_str());
        if (f < -2.0) f = -2.0;
        if (f > 2.0) f = 2.0;
        outValue = FloatToWString(f, 4);
        return true;
    }

    return false;
}

static std::map<std::wstring, std::wstring> FilterPresetForApply(const std::map<std::wstring, std::wstring>& in)
{
    std::map<std::wstring, std::wstring> out;
    for (auto& it : in)
    {
        if (!IsManagedKey(it.first)) continue;
        std::wstring normalized;
        if (!ValidateAndNormalizePresetValue(it.first, it.second, normalized)) continue;
        out[it.first] = normalized;
    }
    return out;
}

// ----------------------------------------------------------------------------
// Resolution enumeration
// ----------------------------------------------------------------------------
static void PopulateResolutions(HWND hCombo)
{
    if (!hCombo) return;

    SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);

    std::vector<std::wstring> seen;

    DEVMODEW dm = {};
    dm.dmSize = sizeof(dm);

    for (DWORD i = 0; EnumDisplaySettingsW(nullptr, i, &dm); i++)
    {
        if (!(dm.dmFields & DM_PELSWIDTH) || !(dm.dmFields & DM_PELSHEIGHT))
            continue;
        if (dm.dmPelsWidth < 800 || dm.dmPelsHeight < 600)
            continue;

        std::wstringstream ss;
        ss << dm.dmPelsWidth << L"x" << dm.dmPelsHeight;
        std::wstring r = ss.str();

        if (std::find(seen.begin(), seen.end(), r) != seen.end())
            continue;

        seen.push_back(r);
    }

    auto key = [](const std::wstring& s) {
        int w = 0, h = 0;
        swscanf_s(s.c_str(), L"%dx%d", &w, &h);
        return (long long)w * 100000LL + h;
        };
    std::sort(seen.begin(), seen.end(), [&](const std::wstring& a, const std::wstring& b) { return key(a) < key(b); });

    for (auto& r : seen)
        AddComboItem(hCombo, r.c_str(), 0);

    std::wstring curW = GetINIString(L"Display", L"iSize W", L"");
    std::wstring curH = GetINIString(L"Display", L"iSize H", L"");
    std::wstring cur = (!curW.empty() && !curH.empty()) ? (curW + L"x" + curH) : L"1920x1080";

    int count = (int)SendMessageW(hCombo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++)
    {
        wchar_t b[256] = {};
        SendMessageW(hCombo, CB_GETLBTEXT, i, (LPARAM)b);
        if (cur == b)
        {
            SendMessageW(hCombo, CB_SETCURSEL, i, 0);
            return;
        }
    }
    if (count > 0) SendMessageW(hCombo, CB_SETCURSEL, count - 1, 0);
}

// ----------------------------------------------------------------------------
// Apply state to controls / controls to INI
// ----------------------------------------------------------------------------
static void ApplyStateToControls(const std::map<std::wstring, std::wstring>& kv, bool preserveResolution)
{
    auto get = [&](const wchar_t* k, const wchar_t* defv) -> std::wstring {
        auto it = kv.find(k);
        return (it != kv.end()) ? it->second : std::wstring(defv);
        };

    if (!preserveResolution && gResCombo)
    {
        std::wstring r = get(L"Display.iSize W", L"1920") + L"x" + get(L"Display.iSize H", L"1080");
        int count = (int)SendMessageW(gResCombo, CB_GETCOUNT, 0, 0);
        for (int i = 0; i < count; i++)
        {
            wchar_t b[256] = {};
            SendMessageW(gResCombo, CB_GETLBTEXT, i, (LPARAM)b);
            if (r == b)
            {
                SendMessageW(gResCombo, CB_SETCURSEL, i, 0);
                break;
            }
        }
    }

    int fullScreen = _wtoi(get(L"Display.bFull Screen", L"1").c_str());
    if (gFullscreenRadio && gWindowedRadio)
    {
        SendMessageW((fullScreen != 0) ? gFullscreenRadio : gWindowedRadio, BM_SETCHECK, BST_CHECKED, 0);
    }
    else
    {
        SetComboByData(gWinModeCombo, (fullScreen != 0) ? 0 : 1);
    }

    SendMessageW(gVsyncCheck, BM_SETCHECK, (_wtoi(get(L"Display.iPresentInterval", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gHdrCheck, BM_SETCHECK, (_wtoi(get(L"Display.HDR", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    int aa = _wtoi(get(L"Display.iMultiSample", L"0").c_str());
    SetComboByData(gAaCombo, aa);
    SetComboByData(gAdapterCombo, _wtoi(get(L"Display.iAdapter", L"0").c_str()));

    SendMessageW(gDistantLandCheck, BM_SETCHECK, (_wtoi(get(L"Display.DistantLand", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDistantBuildingsCheck, BM_SETCHECK, (_wtoi(get(L"Display.DistantBuildings", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDistantTreesCheck, BM_SETCHECK, (_wtoi(get(L"Display.DistantTrees", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessageW(gBloomCheck, BM_SETCHECK, (_wtoi(get(L"Display.bDoBloom", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    if (gEffectsNoneRadio)
    {
        const BOOL noneEffects = (SendMessageW(gBloomCheck, BM_GETCHECK, 0, 0) != BST_CHECKED)
            && (SendMessageW(gHdrCheck, BM_GETCHECK, 0, 0) != BST_CHECKED);
        SendMessageW(gEffectsNoneRadio, BM_SETCHECK, noneEffects ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    SendMessageW(gShadowsOnGrassCheck, BM_SETCHECK, (_wtoi(get(L"Display.bShadowsOnGrass", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gTreeCanopyShadowsCheck, BM_SETCHECK, (_wtoi(get(L"Display.bDoCanopyShadowPass", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SetComboByData(gShadowFilterCombo, _wtoi(get(L"Display.iShadowFilter", L"2").c_str()));

    double specValue = _wtof(get(L"Display.fSpecualrStartMax", L"50").c_str());
    int specPct = 0;
    if (specValue <= 100.0)
        specPct = (int)(specValue + 0.5);
    else
        specPct = (int)((specValue / 1000.0) * 100.0 + 0.5);
    if (specPct < 0) specPct = 0;
    if (specPct > 100) specPct = 100;
    SetTrackPos(gSpecularTrack, specPct);
    SetTextInt(gSpecularVal, specPct);

    SetComboByData(gWaterDetailCombo, _wtoi(get(L"Water.iWaterMult", L"2").c_str()));
    SendMessageW(gWaterReflectionsCheck, BM_SETCHECK, (_wtoi(get(L"Water.bUseWaterReflections", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterReflStaticsCheck, BM_SETCHECK, (_wtoi(get(L"Water.bUseWaterReflectionsStatics", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterReflTreesCheck, BM_SETCHECK, (_wtoi(get(L"Water.bUseWaterReflectionsTrees", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterRipplesCheck, BM_SETCHECK, (_wtoi(get(L"Water.bUseWaterDisplacements", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWindowReflectionsCheck, BM_SETCHECK, (_wtoi(get(L"Display.bDynamicWindowReflections", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterDepthCheck, BM_SETCHECK, (_wtoi(get(L"Water.bUseWaterDepth", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SetComboByData(gBloodDecalsCombo, _wtoi(get(L"Decals.iMaxDecalsPerFrame", L"10").c_str()));

    SetEditInt(gGridDistantCountEdit, _wtoi(get(L"TerrainManager.uGridDistantCount", L"30").c_str()));
    std::wstring treeRange = get(L"Trees.uGridDistantTreeRange", L"");
    if (treeRange.empty()) treeRange = get(L"TerrainManager.uGridDistantTreeRange", L"30");
    SetEditInt(gGridDistantTreeRangeEdit, _wtoi(treeRange.c_str()));

    std::wstring tex = L"Large";
    auto itTexSkip = kv.find(L"Display.iTexMipMapSkip");
    if (itTexSkip != kv.end())
    {
        int texSkip = _wtoi(itTexSkip->second.c_str());
        if (texSkip <= 0) tex = L"Large";
        else if (texSkip == 1) tex = L"Medium";
        else tex = L"Small";
    }
    int tcount = (int)SendMessageW(gTexSizeCombo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < tcount; i++)
    {
        wchar_t b[64] = {};
        SendMessageW(gTexSizeCombo, CB_GETLBTEXT, i, (LPARAM)b);
        if (tex == b) { SendMessageW(gTexSizeCombo, CB_SETCURSEL, i, 0); break; }
    }

    auto clamp01 = [](int v) { return (v < 0) ? 0 : (v > 100) ? 100 : v; };

    int tree = 50;
    auto itTree = kv.find(L"LOD.fLODMultTrees");
    if (itTree != kv.end()) tree = (int)(_wtof(itTree->second.c_str()) * 150.0 + 0.5);
    tree = clamp01(tree);

    int actor = 50;
    auto itActor = kv.find(L"LOD.fLODMultActors");
    if (itActor != kv.end()) actor = (int)(_wtof(itActor->second.c_str()) * 15.0 + 0.5);
    actor = clamp01(actor);

    int item = 50;
    auto itItem = kv.find(L"LOD.fLODMultItems");
    if (itItem != kv.end()) item = (int)(_wtof(itItem->second.c_str()) * 15.0 + 0.5);
    item = clamp01(item);

    int obj = 50;
    auto itObj = kv.find(L"LOD.fLODMultObjects");
    if (itObj != kv.end()) obj = (int)(_wtof(itObj->second.c_str()) * 15.0 + 0.5);
    obj = clamp01(obj);

    SetTrackPos(gTreeFadeTrack, tree);
    SetTrackPos(gActorFadeTrack, actor);
    SetTrackPos(gItemFadeTrack, item);
    SetTrackPos(gObjectFadeTrack, obj);
    SetTextInt(gTreeFadeVal, tree);
    SetTextInt(gActorFadeVal, actor);
    SetTextInt(gItemFadeVal, item);
    SetTextInt(gObjectFadeVal, obj);

    int mv = clamp01(_wtoi(get(L"Audio.MasterVolume", L"80").c_str()));
    int ev = clamp01(_wtoi(get(L"Audio.EffectsVolume", L"80").c_str()));
    int mus = clamp01(_wtoi(get(L"Audio.MusicVolume", L"70").c_str()));

    SetTrackPos(gMasterVolTrack, mv);
    SetTrackPos(gEffectsVolTrack, ev);
    SetTrackPos(gMusicVolTrack, mus);
    SetTextInt(gMasterVolVal, mv);
    SetTextInt(gEffectsVolVal, ev);
    SetTextInt(gMusicVolVal, mus);

    std::wstring msText = get(L"Controls.MouseSensitivity", L"");
    int ms = 50;
    if (!msText.empty())
    {
        ms = clamp01(_wtoi(msText.c_str()));
    }
    else
    {
        const std::wstring legacyFms = get(L"Controls.fMouseSensitivity", L"0.0020");
        ms = clamp01((int)(_wtof(legacyFms.c_str()) * 25000.0 + 0.5));
    }
    SetTrackPos(gMouseSensTrack, ms);
    SetTextInt(gMouseSensVal, ms);

    std::wstring invertText = get(L"Controls.InvertMouse", L"");
    if (invertText.empty()) invertText = get(L"Controls.bInvertYValues", L"0");
    std::wstring runText = get(L"Controls.AlwaysRun", L"");
    if (runText.empty()) runText = get(L"Controls.Always Run", L"1");
    SendMessageW(gInvertMouseCheck, BM_SETCHECK, (_wtoi(invertText.c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gAlwaysRunCheck, BM_SETCHECK, (_wtoi(runText.c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gAutoSaveCheck, BM_SETCHECK, (_wtoi(get(L"Controls.AutoSave", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gUseEyeEnvMappingCheck, BM_SETCHECK, (_wtoi(get(L"General.bUseEyeEnvMapping", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gUseBlurShaderCheck, BM_SETCHECK, (_wtoi(get(L"BlurShader.bUseBlurShader", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterHiResCheck, BM_SETCHECK, (_wtoi(get(L"Water.bUseWaterHiRes", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gActorSelfShadowingCheck, BM_SETCHECK, (_wtoi(get(L"Display.bActorSelfShadowing", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    int jumpDelayTicks = (int)(_wtof(get(L"Controls.fJumpAnimDelay", L"0.2500").c_str()) * 1000.0);
    if (jumpDelayTicks < 0) jumpDelayTicks = 0;
    if (jumpDelayTicks > 1000) jumpDelayTicks = 1000;
    SetTrackPos(gJumpDelayTrack, jumpDelayTicks);
    SetTextFloat(gJumpDelayVal, jumpDelayTicks / 1000.0, 3);

    int treeBiasTicks = (int)((_wtof(get(L"SpeedTree.fLODTreeMipMapLODBias", L"-0.5000").c_str()) + 2.0) * 1000.0);
    if (treeBiasTicks < 0) treeBiasTicks = 0;
    if (treeBiasTicks > 4000) treeBiasTicks = 4000;
    SetTrackPos(gLodTreeBiasTrack, treeBiasTicks);
    SetTextFloat(gLodTreeBiasVal, (treeBiasTicks / 1000.0) - 2.0, 3);

    int localBiasTicks = (int)((_wtof(get(L"SpeedTree.fLocalTreeMipMapLODBias", L"0.0000").c_str()) + 2.0) * 1000.0);
    if (localBiasTicks < 0) localBiasTicks = 0;
    if (localBiasTicks > 4000) localBiasTicks = 4000;
    SetTrackPos(gLodLocalTreeBiasTrack, localBiasTicks);
    SetTextFloat(gLodLocalTreeBiasVal, (localBiasTicks / 1000.0) - 2.0, 3);

    int ber = _wtoi(get(L"MAIN.bEnableBorderRegion", L"1").c_str());
    SendMessageW(gDisableBorderRegionsCheck, BM_SETCHECK, (ber == 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessageW(gDevAllowScreenshotCheck, BM_SETCHECK, (_wtoi(get(L"Display.bAllowScreenShot", L"1").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDevFileLoggingCheck, BM_SETCHECK, (_wtoi(get(L"Messages.iFileLogging", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDevDebugTextCheck, BM_SETCHECK, (_wtoi(get(L"Display.iDebugText", L"0").c_str()) != 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SetEditInt(gDevFpsClampEdit, _wtoi(get(L"General.iFPSClamp", L"0").c_str()));
    SetEditInt(gDevConsoleHistoryEdit, _wtoi(get(L"Menu.iConsoleHistorySize", L"100").c_str()));
}

static void ApplyControlsToINI()
{
    {
        std::wstring res = GetComboText(gResCombo);
        size_t x = res.find(L'x');
        if (x != std::wstring::npos)
        {
            std::wstring w = Trim(res.substr(0, x));
            std::wstring h = Trim(res.substr(x + 1));
            if (!w.empty() && !h.empty())
            {
                WriteINIString(L"Display", L"iSize W", w);
                WriteINIString(L"Display", L"iSize H", h);
                WriteINIString(L"Display", L"Resolution", w + L"x" + h);
            }
        }
    }

    int fullScreen = 1;
    if (gFullscreenRadio && gWindowedRadio)
    {
        fullScreen = (SendMessageW(gFullscreenRadio, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    }
    else
    {
        int mode = (int)GetComboData(gWinModeCombo, 0);
        fullScreen = (mode == 0) ? 1 : 0;
    }
    WriteINIInt(L"Display", L"bFull Screen", fullScreen);
    WriteINIInt(L"Display", L"Windowed", fullScreen ? 0 : 1);

    int vsync = (SendMessageW(gVsyncCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    WriteINIInt(L"Display", L"iPresentInterval", vsync);
    WriteINIInt(L"Display", L"VSync", vsync);
    if (gEffectsNoneRadio && SendMessageW(gEffectsNoneRadio, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        SendMessageW(gHdrCheck, BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessageW(gBloomCheck, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    WriteINIInt(L"Display", L"HDR", (SendMessageW(gHdrCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);

    int aa = (int)GetComboData(gAaCombo, 0);
    WriteINIInt(L"Display", L"iMultiSample", aa);
    WriteINIInt(L"Display", L"AA", aa);
    WriteINIInt(L"Display", L"iAdapter", (int)GetComboData(gAdapterCombo, 0));

    const int distantLand = (SendMessageW(gDistantLandCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    const int distantBuildings = (SendMessageW(gDistantBuildingsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    const int distantTrees = (SendMessageW(gDistantTreesCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    WriteINIInt(L"Display", L"DistantLand", distantLand);
    WriteINIInt(L"Display", L"DistantBuildings", distantBuildings);
    WriteINIInt(L"Display", L"DistantTrees", distantTrees);
    WriteINIInt(L"LOD", L"bDisplayLODLand", distantLand);
    WriteINIInt(L"LOD", L"bDisplayLODBuildings", distantBuildings);
    WriteINIInt(L"LOD", L"bDisplayLODTrees", distantTrees);

    WriteINIInt(L"Display", L"bDoBloom", (SendMessageW(gBloomCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Display", L"bShadowsOnGrass", (SendMessageW(gShadowsOnGrassCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Display", L"bDoCanopyShadowPass", (SendMessageW(gTreeCanopyShadowsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Display", L"iShadowFilter", (int)GetComboData(gShadowFilterCombo, 2));

    float specMax = 1000.0f * ((float)GetTrackPos(gSpecularTrack, 50) / 100.0f);
    WriteINIString(L"Display", L"fSpecualrStartMax", FloatToWString(specMax, 4));

    WriteINIInt(L"Water", L"iWaterMult", (int)GetComboData(gWaterDetailCombo, 2));
    WriteINIInt(L"Water", L"bUseWaterReflections", (SendMessageW(gWaterReflectionsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Water", L"bUseWaterReflectionsStatics", (SendMessageW(gWaterReflStaticsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Water", L"bUseWaterReflectionsTrees", (SendMessageW(gWaterReflTreesCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Water", L"bUseWaterDisplacements", (SendMessageW(gWaterRipplesCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Display", L"bDynamicWindowReflections", (SendMessageW(gWindowReflectionsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Water", L"bUseWaterDepth", (SendMessageW(gWaterDepthCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Decals", L"iMaxDecalsPerFrame", (int)GetComboData(gBloodDecalsCombo, 10));

    WriteINIInt(L"TerrainManager", L"uGridDistantCount", GetEditInt(gGridDistantCountEdit, 30));
    WriteINIInt(L"TerrainManager", L"uGridDistantTreeRange", GetEditInt(gGridDistantTreeRangeEdit, 30));
    WriteINIInt(L"Trees", L"uGridDistantTreeRange", GetEditInt(gGridDistantTreeRangeEdit, 30));

    const std::wstring textureSize = GetComboText(gTexSizeCombo);

    int texMipMapSkip = 0;
    if (_wcsicmp(textureSize.c_str(), L"Medium") == 0) texMipMapSkip = 1;
    else if (_wcsicmp(textureSize.c_str(), L"Small") == 0) texMipMapSkip = 2;
    WriteINIInt(L"Display", L"iTexMipMapSkip", texMipMapSkip);
    WriteINIString(L"Advanced", L"TextureSize", textureSize);

    const int treeFade = GetTrackPos(gTreeFadeTrack, 50);
    const int actorFade = GetTrackPos(gActorFadeTrack, 50);
    const int itemFade = GetTrackPos(gItemFadeTrack, 50);
    const int objectFade = GetTrackPos(gObjectFadeTrack, 50);

    WriteINIInt(L"Advanced", L"TreeFade", treeFade);
    WriteINIInt(L"Advanced", L"ActorFade", actorFade);
    WriteINIInt(L"Advanced", L"ItemFade", itemFade);
    WriteINIInt(L"Advanced", L"ObjectFade", objectFade);

    WriteINIString(L"LOD", L"fLODMultTrees", FloatToWString(treeFade / 150.0, 4));
    WriteINIString(L"LOD", L"fLODMultActors", FloatToWString(actorFade / 15.0, 4));
    WriteINIString(L"LOD", L"fLODMultItems", FloatToWString(itemFade / 15.0, 4));
    WriteINIString(L"LOD", L"fLODMultObjects", FloatToWString(objectFade / 15.0, 4));

    WriteINIInt(L"Audio", L"MasterVolume", GetTrackPos(gMasterVolTrack, 80));
    WriteINIInt(L"Audio", L"EffectsVolume", GetTrackPos(gEffectsVolTrack, 80));
    WriteINIInt(L"Audio", L"MusicVolume", GetTrackPos(gMusicVolTrack, 70));

    int mouseSensitivity = GetTrackPos(gMouseSensTrack, 50);
    int invertMouse = (SendMessageW(gInvertMouseCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    int alwaysRun = (SendMessageW(gAlwaysRunCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    WriteINIInt(L"Controls", L"MouseSensitivity", mouseSensitivity);
    WriteINIString(L"Controls", L"fMouseSensitivity", FloatToWString(mouseSensitivity / 25000.0, 4));
    WriteINIInt(L"Controls", L"InvertMouse", invertMouse);
    WriteINIInt(L"Controls", L"bInvertYValues", invertMouse);
    WriteINIInt(L"Controls", L"AlwaysRun", alwaysRun);
    WriteINIInt(L"Controls", L"Always Run", alwaysRun);
    WriteINIInt(L"Controls", L"AutoSave", (SendMessageW(gAutoSaveCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"General", L"bUseEyeEnvMapping", (SendMessageW(gUseEyeEnvMappingCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"BlurShader", L"bUseBlurShader", (SendMessageW(gUseBlurShaderCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Water", L"bUseWaterHiRes", (SendMessageW(gWaterHiResCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Display", L"bActorSelfShadowing", (SendMessageW(gActorSelfShadowingCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIString(L"Controls", L"fJumpAnimDelay", FloatToWString(GetTrackPos(gJumpDelayTrack, 250) / 1000.0, 4));
    WriteINIString(L"SpeedTree", L"fLODTreeMipMapLODBias", FloatToWString((GetTrackPos(gLodTreeBiasTrack, 1500) / 1000.0) - 2.0, 4));
    WriteINIString(L"SpeedTree", L"fLocalTreeMipMapLODBias", FloatToWString((GetTrackPos(gLodLocalTreeBiasTrack, 2000) / 1000.0) - 2.0, 4));

    WriteINIInt(L"MAIN", L"bEnableBorderRegion",
        (SendMessageW(gDisableBorderRegionsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 0 : 1);
    WriteINIInt(L"Launcher", L"bEnableConstructionSetButton",
        (SendMessageW(gEnableConstructionSetButtonCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Launcher", L"bCloseOnLaunch",
        (SendMessageW(gCloseLauncherOnLaunchCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);

    WriteINIInt(L"Display", L"bAllowScreenShot", (SendMessageW(gDevAllowScreenshotCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Messages", L"iFileLogging", (SendMessageW(gDevFileLoggingCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);
    WriteINIInt(L"Display", L"iDebugText", (SendMessageW(gDevDebugTextCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0);

    {
        int fps = GetEditInt(gDevFpsClampEdit, 0);
        if (fps < 0) fps = 0;
        if (fps > 300) fps = 300;
        WriteINIInt(L"General", L"iFPSClamp", fps);
    }
    {
        int hist = GetEditInt(gDevConsoleHistoryEdit, 100);
        if (hist < 0) hist = 0;
        if (hist > 5000) hist = 5000;
        WriteINIInt(L"Menu", L"iConsoleHistorySize", hist);
    }
}

// ----------------------------------------------------------------------------
// Current state capture (for saving presets)
// ----------------------------------------------------------------------------
static std::map<std::wstring, std::wstring> CaptureCurrentState()
{
    std::map<std::wstring, std::wstring> kv;

    std::wstring res = GetComboText(gResCombo);
    size_t x = res.find(L'x');
    if (x != std::wstring::npos)
    {
        kv[L"Display.iSize W"] = Trim(res.substr(0, x));
        kv[L"Display.iSize H"] = Trim(res.substr(x + 1));
    }

    int fullScreen = 1;
    if (gFullscreenRadio && gWindowedRadio)
        fullScreen = (SendMessageW(gFullscreenRadio, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
    else
        fullScreen = ((int)GetComboData(gWinModeCombo, 0) == 0) ? 1 : 0;
    kv[L"Display.bFull Screen"] = fullScreen ? L"1" : L"0";

    kv[L"Display.iPresentInterval"] = (SendMessageW(gVsyncCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.HDR"] = (SendMessageW(gHdrCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";

    int aa = (int)GetComboData(gAaCombo, 0);
    kv[L"Display.iMultiSample"] = std::to_wstring(aa);
    kv[L"Display.iAdapter"] = std::to_wstring((int)GetComboData(gAdapterCombo, 0));

    kv[L"Display.DistantLand"] = (SendMessageW(gDistantLandCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.DistantBuildings"] = (SendMessageW(gDistantBuildingsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.DistantTrees"] = (SendMessageW(gDistantTreesCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";

    kv[L"Display.bDoBloom"] = (SendMessageW(gBloomCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.bShadowsOnGrass"] = (SendMessageW(gShadowsOnGrassCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.bDoCanopyShadowPass"] = (SendMessageW(gTreeCanopyShadowsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.iShadowFilter"] = std::to_wstring((int)GetComboData(gShadowFilterCombo, 2));

    // Stored as 0..100 (slider percent). Converted to 0..1000 float on INI apply.
    kv[L"Display.fSpecualrStartMax"] = std::to_wstring(GetTrackPos(gSpecularTrack, 50));

    kv[L"Water.iWaterMult"] = std::to_wstring((int)GetComboData(gWaterDetailCombo, 2));
    kv[L"Water.bUseWaterReflections"] = (SendMessageW(gWaterReflectionsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Water.bUseWaterReflectionsStatics"] = (SendMessageW(gWaterReflStaticsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Water.bUseWaterReflectionsTrees"] = (SendMessageW(gWaterReflTreesCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Water.bUseWaterDisplacements"] = (SendMessageW(gWaterRipplesCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.bDynamicWindowReflections"] = (SendMessageW(gWindowReflectionsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Water.bUseWaterDepth"] = (SendMessageW(gWaterDepthCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Decals.iMaxDecalsPerFrame"] = std::to_wstring((int)GetComboData(gBloodDecalsCombo, 10));

    kv[L"TerrainManager.uGridDistantCount"] = std::to_wstring(GetEditInt(gGridDistantCountEdit, 30));
    kv[L"Trees.uGridDistantTreeRange"] = std::to_wstring(GetEditInt(gGridDistantTreeRangeEdit, 30));

    int texMipMapSkip = 0;
    std::wstring textureSize = GetComboText(gTexSizeCombo);
    if (_wcsicmp(textureSize.c_str(), L"Medium") == 0) texMipMapSkip = 1;
    else if (_wcsicmp(textureSize.c_str(), L"Small") == 0) texMipMapSkip = 2;
    kv[L"Display.iTexMipMapSkip"] = std::to_wstring(texMipMapSkip);

    const int treeFade = GetTrackPos(gTreeFadeTrack, 50);
    const int actorFade = GetTrackPos(gActorFadeTrack, 50);
    const int itemFade = GetTrackPos(gItemFadeTrack, 50);
    const int objectFade = GetTrackPos(gObjectFadeTrack, 50);
    kv[L"LOD.fLODMultTrees"] = FloatToWString(treeFade / 150.0, 4);
    kv[L"LOD.fLODMultActors"] = FloatToWString(actorFade / 15.0, 4);
    kv[L"LOD.fLODMultItems"] = FloatToWString(itemFade / 15.0, 4);
    kv[L"LOD.fLODMultObjects"] = FloatToWString(objectFade / 15.0, 4);

    kv[L"Audio.MasterVolume"] = std::to_wstring(GetTrackPos(gMasterVolTrack, 80));
    kv[L"Audio.EffectsVolume"] = std::to_wstring(GetTrackPos(gEffectsVolTrack, 80));
    kv[L"Audio.MusicVolume"] = std::to_wstring(GetTrackPos(gMusicVolTrack, 70));

    kv[L"Controls.MouseSensitivity"] = std::to_wstring(GetTrackPos(gMouseSensTrack, 50));
    kv[L"Controls.InvertMouse"] = (SendMessageW(gInvertMouseCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Controls.AlwaysRun"] = (SendMessageW(gAlwaysRunCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Controls.AutoSave"] = (SendMessageW(gAutoSaveCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"General.bUseEyeEnvMapping"] = (SendMessageW(gUseEyeEnvMappingCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"BlurShader.bUseBlurShader"] = (SendMessageW(gUseBlurShaderCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Water.bUseWaterHiRes"] = (SendMessageW(gWaterHiResCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.bActorSelfShadowing"] = (SendMessageW(gActorSelfShadowingCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Controls.fJumpAnimDelay"] = FloatToWString(GetTrackPos(gJumpDelayTrack, 250) / 1000.0, 4);
    kv[L"SpeedTree.fLODTreeMipMapLODBias"] = FloatToWString((GetTrackPos(gLodTreeBiasTrack, 1500) / 1000.0) - 2.0, 4);
    kv[L"SpeedTree.fLocalTreeMipMapLODBias"] = FloatToWString((GetTrackPos(gLodLocalTreeBiasTrack, 2000) / 1000.0) - 2.0, 4);

    kv[L"MAIN.bEnableBorderRegion"] = (SendMessageW(gDisableBorderRegionsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"0" : L"1";

    kv[L"Display.bAllowScreenShot"] = (SendMessageW(gDevAllowScreenshotCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Messages.iFileLogging"] = (SendMessageW(gDevFileLoggingCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"Display.iDebugText"] = (SendMessageW(gDevDebugTextCheck, BM_GETCHECK, 0, 0) == BST_CHECKED) ? L"1" : L"0";
    kv[L"General.iFPSClamp"] = std::to_wstring(GetEditInt(gDevFpsClampEdit, 0));
    kv[L"Menu.iConsoleHistorySize"] = std::to_wstring(GetEditInt(gDevConsoleHistoryEdit, 100));

    return kv;
}

// ----------------------------------------------------------------------------
// Preset combo management
// ----------------------------------------------------------------------------
static void PopulatePresetCombo()
{
    if (!gPresetCombo) return;

    SendMessageW(gPresetCombo, CB_RESETCONTENT, 0, 0);
    gPresetPaths.clear();

    const std::wstring dir = PresetsDir();
    WIN32_FIND_DATAW fd = {};
    std::wstring glob = dir;
    glob += L"\\*.txt";
    HANDLE h = FindFirstFileW(glob.c_str(), &fd);

    struct Item { std::wstring name; std::wstring path; };
    std::vector<Item> items;

    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

            std::wstring file = fd.cFileName;
            if (file.size() < 5) continue;
            if (_wcsicmp(file.substr(file.size() - 4).c_str(), L".txt") != 0) continue;

            std::wstring name = file.substr(0, file.size() - 4);
            name = Trim(name);
            if (name.empty()) continue;
            if (name.size() > 64) name.resize(64);

            bool dup = false;
            for (auto& it : items)
            {
                if (_wcsicmp(it.name.c_str(), name.c_str()) == 0) { dup = true; break; }
            }
            if (dup) continue;

            Item it;
            it.name = name;
            it.path = dir + L"\\" + file;
            items.push_back(std::move(it));

        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }

    std::sort(items.begin(), items.end(), [&](const Item& a, const Item& b) {
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
        });

    for (size_t i = 0; i < items.size(); i++)
    {
        gPresetPaths.push_back(items[i].path);
        AddComboItem(gPresetCombo, items[i].name.c_str(), (LPARAM)(INT_PTR)i);
    }

    if (!items.empty())
        SendMessageW(gPresetCombo, CB_SETCURSEL, 0, 0);
}

static void OnPresetChange()
{
    int sel = (int)SendMessageW(gPresetCombo, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR)
    {
        gPendingPresetPath.clear();
        return;
    }

    LRESULT data = SendMessageW(gPresetCombo, CB_GETITEMDATA, sel, 0);
    size_t idx = (size_t)(INT_PTR)data;
    if (idx >= gPresetPaths.size())
    {
        gPendingPresetPath.clear();
        return;
    }

    gPendingPresetPath = gPresetPaths[idx];
}

static void OnSavePreset()
{
    auto kv = CaptureCurrentState();

    const int resp = MessageBoxW(
        gWnd,
        L"New Custom Preset?\r\n\r\nYes = create a new Custom (n)\r\nNo = overwrite Custom",
        L"Save Preset",
        MB_ICONQUESTION | MB_YESNO);

    std::wstring displayName;
    std::wstring path;

    if (resp == IDYES)
    {
        path = MakeUniquePresetPath(L"Custom", &displayName);
    }
    else
    {
        displayName = L"Custom";
        path = PresetsDir();
        path += L"\\Custom.txt";
    }

    if (!SavePresetFile(path, kv))
    {
        std::wstring msg = L"Failed to write preset file:\r\n\r\n" + path;
        MessageBoxW(gWnd, msg.c_str(), L"Presets", MB_OK | MB_ICONERROR);
        return;
    }

    PopulatePresetCombo();

    int count = (int)SendMessageW(gPresetCombo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++)
    {
        LRESULT data = SendMessageW(gPresetCombo, CB_GETITEMDATA, i, 0);
        size_t idx = (size_t)(INT_PTR)data;
        if (idx < gPresetPaths.size() && _wcsicmp(gPresetPaths[idx].c_str(), path.c_str()) == 0)
        {
            SendMessageW(gPresetCombo, CB_SETCURSEL, i, 0);
            break;
        }
    }

    std::wstring msg = L"Preset saved as: " + displayName + L"\r\n\r\n" + path;
    MessageBoxW(gWnd, msg.c_str(), L"Presets", MB_OK | MB_ICONINFORMATION);
}

// ----------------------------------------------------------------------------
// INI -> controls
// ----------------------------------------------------------------------------
static void LoadFromINI()
{
    PopulateResolutions(gResCombo);

    int fullScreen = GetINIInt(L"Display", L"bFull Screen", INT_MIN);
    if (fullScreen == INT_MIN)
    {
        int windowed = GetINIInt(L"Display", L"Windowed", 0);
        fullScreen = windowed ? 0 : 1;
    }
    if (gFullscreenRadio && gWindowedRadio)
        SendMessageW((fullScreen != 0) ? gFullscreenRadio : gWindowedRadio, BM_SETCHECK, BST_CHECKED, 0);
    else
        SetComboByData(gWinModeCombo, (fullScreen != 0) ? 0 : 1);

    int vsync = GetINIInt(L"Display", L"iPresentInterval", INT_MIN);
    if (vsync == INT_MIN) vsync = GetINIInt(L"Display", L"VSync", 1);
    SendMessageW(gVsyncCheck, BM_SETCHECK, vsync ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gHdrCheck, BM_SETCHECK, GetINIInt(L"Display", L"HDR", 1) ? BST_CHECKED : BST_UNCHECKED, 0);

    int aa = GetINIInt(L"Display", L"iMultiSample", INT_MIN);
    if (aa == INT_MIN) aa = GetINIInt(L"Display", L"AA", 0);
    SetComboByData(gAaCombo, aa);
    SetComboByData(gAdapterCombo, GetINIInt(L"Display", L"iAdapter", 0));

    int distLand = GetINIInt(L"Display", L"DistantLand", INT_MIN);
    if (distLand == INT_MIN) distLand = GetINIInt(L"LOD", L"bDisplayLODLand", 1);
    int distBuild = GetINIInt(L"Display", L"DistantBuildings", INT_MIN);
    if (distBuild == INT_MIN) distBuild = GetINIInt(L"LOD", L"bDisplayLODBuildings", 1);
    int distTrees = GetINIInt(L"Display", L"DistantTrees", INT_MIN);
    if (distTrees == INT_MIN) distTrees = GetINIInt(L"LOD", L"bDisplayLODTrees", 0);
    SendMessageW(gDistantLandCheck, BM_SETCHECK, distLand ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDistantBuildingsCheck, BM_SETCHECK, distBuild ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDistantTreesCheck, BM_SETCHECK, distTrees ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessageW(gBloomCheck, BM_SETCHECK, GetINIInt(L"Display", L"bDoBloom", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    if (gEffectsNoneRadio)
    {
        const BOOL noneEffects = (SendMessageW(gBloomCheck, BM_GETCHECK, 0, 0) != BST_CHECKED)
            && (SendMessageW(gHdrCheck, BM_GETCHECK, 0, 0) != BST_CHECKED);
        SendMessageW(gEffectsNoneRadio, BM_SETCHECK, noneEffects ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    SendMessageW(gShadowsOnGrassCheck, BM_SETCHECK, GetINIInt(L"Display", L"bShadowsOnGrass", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gTreeCanopyShadowsCheck, BM_SETCHECK, GetINIInt(L"Display", L"bDoCanopyShadowPass", 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    SetComboByData(gShadowFilterCombo, GetINIInt(L"Display", L"iShadowFilter", 2));

    {
        float f = (float)_wtof(GetINIString(L"Display", L"fSpecualrStartMax", L"500").c_str());
        int pct = (int)((f / 1000.0f) * 100.0f + 0.5f);
        if (pct < 0) pct = 0;
        if (pct > 100) pct = 100;
        SetTrackPos(gSpecularTrack, pct);
        SetTextInt(gSpecularVal, pct);
    }

    SetComboByData(gWaterDetailCombo, GetINIInt(L"Water", L"iWaterMult", 2));
    SendMessageW(gWaterReflectionsCheck, BM_SETCHECK, GetINIInt(L"Water", L"bUseWaterReflections", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterReflStaticsCheck, BM_SETCHECK, GetINIInt(L"Water", L"bUseWaterReflectionsStatics", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterReflTreesCheck, BM_SETCHECK, GetINIInt(L"Water", L"bUseWaterReflectionsTrees", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterRipplesCheck, BM_SETCHECK, GetINIInt(L"Water", L"bUseWaterDisplacements", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWindowReflectionsCheck, BM_SETCHECK, GetINIInt(L"Display", L"bDynamicWindowReflections", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterDepthCheck, BM_SETCHECK, GetINIInt(L"Water", L"bUseWaterDepth", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SetComboByData(gBloodDecalsCombo, GetINIInt(L"Decals", L"iMaxDecalsPerFrame", 10));

    SetEditInt(gGridDistantCountEdit, GetINIInt(L"TerrainManager", L"uGridDistantCount", 30));
    int treeRangeIni = GetINIInt(L"Trees", L"uGridDistantTreeRange", INT_MIN);
    if (treeRangeIni == INT_MIN) treeRangeIni = GetINIInt(L"TerrainManager", L"uGridDistantTreeRange", 30);
    SetEditInt(gGridDistantTreeRangeEdit, treeRangeIni);

    std::wstring tex = GetINIString(L"Advanced", L"TextureSize", L"");
    if (tex.empty())
    {
        int texSkip = GetINIInt(L"Display", L"iTexMipMapSkip", 0);
        if (texSkip <= 0) tex = L"Large";
        else if (texSkip == 1) tex = L"Medium";
        else tex = L"Small";
    }
    int tcount = (int)SendMessageW(gTexSizeCombo, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < tcount; i++)
    {
        wchar_t b[64] = {};
        SendMessageW(gTexSizeCombo, CB_GETLBTEXT, i, (LPARAM)b);
        if (tex == b) { SendMessageW(gTexSizeCombo, CB_SETCURSEL, i, 0); break; }
    }

    auto clamp01 = [](int v) { return (v < 0) ? 0 : (v > 100) ? 100 : v; };

    int tree = GetINIInt(L"Advanced", L"TreeFade", INT_MIN);
    if (tree == INT_MIN)
        tree = (int)(_wtof(GetINIString(L"LOD", L"fLODMultTrees", L"0.3333").c_str()) * 150.0 + 0.5);
    tree = clamp01(tree);

    int actor = GetINIInt(L"Advanced", L"ActorFade", INT_MIN);
    if (actor == INT_MIN)
        actor = (int)(_wtof(GetINIString(L"LOD", L"fLODMultActors", L"3.3333").c_str()) * 15.0 + 0.5);
    actor = clamp01(actor);

    int item = GetINIInt(L"Advanced", L"ItemFade", INT_MIN);
    if (item == INT_MIN)
        item = (int)(_wtof(GetINIString(L"LOD", L"fLODMultItems", L"3.3333").c_str()) * 15.0 + 0.5);
    item = clamp01(item);

    int obj = GetINIInt(L"Advanced", L"ObjectFade", INT_MIN);
    if (obj == INT_MIN)
        obj = (int)(_wtof(GetINIString(L"LOD", L"fLODMultObjects", L"3.3333").c_str()) * 15.0 + 0.5);
    obj = clamp01(obj);

    SetTrackPos(gTreeFadeTrack, tree);
    SetTrackPos(gActorFadeTrack, actor);
    SetTrackPos(gItemFadeTrack, item);
    SetTrackPos(gObjectFadeTrack, obj);
    SetTextInt(gTreeFadeVal, tree);
    SetTextInt(gActorFadeVal, actor);
    SetTextInt(gItemFadeVal, item);
    SetTextInt(gObjectFadeVal, obj);

    int mv = clamp01(GetINIInt(L"Audio", L"MasterVolume", 80));
    int ev = clamp01(GetINIInt(L"Audio", L"EffectsVolume", 80));
    int mus = clamp01(GetINIInt(L"Audio", L"MusicVolume", 70));

    SetTrackPos(gMasterVolTrack, mv);
    SetTrackPos(gEffectsVolTrack, ev);
    SetTrackPos(gMusicVolTrack, mus);
    SetTextInt(gMasterVolVal, mv);
    SetTextInt(gEffectsVolVal, ev);
    SetTextInt(gMusicVolVal, mus);

    int ms = GetINIInt(L"Controls", L"MouseSensitivity", INT_MIN);
    if (ms == INT_MIN)
    {
        double fms = _wtof(GetINIString(L"Controls", L"fMouseSensitivity", L"0.0020").c_str());
        ms = (int)(fms * 25000.0 + 0.5);
    }
    ms = clamp01(ms);
    SetTrackPos(gMouseSensTrack, ms);
    SetTextInt(gMouseSensVal, ms);

    int invertMouse = GetINIInt(L"Controls", L"InvertMouse", INT_MIN);
    if (invertMouse == INT_MIN) invertMouse = GetINIInt(L"Controls", L"bInvertYValues", 0);
    int alwaysRun = GetINIInt(L"Controls", L"AlwaysRun", INT_MIN);
    if (alwaysRun == INT_MIN) alwaysRun = GetINIInt(L"Controls", L"Always Run", 1);
    SendMessageW(gInvertMouseCheck, BM_SETCHECK, invertMouse ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gAlwaysRunCheck, BM_SETCHECK, alwaysRun ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gAutoSaveCheck, BM_SETCHECK, GetINIInt(L"Controls", L"AutoSave", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gUseEyeEnvMappingCheck, BM_SETCHECK, GetINIInt(L"General", L"bUseEyeEnvMapping", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gUseBlurShaderCheck, BM_SETCHECK, GetINIInt(L"BlurShader", L"bUseBlurShader", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gWaterHiResCheck, BM_SETCHECK, GetINIInt(L"Water", L"bUseWaterHiRes", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gActorSelfShadowingCheck, BM_SETCHECK, GetINIInt(L"Display", L"bActorSelfShadowing", 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    int jumpDelayTicks = (int)(_wtof(GetINIString(L"Controls", L"fJumpAnimDelay", L"0.2500").c_str()) * 1000.0);
    if (jumpDelayTicks < 0) jumpDelayTicks = 0;
    if (jumpDelayTicks > 1000) jumpDelayTicks = 1000;
    SetTrackPos(gJumpDelayTrack, jumpDelayTicks);
    SetTextFloat(gJumpDelayVal, jumpDelayTicks / 1000.0, 3);

    int treeBiasTicks = (int)((_wtof(GetINIString(L"SpeedTree", L"fLODTreeMipMapLODBias", L"-0.5000").c_str()) + 2.0) * 1000.0);
    if (treeBiasTicks < 0) treeBiasTicks = 0;
    if (treeBiasTicks > 4000) treeBiasTicks = 4000;
    SetTrackPos(gLodTreeBiasTrack, treeBiasTicks);
    SetTextFloat(gLodTreeBiasVal, (treeBiasTicks / 1000.0) - 2.0, 3);

    int localBiasTicks = (int)((_wtof(GetINIString(L"SpeedTree", L"fLocalTreeMipMapLODBias", L"0.0000").c_str()) + 2.0) * 1000.0);
    if (localBiasTicks < 0) localBiasTicks = 0;
    if (localBiasTicks > 4000) localBiasTicks = 4000;
    SetTrackPos(gLodLocalTreeBiasTrack, localBiasTicks);
    SetTextFloat(gLodLocalTreeBiasVal, (localBiasTicks / 1000.0) - 2.0, 3);

    std::wstring rw = GetINIString(L"Display", L"iSize W", L"");
    std::wstring rh = GetINIString(L"Display", L"iSize H", L"");
    if (!rw.empty() && !rh.empty())
    {
        std::wstring res = rw + L"x" + rh;
        int cnt = (int)SendMessageW(gResCombo, CB_GETCOUNT, 0, 0);
        int found = -1;
        for (int i = 0; i < cnt; i++)
        {
            wchar_t t[64] = {};
            SendMessageW(gResCombo, CB_GETLBTEXT, i, (LPARAM)t);
            if (_wcsicmp(t, res.c_str()) == 0) { found = i; break; }
        }
        if (found >= 0)
            SendMessageW(gResCombo, CB_SETCURSEL, found, 0);
    }

    int ber = GetINIInt(L"MAIN", L"bEnableBorderRegion", 1);
    SendMessageW(gDisableBorderRegionsCheck, BM_SETCHECK, (ber == 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gEnableConstructionSetButtonCheck, BM_SETCHECK,
        GetINIInt(L"Launcher", L"bEnableConstructionSetButton", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gCloseLauncherOnLaunchCheck, BM_SETCHECK,
        GetINIInt(L"Launcher", L"bCloseOnLaunch", 0) ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessageW(gDevAllowScreenshotCheck, BM_SETCHECK, GetINIInt(L"Display", L"bAllowScreenShot", 1) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDevFileLoggingCheck, BM_SETCHECK, GetINIInt(L"Messages", L"iFileLogging", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(gDevDebugTextCheck, BM_SETCHECK, GetINIInt(L"Display", L"iDebugText", 0) ? BST_CHECKED : BST_UNCHECKED, 0);
    SetEditInt(gDevFpsClampEdit, GetINIInt(L"General", L"iFPSClamp", 0));
    SetEditInt(gDevConsoleHistoryEdit, GetINIInt(L"Menu", L"iConsoleHistorySize", 100));
}

static std::map<std::wstring, std::wstring> BuildVideoPresetState(const std::wstring& presetName)
{
    std::map<std::wstring, std::wstring> kv;

    // Shared baseline
    kv[L"Display.bFull Screen"] = L"0";
    kv[L"Display.iPresentInterval"] = L"1";
    kv[L"Display.HDR"] = L"1";
    kv[L"Display.iMultiSample"] = L"0";
    kv[L"Display.iAdapter"] = L"0";
    kv[L"Display.DistantLand"] = L"1";
    kv[L"Display.DistantBuildings"] = L"1";
    kv[L"Display.DistantTrees"] = L"1";
    kv[L"Display.bDoBloom"] = L"0";
    kv[L"Display.bShadowsOnGrass"] = L"0";
    kv[L"Display.bDoCanopyShadowPass"] = L"0";
    kv[L"Display.iShadowFilter"] = L"2";
    kv[L"Display.fSpecualrStartMax"] = L"50";
    kv[L"Display.bDynamicWindowReflections"] = L"1";
    kv[L"Water.iWaterMult"] = L"3";
    kv[L"Water.bUseWaterReflections"] = L"1";
    kv[L"Water.bUseWaterReflectionsStatics"] = L"1";
    kv[L"Water.bUseWaterReflectionsTrees"] = L"1";
    kv[L"Water.bUseWaterDisplacements"] = L"1";
    kv[L"Water.bUseWaterDepth"] = L"0";
    kv[L"Water.bUseWaterHiRes"] = L"0";
    kv[L"Display.bActorSelfShadowing"] = L"0";
    kv[L"Decals.iMaxDecalsPerFrame"] = L"10";
    kv[L"TerrainManager.uGridDistantCount"] = L"30";
    kv[L"TerrainManager.uGridDistantTreeRange"] = L"30";
    kv[L"Trees.uGridDistantTreeRange"] = L"30";
    kv[L"Display.iTexMipMapSkip"] = L"0";
    kv[L"LOD.fLODMultTrees"] = L"0.6667";
    kv[L"LOD.fLODMultActors"] = L"6.6667";
    kv[L"LOD.fLODMultItems"] = L"6.6667";
    kv[L"LOD.fLODMultObjects"] = L"6.6667";
    kv[L"Controls.fJumpAnimDelay"] = L"0.2500";
    kv[L"SpeedTree.fLODTreeMipMapLODBias"] = L"-0.5000";
    kv[L"SpeedTree.fLocalTreeMipMapLODBias"] = L"0.0000";

    const std::wstring n = presetName;
    if (_wcsicmp(n.c_str(), L"Very Low") == 0)
    {
        kv[L"Display.iMultiSample"] = L"0";
        kv[L"Display.DistantTrees"] = L"0";
        kv[L"Display.HDR"] = L"0";
        kv[L"Display.bDoBloom"] = L"0";
        kv[L"Display.iShadowFilter"] = L"0";
        kv[L"Display.fSpecualrStartMax"] = L"15";
        kv[L"Water.iWaterMult"] = L"1";
        kv[L"Water.bUseWaterReflections"] = L"0";
        kv[L"Water.bUseWaterDisplacements"] = L"0";
        kv[L"Display.bDynamicWindowReflections"] = L"0";
        kv[L"Decals.iMaxDecalsPerFrame"] = L"2";
        kv[L"Display.iTexMipMapSkip"] = L"2";
        kv[L"LOD.fLODMultTrees"] = L"0.2333";
        kv[L"LOD.fLODMultActors"] = L"2.3333";
        kv[L"LOD.fLODMultItems"] = L"2.3333";
        kv[L"LOD.fLODMultObjects"] = L"2.3333";
    }
    else if (_wcsicmp(n.c_str(), L"Low") == 0)
    {
        kv[L"Display.DistantTrees"] = L"0";
        kv[L"Display.HDR"] = L"0";
        kv[L"Display.bDoBloom"] = L"1";
        kv[L"Display.iShadowFilter"] = L"1";
        kv[L"Display.fSpecualrStartMax"] = L"25";
        kv[L"Water.iWaterMult"] = L"1";
        kv[L"Water.bUseWaterReflections"] = L"0";
        kv[L"Water.bUseWaterDisplacements"] = L"0";
        kv[L"Display.bDynamicWindowReflections"] = L"0";
        kv[L"Decals.iMaxDecalsPerFrame"] = L"2";
        kv[L"Display.iTexMipMapSkip"] = L"2";
        kv[L"Water.bUseWaterDepth"] = L"0";
        kv[L"Water.bUseWaterHiRes"] = L"0";
        kv[L"TerrainManager.uGridDistantCount"] = L"14";
        kv[L"TerrainManager.uGridDistantTreeRange"] = L"14";
        kv[L"Trees.uGridDistantTreeRange"] = L"14";
        kv[L"LOD.fLODMultTrees"] = L"0.3333";
        kv[L"LOD.fLODMultActors"] = L"3.3333";
        kv[L"LOD.fLODMultItems"] = L"3.3333";
        kv[L"LOD.fLODMultObjects"] = L"3.3333";
    }
    else if (_wcsicmp(n.c_str(), L"Medium") == 0)
    {
        kv[L"Display.HDR"] = L"1";
        kv[L"Display.iMultiSample"] = L"2";
        kv[L"Display.iShadowFilter"] = L"1";
        kv[L"Display.fSpecualrStartMax"] = L"40";
        kv[L"Water.iWaterMult"] = L"2";
        kv[L"Display.bDynamicWindowReflections"] = L"0";
        kv[L"Decals.iMaxDecalsPerFrame"] = L"5";
        kv[L"Display.iTexMipMapSkip"] = L"1";
        kv[L"LOD.fLODMultTrees"] = L"0.4667";
        kv[L"LOD.fLODMultActors"] = L"4.6667";
        kv[L"LOD.fLODMultItems"] = L"4.6667";
        kv[L"LOD.fLODMultObjects"] = L"4.6667";
    }
    else if (_wcsicmp(n.c_str(), L"High") == 0)
    {
        kv[L"Display.iMultiSample"] = L"4";
        kv[L"Display.iShadowFilter"] = L"2";
        kv[L"Display.fSpecualrStartMax"] = L"50";
        kv[L"Water.iWaterMult"] = L"3";
        kv[L"Water.bUseWaterDepth"] = L"1";
        kv[L"Water.bUseWaterHiRes"] = L"1";
        kv[L"TerrainManager.uGridDistantCount"] = L"34";
        kv[L"TerrainManager.uGridDistantTreeRange"] = L"34";
        kv[L"Trees.uGridDistantTreeRange"] = L"34";
        kv[L"Decals.iMaxDecalsPerFrame"] = L"10";
        kv[L"Display.iTexMipMapSkip"] = L"0";
        kv[L"LOD.fLODMultTrees"] = L"0.5667";
        kv[L"LOD.fLODMultActors"] = L"5.6667";
        kv[L"LOD.fLODMultItems"] = L"5.6667";
        kv[L"LOD.fLODMultObjects"] = L"5.6667";
    }
    else // Ultra High
    {
        kv[L"Display.iMultiSample"] = L"8";
        kv[L"Display.bShadowsOnGrass"] = L"1";
        kv[L"Display.bDoCanopyShadowPass"] = L"1";
        kv[L"Display.iShadowFilter"] = L"3";
        kv[L"Display.fSpecualrStartMax"] = L"100";
        kv[L"Water.iWaterMult"] = L"3";
        kv[L"Water.bUseWaterDepth"] = L"1";
        kv[L"Water.bUseWaterHiRes"] = L"1";
        kv[L"Display.bActorSelfShadowing"] = L"1";
        kv[L"TerrainManager.uGridDistantCount"] = L"44";
        kv[L"TerrainManager.uGridDistantTreeRange"] = L"44";
        kv[L"Trees.uGridDistantTreeRange"] = L"44";
        kv[L"Display.bDynamicWindowReflections"] = L"1";
        kv[L"Decals.iMaxDecalsPerFrame"] = L"10";
        kv[L"Display.iTexMipMapSkip"] = L"0";
        kv[L"LOD.fLODMultTrees"] = L"0.6667";
        kv[L"LOD.fLODMultActors"] = L"6.6667";
        kv[L"LOD.fLODMultItems"] = L"6.6667";
        kv[L"LOD.fLODMultObjects"] = L"6.6667";
    }

    return kv;
}

static void ApplyPresetByName(const wchar_t* presetName)
{
    std::map<std::wstring, std::wstring> kv = BuildVideoPresetState(presetName);

    const std::wstring presetPath = PresetPathFromName(presetName);
    DWORD attr = GetFileAttributesW(presetPath.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
        auto fileKv = FilterPresetForApply(LoadPresetFile(presetPath));
        for (const auto& it : fileKv) kv[it.first] = it.second;
    }

    ApplyStateToControls(kv, false);
    gPendingPresetPath.clear();
}

// ----------------------------------------------------------------------------
// UI helpers
// ----------------------------------------------------------------------------
static HWND CreateGroupBox(HWND parent, const wchar_t* text, int x, int y, int w, int h)
{
    HWND g = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        x, y, w, h, parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(g);
    return g;
}

static HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h)
{
    HWND s = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
        x, y, w, h, parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(s);
    return s;
}

static HWND CreateCheckbox(HWND parent, const wchar_t* text, int id, int x, int y, int w, int h)
{
    HWND c = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(c);
    return c;
}

static HWND CreateCombo(HWND parent, int id, int x, int y, int w, int h)
{
    HWND c = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(c);
    return c;
}

static HWND CreateTrack(HWND parent, int id, int x, int y, int w, int h, int minV, int maxV)
{
    HWND t = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(t);
    SendMessageW(t, TBM_SETRANGE, TRUE, MAKELONG(minV, maxV));
    SendMessageW(t, TBM_SETTICFREQ, 10, 0);
    return t;
}

// ----------------------------------------------------------------------------
// Build UI
// ----------------------------------------------------------------------------
static void BuildUI(HWND hwnd)
{
    auto dpiForWindow = [](HWND h) -> int {
        HMODULE user32 = GetModuleHandleW(L"user32.dll");
        if (!user32) return 96;
        auto pGetDpiForWindow = (UINT(WINAPI*)(HWND))GetProcAddress(user32, "GetDpiForWindow");
        if (!pGetDpiForWindow) return 96;
        return (int)pGetDpiForWindow(h);
        };
    auto sx = [&](int v) { return MulDiv(v, dpiForWindow(hwnd), 96); };

    NONCLIENTMETRICSW ncm = {};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    wcscpy_s(ncm.lfMessageFont.lfFaceName, L"Segoe UI");
    ncm.lfMessageFont.lfHeight = -sx(11);
    gFont = CreateFontIndirectW(&ncm.lfMessageFont);

    const int pad = sx(10);
    const int headerH = sx(10);
    const int footerH = sx(56);

    RECT rc = {};
    GetClientRect(hwnd, &rc);
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;

    gScrollHost = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        0, headerH, w, (h - headerH - footerH), hwnd, (HMENU)(INT_PTR)2000, GetModuleHandleW(nullptr), nullptr);
    SetWindowSubclass(gScrollHost, ScrollHostSubclassProc, 1, 0);

    gContent = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE,
        0, 0, w, 10, gScrollHost, (HMENU)(INT_PTR)2001, GetModuleHandleW(nullptr), nullptr);
    SetWindowSubclass(gContent, ContentSubclassProc, 2, 0);

    SetCtrlFont(gScrollHost);
    SetCtrlFont(gContent);


    const int contentW = w - pad * 2 - sx(18);
    const int innerPad = sx(10);
    const int rowH = sx(22);
    const int rowGap = sx(6);
    const int labelH = sx(18);
    const int comboH = sx(320);

    const int labelW = sx(120);
    const int editW = sx(70);
    const int comboW = sx(190);

    const int laneGap = sx(20);

    const int LxLabel = pad + innerPad;
    const int LxCtrl = LxLabel + labelW + sx(10);

    const int RxLabel = pad + (contentW / 2) + (laneGap / 2);
    const int RxCtrl = RxLabel + labelW + sx(10);

    const int sliderW = sx(330);
    const int sliderValW = sx(40);
    const int sliderValGap = sx(10);

    auto AlignLabelY = [&](int y) { return y + sx(5); };

    auto RowLabelCombo = [&](int y, const wchar_t* label, HWND& outCombo, int id, bool rightLane)
        {
            const int xL = rightLane ? RxLabel : LxLabel;
            const int xC = rightLane ? RxCtrl : LxCtrl;
            CreateLabel(gContent, label, xL, AlignLabelY(y), labelW, labelH);
            outCombo = CreateCombo(gContent, id, xC, y, comboW, comboH);
        };

    auto RowLabelEdit = [&](int y, const wchar_t* label, HWND& outEdit, int id, bool rightLane)
        {
            const int xL = rightLane ? RxLabel : LxLabel;
            const int xC = rightLane ? RxCtrl : LxCtrl;
            CreateLabel(gContent, label, xL, AlignLabelY(y), labelW, labelH);
            outEdit = CreateEditBox(gContent, id, xC, y, editW, sx(22));
        };

    auto RowCheckbox = [&](int y, const wchar_t* text, HWND& outChk, int id, bool rightLane)
        {
            const int xC = rightLane ? RxLabel : LxLabel;
            outChk = CreateCheckbox(gContent, text, id, xC, AlignLabelY(y) - sx(2), sx(260), sx(20));
        };

    auto RowSlider = [&](int y, const wchar_t* label, HWND& outTrack, int id, HWND& outVal, const wchar_t* valText)
        {
            CreateLabel(gContent, label, LxLabel, AlignLabelY(y), labelW, labelH);
            outTrack = CreateTrack(gContent, id, LxCtrl, y - sx(2), sliderW, sx(26), 0, 100);
            outVal = CreateLabel(gContent, valText, LxCtrl + sliderW + sliderValGap, AlignLabelY(y), sliderValW, labelH);
        };

    int y = pad;

    // GRAPHICS ADAPTER AND RESOLUTION
    {
        const int gbH = sx(138);
        CreateGroupBox(gContent, L"Graphics Adapter and Resolution", pad, y, contentW, gbH);

        int ry = y + sx(28);
        CreateLabel(gContent, L"Adapter:", LxLabel, AlignLabelY(ry), labelW, labelH);
        gAdapterCombo = CreateCombo(gContent, IDC_ADAPTER_COMBO, LxCtrl, ry, contentW - (LxCtrl - pad) - sx(26), comboH);
        AddComboItem(gAdapterCombo, L"Default Adapter", 0);
        SendMessageW(gAdapterCombo, CB_SETCURSEL, 0, 0);

        ry += rowH + rowGap;
        CreateLabel(gContent, L"Resolution:", LxLabel, AlignLabelY(ry), labelW, labelH);
        gResCombo = CreateCombo(gContent, IDC_RES_COMBO, LxCtrl, ry, contentW - (LxCtrl - pad) - sx(26), comboH);

        ry += rowH + rowGap;
        CreateLabel(gContent, L"Antialiasing:", LxLabel, AlignLabelY(ry), labelW, labelH);
        gAaCombo = CreateCombo(gContent, IDC_AA_COMBO, LxCtrl, ry, contentW - (LxCtrl - pad) - sx(26), comboH);
        AddComboItem(gAaCombo, L"None (best performance)", 0);
        AddComboItem(gAaCombo, L"2x", 2);
        AddComboItem(gAaCombo, L"4x", 4);
        AddComboItem(gAaCombo, L"8x", 8);

        y += gbH + sx(12);
    }

    {
        const int gbY = y;
        const int gbH = sx(116);
        const int colGap = sx(12);
        const int colW = (contentW - (colGap * 2)) / 3;

        CreateGroupBox(gContent, L"Mode", pad, gbY, colW, gbH);
        CreateGroupBox(gContent, L"Screen Effects", pad + colW + colGap, gbY, colW, gbH);
        CreateGroupBox(gContent, L"Distant Rendering", pad + (colW + colGap) * 2, gbY, colW, gbH);

        const int row1 = gbY + sx(30);
        gWindowedRadio = CreateWindowExW(0, L"BUTTON", L"Windowed", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
            pad + innerPad, row1, sx(120), sx(20), gContent, (HMENU)(INT_PTR)IDC_MODE_WINDOWED, GetModuleHandleW(nullptr), nullptr);
        SetCtrlFont(gWindowedRadio);
        gFullscreenRadio = CreateWindowExW(0, L"BUTTON", L"Fullscreen", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            pad + innerPad, row1 + rowH, sx(120), sx(20), gContent, (HMENU)(INT_PTR)IDC_MODE_FULLSCREEN, GetModuleHandleW(nullptr), nullptr);
        SetCtrlFont(gFullscreenRadio);
        gVsyncCheck = CreateCheckbox(gContent, L"V. Sync", IDC_VSYNC, pad + innerPad, row1 + rowH * 2, sx(100), sx(20));

        const int effectsX = pad + colW + colGap + innerPad;
        gEffectsNoneRadio = CreateWindowExW(0, L"BUTTON", L"None", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
            effectsX, row1, sx(120), sx(20), gContent, (HMENU)(INT_PTR)IDC_EFFECTS_NONE, GetModuleHandleW(nullptr), nullptr);
        SetCtrlFont(gEffectsNoneRadio);
        gBloomCheck = CreateWindowExW(0, L"BUTTON", L"Bloom", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            effectsX, row1 + rowH, sx(120), sx(20), gContent, (HMENU)(INT_PTR)IDC_BLOOM, GetModuleHandleW(nullptr), nullptr);
        SetCtrlFont(gBloomCheck);
        gHdrCheck = CreateWindowExW(0, L"BUTTON", L"HDR", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            effectsX, row1 + rowH * 2, sx(120), sx(20), gContent, (HMENU)(INT_PTR)IDC_HDR, GetModuleHandleW(nullptr), nullptr);
        SetCtrlFont(gHdrCheck);

        const int distX = pad + (colW + colGap) * 2 + innerPad;
        gDistantLandCheck = CreateCheckbox(gContent, L"Distant Landscape", IDC_DIST_LAND, distX, row1, colW - innerPad, sx(20));
        gDistantBuildingsCheck = CreateCheckbox(gContent, L"Distant Buildings", IDC_DIST_BUILD, distX, row1 + rowH, colW - innerPad, sx(20));
        gDistantTreesCheck = CreateCheckbox(gContent, L"Distant Trees", IDC_DIST_TREES, distX, row1 + rowH * 2, colW - innerPad, sx(20));

        y += gbH + sx(12);
    }

    {
        const int gbH = sx(74);
        CreateGroupBox(gContent, L"Video Quality Presets", pad, y, contentW, gbH);
        const int by = y + sx(22);
        const int presetsGap = sx(6);
        const int defaultBtnW = sx(130);
        const int btnH = sx(28);

        gBtnDefaults = CreateWindowExW(0, L"BUTTON", L"Reset to Defaults", WS_CHILD | WS_VISIBLE,
            pad + sx(10), by, defaultBtnW, btnH, gContent, (HMENU)(INT_PTR)IDC_DEFAULTS, GetModuleHandleW(nullptr), nullptr);
        SetCtrlFont(gBtnDefaults);

        const int presetsStartX = pad + sx(10) + defaultBtnW + presetsGap;
        const int presetsRightPad = sx(10);
        const int presetsAreaW = (pad + contentW - presetsRightPad) - presetsStartX;
        const int presetCount = 5;
        const int presetBtnW = (presetsAreaW - (presetsGap * (presetCount - 1))) / presetCount;

        int px = presetsStartX;
        auto mkBtn=[&](HWND& out,const wchar_t* t,int id)
        {
            out = CreateWindowExW(0, L"BUTTON", t, WS_CHILD | WS_VISIBLE,
                px, by, presetBtnW, btnH, gContent, (HMENU)(INT_PTR)id, GetModuleHandleW(nullptr), nullptr);
            SetCtrlFont(out);
            px += presetBtnW + presetsGap;
        };

        mkBtn(gBtnPresetVeryLow, L"Very Low", IDC_PRESET_VERYLOW);
        mkBtn(gBtnPresetLow, L"Low", IDC_PRESET_LOW);
        mkBtn(gBtnPresetMedium, L"Medium", IDC_PRESET_MEDIUM);
        mkBtn(gBtnPresetHigh, L"High", IDC_PRESET_HIGH);
        mkBtn(gBtnPresetUltra, L"Ultra High", IDC_PRESET_ULTRA);

        y += gbH + sx(10);
    }

    // ADDITIONAL GRAPHICS
    {
        const int gbH = sx(236) + rowH + rowH + rowGap;
        CreateGroupBox(gContent, L"Additional Graphics", pad, y, contentW, gbH);

        int ry = y + sx(24);

        gShadowsOnGrassCheck = CreateCheckbox(gContent, L"Shadows on Grass", IDC_SHADOWS_GRASS, LxLabel, AlignLabelY(ry) - sx(2), sx(200), sx(20));
        gTreeCanopyShadowsCheck = CreateCheckbox(gContent, L"Tree Canopy Shadows", IDC_CANOPY_SHADOWS, LxLabel + sx(210), AlignLabelY(ry) - sx(2), sx(230), sx(20));

        ry += rowH + rowGap;

        CreateLabel(gContent, L"Shadow Filtering", LxLabel, AlignLabelY(ry), labelW, labelH);
        gShadowFilterCombo = CreateComboBox(gContent, IDC_SHADOW_FILTER, LxCtrl, ry, sx(140), sx(200));
        AddComboItemWithData(gShadowFilterCombo, L"Low", 0);
        AddComboItemWithData(gShadowFilterCombo, L"Medium", 1);
        AddComboItemWithData(gShadowFilterCombo, L"High", 2);
        AddComboItemWithData(gShadowFilterCombo, L"Ultra", 3);
        SetComboByData(gShadowFilterCombo, 2);

        CreateLabel(gContent, L"Specular Distance", RxLabel, AlignLabelY(ry), labelW, labelH);
        gSpecularTrack = CreateTrackBar(gContent, IDC_SPECULAR, RxCtrl, ry - sx(2), sx(140), sx(24), 0, 100);
        gSpecularVal = CreateValueLabel(gContent, L"50", RxCtrl + sx(150), AlignLabelY(ry), sx(40), labelH);
        SetTrackPos(gSpecularTrack, 50);

        ry += rowH + rowGap;

        CreateLabel(gContent, L"Water Detail", LxLabel, AlignLabelY(ry), labelW, labelH);
        gWaterDetailCombo = CreateComboBox(gContent, IDC_WATER_DETAIL, LxCtrl, ry, sx(140), sx(200));
        AddComboItemWithData(gWaterDetailCombo, L"Low", 1);
        AddComboItemWithData(gWaterDetailCombo, L"Medium", 2);
        AddComboItemWithData(gWaterDetailCombo, L"High", 3);
        SetComboByData(gWaterDetailCombo, 3);

        gWaterReflectionsCheck = CreateCheckbox(gContent, L"Water Reflections", IDC_WATER_REFLECT, RxLabel, AlignLabelY(ry) - sx(2), sx(200), sx(20));
        gWaterReflStaticsCheck = CreateCheckbox(gContent, L"Statics", IDC_WATER_REFLECT_STATICS, RxLabel + sx(210), AlignLabelY(ry) - sx(2), sx(90), sx(20));

        ry += rowH;
        gWaterReflTreesCheck = CreateCheckbox(gContent, L"Trees", IDC_WATER_REFLECT_TREES, RxLabel + sx(210), AlignLabelY(ry) - sx(2), sx(90), sx(20));

        ry += rowH + rowGap;
        gWaterDepthCheck = CreateCheckbox(gContent, L"Use Water Depth", IDC_WATER_DEPTH, LxLabel, AlignLabelY(ry) - sx(2), sx(200), sx(20));
        gWaterRipplesCheck = CreateCheckbox(gContent, L"Water Ripples", IDC_WATER_RIPPLES, RxLabel, AlignLabelY(ry) - sx(2), sx(200), sx(20));

        ry += rowH + rowGap;
        gWindowReflectionsCheck = CreateCheckbox(gContent, L"Window Reflections", IDC_WINDOW_REFLECT, RxLabel, AlignLabelY(ry) - sx(2), sx(220), sx(20));
        gWaterHiResCheck = CreateCheckbox(gContent, L"Hi-Res Water", IDC_WATER_HIRES, LxLabel, AlignLabelY(ry) - sx(2), sx(180), sx(20));

        ry += rowH + rowGap;
        gActorSelfShadowingCheck = CreateCheckbox(gContent, L"Actor Self Shadowing", IDC_ACTOR_SELF_SHADOW, RxLabel, AlignLabelY(ry) - sx(2), sx(220), sx(20));

        CreateLabel(gContent, L"Blood Decals", LxLabel, AlignLabelY(ry), labelW, labelH);
        gBloodDecalsCombo = CreateComboBox(gContent, IDC_BLOOD_DECALS, LxCtrl, ry, sx(140), sx(200));
        AddComboItemWithData(gBloodDecalsCombo, L"Off", 0);
        AddComboItemWithData(gBloodDecalsCombo, L"Low", 2);
        AddComboItemWithData(gBloodDecalsCombo, L"High", 10);
        SetComboByData(gBloodDecalsCombo, 10);

        ry += rowH + rowGap;
        RowLabelEdit(ry, L"uGridDistantCount", gGridDistantCountEdit, IDC_UGRID_DISTANT_COUNT, false);
        SetEditInt(gGridDistantCountEdit, 30);

        RowLabelEdit(ry, L"uGridDistantTreeRange", gGridDistantTreeRangeEdit, IDC_UGRID_DISTANT_TREE_RANGE, true);
        SetEditInt(gGridDistantTreeRangeEdit, 30);

        y += gbH + sx(12);
    }

    // ADVANCED
    {
        const int gbH = sx(340);
        CreateGroupBox(gContent, L"Advanced", pad, y, contentW, gbH);

        int ry = y + sx(24);

        RowLabelCombo(ry, L"Texture Size", gTexSizeCombo, IDC_TEXSIZE_COMBO, false);
        AddComboItem(gTexSizeCombo, L"Small", 0);
        AddComboItem(gTexSizeCombo, L"Medium", 0);
        AddComboItem(gTexSizeCombo, L"Large", 0);
        SendMessageW(gTexSizeCombo, CB_SETCURSEL, 2, 0);

        ry += rowH + rowGap + sx(6);
        RowSlider(ry, L"Tree Fade", gTreeFadeTrack, IDC_TREEFADE, gTreeFadeVal, L"50");
        ry += rowH + rowGap;
        RowSlider(ry, L"Actor Fade", gActorFadeTrack, IDC_ACTORFADE, gActorFadeVal, L"50");
        ry += rowH + rowGap;
        RowSlider(ry, L"Item Fade", gItemFadeTrack, IDC_ITEMFADE, gItemFadeVal, L"50");
        ry += rowH + rowGap;
        RowSlider(ry, L"Object Fade", gObjectFadeTrack, IDC_OBJECTFADE, gObjectFadeVal, L"50");

        ry += rowH + rowGap + sx(4);
        CreateLabel(gContent, L"Jump Delay", LxLabel, AlignLabelY(ry), labelW, labelH);
        gJumpDelayTrack = CreateTrackBar(gContent, IDC_JUMP_DELAY, LxCtrl, ry - sx(2), sliderW, sx(24), 0, 1000);
        gJumpDelayVal = CreateValueLabel(gContent, L"0.250", LxCtrl + sliderW + sliderValGap, AlignLabelY(ry), sliderValW + sx(20), labelH);
        SetTrackPos(gJumpDelayTrack, 250);

        ry += rowH + rowGap;
        CreateLabel(gContent, L"LOD Tree Bias", LxLabel, AlignLabelY(ry), labelW, labelH);
        gLodTreeBiasTrack = CreateTrackBar(gContent, IDC_LOD_TREE_BIAS, LxCtrl, ry - sx(2), sliderW, sx(24), 0, 4000);
        gLodTreeBiasVal = CreateValueLabel(gContent, L"-0.500", LxCtrl + sliderW + sliderValGap, AlignLabelY(ry), sliderValW + sx(20), labelH);
        SetTrackPos(gLodTreeBiasTrack, 1500);

        ry += rowH + rowGap;
        CreateLabel(gContent, L"LOD Local Bias", LxLabel, AlignLabelY(ry), labelW, labelH);
        gLodLocalTreeBiasTrack = CreateTrackBar(gContent, IDC_LOCAL_TREE_BIAS, LxCtrl, ry - sx(2), sliderW, sx(24), 0, 4000);
        gLodLocalTreeBiasVal = CreateValueLabel(gContent, L"0.000", LxCtrl + sliderW + sliderValGap, AlignLabelY(ry), sliderValW + sx(20), labelH);
        SetTrackPos(gLodLocalTreeBiasTrack, 2000);

        y += gbH + sx(12);
    }

    // AUDIO
    {
        const int gbH = sx(150);
        CreateGroupBox(gContent, L"Audio", pad, y, contentW, gbH);

        int ry = y + sx(28);
        RowSlider(ry, L"Master", gMasterVolTrack, IDC_MASTERVOL, gMasterVolVal, L"80");
        ry += rowH + rowGap;
        RowSlider(ry, L"Effects", gEffectsVolTrack, IDC_EFFECTSVOL, gEffectsVolVal, L"80");
        ry += rowH + rowGap;
        RowSlider(ry, L"Music", gMusicVolTrack, IDC_MUSICVOL, gMusicVolVal, L"70");

        y += gbH + sx(12);
    }

    // CONTROLS
    {
        const int gbH = sx(115);
        CreateGroupBox(gContent, L"Controls", pad, y, contentW, gbH);

        int ry = y + sx(28);
        CreateLabel(gContent, L"Mouse Sensitivity", LxLabel, AlignLabelY(ry), labelW, labelH);
        gMouseSensTrack = CreateTrack(gContent, IDC_MOUSESENS, LxCtrl, ry - sx(2), sliderW, sx(26), 1, 100);
        gMouseSensVal = CreateLabel(gContent, L"50", LxCtrl + sliderW + sliderValGap, AlignLabelY(ry), sliderValW, labelH);

        ry += rowH + rowGap;
        gInvertMouseCheck = CreateCheckbox(gContent, L"Invert Mouse", IDC_INVERTMOUSE, LxLabel, AlignLabelY(ry) - sx(2), sx(180), sx(20));
        gAlwaysRunCheck = CreateCheckbox(gContent, L"Always Run", IDC_ALWAYSRUN, LxLabel + sx(200), AlignLabelY(ry) - sx(2), sx(160), sx(20));
        gAutoSaveCheck = CreateCheckbox(gContent, L"Auto Save", IDC_AUTOSAVE, LxLabel + sx(380), AlignLabelY(ry) - sx(2), sx(160), sx(20));

        y += gbH + sx(12);
    }

    // GAMEPLAY
    {
        const int gbH = sx(174);
        CreateGroupBox(gContent, L"Gameplay", pad, y, contentW, gbH);

        int ry = y + sx(28);
        gDisableBorderRegionsCheck = CreateCheckbox(gContent, L"Disable Border Regions", IDC_DISABLE_BORDER, LxLabel, AlignLabelY(ry) - sx(2), sx(260), sx(20));
        gUseEyeEnvMappingCheck = CreateCheckbox(gContent, L"Use Eye Env Mapping", IDC_EYE_ENVMAP, LxLabel + sx(280), AlignLabelY(ry) - sx(2), sx(220), sx(20));

        ry += rowH + rowGap;
        gUseBlurShaderCheck = CreateCheckbox(gContent, L"Use Blur Shader", IDC_BLUR_SHADER, LxLabel, AlignLabelY(ry) - sx(2), sx(220), sx(20));

        ry += rowH + rowGap;
        gEnableConstructionSetButtonCheck = CreateCheckbox(gContent, L"Enable Construction Set button?", IDC_ENABLE_CS_BUTTON,
            LxLabel, AlignLabelY(ry) - sx(2), sx(320), sx(20));

        ry += rowH + rowGap;
        gCloseLauncherOnLaunchCheck = CreateCheckbox(gContent,
            L"Close launcher after Play / Launch CS/CSE",
            IDC_CLOSE_ON_LAUNCH,
            LxLabel, AlignLabelY(ry) - sx(2), sx(420), sx(20));

        y += gbH + sx(12);
    }

    // DEVELOPER
    {
        const int gbH = sx(150);
        CreateGroupBox(gContent, L"Developer", pad, y, contentW, gbH);

        int ry = y + sx(28);

        gDevAllowScreenshotCheck = CreateCheckbox(gContent, L"Allow Screenshots", IDC_DEV_ALLOWSCREENSHOT, LxLabel, AlignLabelY(ry) - sx(2), sx(180), sx(20));
        gDevFileLoggingCheck = CreateCheckbox(gContent, L"Enable File Logging", IDC_DEV_FILELOGGING, LxLabel + sx(200), AlignLabelY(ry) - sx(2), sx(200), sx(20));
        gDevDebugTextCheck = CreateCheckbox(gContent, L"Debug Text", IDC_DEV_DEBUGTEXT, LxLabel + sx(420), AlignLabelY(ry) - sx(2), sx(140), sx(20));

        ry += rowH + rowGap;

        RowLabelEdit(ry, L"FPS Clamp", gDevFpsClampEdit, IDC_DEV_FPSCLAMP, false);
        RowLabelEdit(ry, L"Console History", gDevConsoleHistoryEdit, IDC_DEV_CONSOLEHIST, true);

        y += gbH + sx(12);
    }

    gContentHeight = y;
    SetWindowPos(gContent, nullptr, 0, 0, w, gContentHeight, SWP_NOZORDER | SWP_NOACTIVATE);

    int btnY = h - footerH + sx(18);

    gBtnCancel = CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        w - pad - sx(164), btnY, sx(100), sx(28), hwnd, (HMENU)(INT_PTR)IDC_CANCEL, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(gBtnCancel);

    gBtnApply = CreateWindowExW(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE,
        w - pad - sx(56), btnY, sx(56), sx(28), hwnd, (HMENU)(INT_PTR)IDC_APPLY, GetModuleHandleW(nullptr), nullptr);
    SetCtrlFont(gBtnApply);

    PopulatePresetCombo();
    LoadFromINI();

    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = (gContentHeight > 0) ? (gContentHeight - 1) : 0;

    RECT src = {};
    GetClientRect(gScrollHost, &src);
    si.nPage = (UINT)(src.bottom - src.top);
    si.nPos = 0;
    SetScrollInfo(gScrollHost, SB_VERT, &si, TRUE);
}

// ----------------------------------------------------------------------------
// Track -> label sync
// ----------------------------------------------------------------------------
static void SyncValueLabelsFromTracks()
{
    if (!gWnd) return;

    if (gTreeFadeTrack)  SetTextInt(gTreeFadeVal, GetTrackPos(gTreeFadeTrack, 50));
    if (gActorFadeTrack) SetTextInt(gActorFadeVal, GetTrackPos(gActorFadeTrack, 50));
    if (gItemFadeTrack)  SetTextInt(gItemFadeVal, GetTrackPos(gItemFadeTrack, 50));
    if (gObjectFadeTrack)SetTextInt(gObjectFadeVal, GetTrackPos(gObjectFadeTrack, 50));
    if (gSpecularTrack)  SetTextInt(gSpecularVal, GetTrackPos(gSpecularTrack, 50));

    if (gMasterVolTrack) SetTextInt(gMasterVolVal, GetTrackPos(gMasterVolTrack, 80));
    if (gEffectsVolTrack)SetTextInt(gEffectsVolVal, GetTrackPos(gEffectsVolTrack, 80));
    if (gMusicVolTrack)  SetTextInt(gMusicVolVal, GetTrackPos(gMusicVolTrack, 70));

    if (gMouseSensTrack) SetTextInt(gMouseSensVal, GetTrackPos(gMouseSensTrack, 50));

    if (gJumpDelayTrack) SetTextFloat(gJumpDelayVal, GetTrackPos(gJumpDelayTrack, 250) / 1000.0, 3);
    if (gLodTreeBiasTrack) SetTextFloat(gLodTreeBiasVal, (GetTrackPos(gLodTreeBiasTrack, 1500) / 1000.0) - 2.0, 3);
    if (gLodLocalTreeBiasTrack) SetTextFloat(gLodLocalTreeBiasVal, (GetTrackPos(gLodLocalTreeBiasTrack, 2000) / 1000.0) - 2.0, 3);
}

// ----------------------------------------------------------------------------
// Window proc
// ----------------------------------------------------------------------------
static LRESULT CALLBACK OptionsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        gWnd = hwnd;
        BuildUI(hwnd);
        return 0;

    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE)
            LoadFromINI();
        return 0;

    case WM_HSCROLL:
    case WM_VSCROLL:
        if ((HWND)lParam && (HWND)lParam != gScrollHost)
        {
            SyncValueLabelsFromTracks();
            return 0;
        }
        if ((HWND)lParam == gScrollHost || lParam == 0)
        {
            SCROLLINFO si = {};
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            GetScrollInfo(gScrollHost, SB_VERT, &si);

            int pos = si.nPos;
            switch (LOWORD(wParam))
            {
            case SB_TOP: pos = si.nMin; break;
            case SB_BOTTOM: pos = si.nMax; break;
            case SB_LINEUP: pos -= 24; break;
            case SB_LINEDOWN: pos += 24; break;
            case SB_PAGEUP: pos -= (int)si.nPage; break;
            case SB_PAGEDOWN: pos += (int)si.nPage; break;
            case SB_THUMBTRACK: pos = si.nTrackPos; break;
            default: break;
            }

            int maxPos = (int)si.nMax - (int)si.nPage + 1;
            if (maxPos < 0) maxPos = 0;
            if (pos < si.nMin) pos = si.nMin;
            if (pos > maxPos) pos = maxPos;

            si.fMask = SIF_POS;
            si.nPos = pos;
            SetScrollInfo(gScrollHost, SB_VERT, &si, TRUE);

            gScrollPos = pos;
            SetWindowPos(gContent, nullptr, 0, -gScrollPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }
        return 0;

    case WM_MOUSEWHEEL:
        if (gScrollHost)
        {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int steps = delta / WHEEL_DELTA;
            UINT code = (steps < 0) ? SB_LINEDOWN : SB_LINEUP;
            int n = std::abs(steps) * 3;
            if (n < 1) n = 1;
            for (int i = 0; i < n; ++i)
                SendMessageW(hwnd, WM_VSCROLL, MAKEWPARAM(code, 0), (LPARAM)gScrollHost);
            return 0;
        }
        break;

    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        SetBkMode(hdc, TRANSPARENT);
        return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
    }

    case WM_CTLCOLOREDIT:
    {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
        return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
    }

    case WM_SIZE:
        if (gScrollHost)
        {
            RECT rc = {};
            GetClientRect(hwnd, &rc);
            int w = rc.right - rc.left;
            int h = rc.bottom - rc.top;

            int headerH = 10;
            int footerH = 56;

            int dpi = 96;
            HMODULE user32 = GetModuleHandleW(L"user32.dll");
            if (user32)
            {
                auto pGetDpiForWindow = (UINT(WINAPI*)(HWND))GetProcAddress(user32, "GetDpiForWindow");
                if (pGetDpiForWindow) dpi = (int)pGetDpiForWindow(hwnd);
            }

            headerH = MulDiv(headerH, dpi, 96);
            footerH = MulDiv(footerH, dpi, 96);

            SetWindowPos(gScrollHost, nullptr, 0, headerH, w, (h - headerH - footerH), SWP_NOZORDER | SWP_NOACTIVATE);

            SCROLLINFO si = {};
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMin = 0;
            si.nMax = (gContentHeight > 0) ? (gContentHeight - 1) : 0;

            RECT src = {};
            GetClientRect(gScrollHost, &src);
            si.nPage = (UINT)(src.bottom - src.top);
            si.nPos = gScrollPos;
            SetScrollInfo(gScrollHost, SB_VERT, &si, TRUE);

            int maxPos = (int)si.nMax - (int)si.nPage + 1;
            if (maxPos < 0) maxPos = 0;
            if (gScrollPos > maxPos) gScrollPos = maxPos;
            SetWindowPos(gContent, nullptr, 0, -gScrollPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

            const int pad = MulDiv(10, dpi, 96);
            const int btnY = h - footerH + MulDiv(18, dpi, 96);

            if (gBtnCancel)          SetWindowPos(gBtnCancel, nullptr, w - pad - MulDiv(164, dpi, 96), btnY, MulDiv(100, dpi, 96), MulDiv(28, dpi, 96), SWP_NOZORDER | SWP_NOACTIVATE);
            if (gBtnApply)           SetWindowPos(gBtnApply, nullptr, w - pad - MulDiv(56, dpi, 96), btnY, MulDiv(56, dpi, 96), MulDiv(28, dpi, 96), SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if ((id == IDC_PRESET_VERYLOW || id == IDC_PRESET_LOW || id == IDC_PRESET_MEDIUM || id == IDC_PRESET_HIGH || id == IDC_PRESET_ULTRA) && code == BN_CLICKED)
        {
            if (id == IDC_PRESET_VERYLOW) ApplyPresetByName(L"Very Low");
            else if (id == IDC_PRESET_LOW) ApplyPresetByName(L"Low");
            else if (id == IDC_PRESET_MEDIUM) ApplyPresetByName(L"Medium");
            else if (id == IDC_PRESET_HIGH) ApplyPresetByName(L"High");
            else if (id == IDC_PRESET_ULTRA) ApplyPresetByName(L"Ultra High");
            return 0;
        }

        if (id == IDC_DEFAULTS && code == BN_CLICKED)
        {
            ResetINIFile();
            gPendingPresetPath.clear();
            LoadFromINI();
            return 0;
        }

        if (id == IDC_EFFECTS_NONE && code == BN_CLICKED)
        {
            SendMessageW(gBloomCheck, BM_SETCHECK, BST_UNCHECKED, 0);
            SendMessageW(gHdrCheck, BM_SETCHECK, BST_UNCHECKED, 0);
            return 0;
        }
        if ((id == IDC_BLOOM || id == IDC_HDR) && code == BN_CLICKED)
        {
            if (gEffectsNoneRadio) SendMessageW(gEffectsNoneRadio, BM_SETCHECK, BST_UNCHECKED, 0);
            return 0;
        }
        if (id == IDC_CANCEL && code == BN_CLICKED)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        if (id == IDC_APPLY && code == BN_CLICKED)
        {
            ApplyControlsToINI();
            LoadFromINI();
            DestroyWindow(hwnd);
            return 0;
        }
        return 0;
    }

    case WM_DESTROY:
        if (gScrollHost)
            RemoveWindowSubclass(gScrollHost, ScrollHostSubclassProc, 1);
        if (gContent)
            RemoveWindowSubclass(gContent, ContentSubclassProc, 2);

        if (gFont) { DeleteObject(gFont); gFont = nullptr; }

        gWnd = nullptr;

        gTitleText = nullptr;
        gPresetLabel = nullptr;

        gPresetCombo = nullptr;
        gScrollHost = nullptr;
        gContent = nullptr;

        gScrollPos = 0;
        gContentHeight = 0;

        gPendingPresetPath.clear();
        gPresetPaths.clear();
        return 0;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ----------------------------------------------------------------------------
// Register + public entry
// ----------------------------------------------------------------------------
static ATOM RegisterOptionsClass(HINSTANCE hInst)
{
    WNDCLASSW wc = {};
    wc.lpfnWndProc = OptionsProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"MOL_Options";
    return RegisterClassW(&wc);
}

void OpenOptionsWindow(HWND hParent)
{
    InitializeINI();

    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    HINSTANCE hInst = GetModuleHandleW(nullptr);
    static ATOM atom = 0;
    if (!atom) atom = RegisterOptionsClass(hInst);

    const int w = 720;
    const int h = 720;

    RECT pr = {};
    if (hParent && GetWindowRect(hParent, &pr))
    {
        int px = pr.left + ((pr.right - pr.left) - w) / 2;
        int py = pr.top + ((pr.bottom - pr.top) - h) / 2;
        gWnd = CreateWindowExW(WS_EX_DLGMODALFRAME, L"MOL_Options", L"Options",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            px, py, w, h,
            hParent, nullptr, hInst, nullptr);
    }
    else
    {
        gWnd = CreateWindowExW(WS_EX_DLGMODALFRAME, L"MOL_Options", L"Options",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, w, h,
            hParent, nullptr, hInst, nullptr);
    }

    if (!gWnd) return;

    ShowWindow(gWnd, SW_SHOW);
    UpdateWindow(gWnd);
}
