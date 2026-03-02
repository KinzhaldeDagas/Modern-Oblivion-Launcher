#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>       // <-- Add this
#include "DataFilesWindow.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include <winreg.h>



#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

#define IDC_PLUGIN_LIST 1001
#define IDC_OK_BUTTON 1002
#define IDC_CANCEL_BUTTON 1003

static std::vector<std::wstring> pluginFiles;
static std::vector<std::wstring> activePlugins;
static HWND hPluginList;
static std::wstring dataPath;

static HFONT g_hCommonFont = NULL;
static std::wstring g_fontFaceName;
static std::wstring g_fontFilePath;

static std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == L'\\') return a + b;
    return a + L"\\" + b;
}

static std::wstring GetExeDir() {
    wchar_t buf[MAX_PATH] = {};
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    PathRemoveFileSpecW(buf);
    return buf;
}

static std::wstring ParentDir(std::wstring p) {
    if (p.empty()) return p;
    wchar_t tmp[MAX_PATH] = {};
    wcsncpy_s(tmp, p.c_str(), _TRUNCATE);
    PathRemoveFileSpecW(tmp);
    return tmp;
}

static std::wstring FindFontFileNearSolutionRoot() {
    const std::wstring fontName = L"oblivionfont.ttf";
    const std::wstring slnName = L"Modern Oblivion Launcher.sln";

    std::vector<std::wstring> roots;
    {
        wchar_t cwd[MAX_PATH] = {};
        GetCurrentDirectoryW(MAX_PATH, cwd);
        roots.push_back(cwd);
    }
    roots.push_back(GetExeDir());

    for (const auto& start : roots) {
        std::wstring dir = start;
        for (int i = 0; i < 6 && !dir.empty(); i++) {
            const std::wstring fontPath = JoinPath(dir, fontName);
            if (PathFileExistsW(fontPath.c_str())) return fontPath;

            const std::wstring slnPath = JoinPath(dir, slnName);
            if (PathFileExistsW(slnPath.c_str())) {
                const std::wstring slnFont = JoinPath(dir, fontName);
                if (PathFileExistsW(slnFont.c_str())) return slnFont;
            }
            dir = ParentDir(dir);
        }
    }
    return L"";
}

static uint16_t ReadU16BE(const uint8_t* p) { return (uint16_t)((p[0] << 8) | p[1]); }
static uint32_t ReadU32BE(const uint8_t* p) { return (uint32_t)((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]); }

static std::wstring ExtractFamilyNameFromTTF(const std::wstring& ttfPath) {
    std::ifstream f(ttfPath, std::ios::binary);
    if (!f) return L"";
    f.seekg(0, std::ios::end);
    const std::streamoff sz = f.tellg();
    if (sz <= 0 || sz > (50ll * 1024ll * 1024ll)) return L"";
    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes((size_t)sz);
    f.read((char*)bytes.data(), sz);
    if (!f || bytes.size() < 12) return L"";

    const uint16_t numTables = ReadU16BE(&bytes[4]);
    if (bytes.size() < 12ull + (size_t)numTables * 16ull) return L"";

    uint32_t nameOffset = 0, nameLength = 0;
    for (uint16_t i = 0; i < numTables; i++) {
        const size_t off = 12ull + (size_t)i * 16ull;
        const uint32_t tag = ReadU32BE(&bytes[off + 0]);
        const uint32_t to = ReadU32BE(&bytes[off + 8]);
        const uint32_t tl = ReadU32BE(&bytes[off + 12]);
        if (tag == 0x6E616D65u) { nameOffset = to; nameLength = tl; break; } // 'name'
    }
    if (!nameOffset || nameOffset + nameLength > bytes.size() || nameLength < 6) return L"";

    const uint8_t* nt = bytes.data() + nameOffset;
    const uint16_t count = ReadU16BE(nt + 2);
    const uint16_t stringOffset = ReadU16BE(nt + 4);
    if (6ull + (size_t)count * 12ull > nameLength) return L"";

    auto pick = [&](uint16_t wantedNameId) -> std::wstring {
        for (uint16_t i = 0; i < count; i++) {
            const uint8_t* r = nt + 6 + (size_t)i * 12;
            const uint16_t platformID = ReadU16BE(r + 0);
            const uint16_t encodingID = ReadU16BE(r + 2);
            const uint16_t languageID = ReadU16BE(r + 4);
            const uint16_t nameID = ReadU16BE(r + 6);
            const uint16_t len = ReadU16BE(r + 8);
            const uint16_t roff = ReadU16BE(r + 10);

            if (platformID != 3) continue;
            if (!(encodingID == 0 || encodingID == 1 || encodingID == 10)) continue;
            if (languageID != 0x0409) continue;
            if (nameID != wantedNameId) continue;

            const size_t sbase = nameOffset + (size_t)stringOffset + (size_t)roff;
            if (sbase + len > bytes.size()) continue;

            std::wstring out;
            out.reserve(len / 2);
            for (size_t j = 0; j + 1 < len; j += 2) {
                const uint8_t b0 = bytes[sbase + j];
                const uint8_t b1 = bytes[sbase + j + 1];
                const wchar_t wc = (wchar_t)((b0 << 8) | b1);
                if (wc) out.push_back(wc);
            }
            return out;
        }
        return L"";
        };

    std::wstring fam = pick(1);
    if (!fam.empty()) return fam;
    return pick(4);
}

static HFONT LoadOblivionFontFromFile(float pt, bool bold) {
    g_fontFilePath = FindFontFileNearSolutionRoot();
    if (g_fontFilePath.empty()) return NULL;

    g_fontFaceName = ExtractFamilyNameFromTTF(g_fontFilePath);
    if (g_fontFaceName.empty()) g_fontFaceName = L"Oblivion";

    // (disabled) AddFontResourceExW(g_fontFilePath.c_str(), FR_PRIVATE, NULL);
    HDC hdc = GetDC(NULL);
    const int logpx = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);
    const int height = -MulDiv((int)(pt + 0.5f), logpx, 72);

    LOGFONTW lf = {};
    lf.lfHeight = height;
    lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    wcsncpy_s(lf.lfFaceName, g_fontFaceName.c_str(), _TRUNCATE);
    return CreateFontIndirectW(&lf);
}

std::wstring GetOblivionDataPath() {
    HKEY hKey;
    wchar_t path[MAX_PATH];
    DWORD size = sizeof(path);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Bethesda Softworks\\Oblivion", 0, KEY_READ | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, L"Installed Path", NULL, NULL, (LPBYTE)path, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::wstring(path) + L"Data\\";
        }
        RegCloseKey(hKey);
    }
    return L"";
}

void LoadPlugins() {
    pluginFiles.clear();

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile((dataPath + L"*.es?").c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            pluginFiles.push_back(findData.cFileName);
        } while (FindNextFile(hFind, &findData));
        FindClose(hFind);
    }
}

void LoadActivePlugins() {
    activePlugins.clear();
    wchar_t appData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appData))) {
        std::wstring pluginsTxt = std::wstring(appData) + L"\\Oblivion\\Plugins.txt";
        std::wifstream inFile(pluginsTxt);
        if (inFile) {
            std::wstring line;
            while (std::getline(inFile, line)) {
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
                if (!line.empty()) {
                    activePlugins.push_back(line);
                }
            }
        }
    }
}

void SaveActivePlugins() {
    wchar_t appData[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appData))) {
        std::wstring pluginsTxt = std::wstring(appData) + L"\\Oblivion\\Plugins.txt";
        std::wofstream outFile(pluginsTxt, std::ios::trunc);
        if (outFile) {
            int count = ListView_GetItemCount(hPluginList);
            for (int i = 0; i < count; ++i) {
                if (ListView_GetCheckState(hPluginList, i)) {
                    wchar_t buffer[MAX_PATH];
                    ListView_GetItemText(hPluginList, i, 0, buffer, MAX_PATH);
                    outFile << buffer << std::endl;
                }
            }
        }
    }
}

void PopulatePluginList(HWND hwndList) {
    ListView_DeleteAllItems(hwndList);

    for (size_t i = 0; i < pluginFiles.size(); ++i) {
        LVITEM item = {};
        item.mask = LVIF_TEXT;
        item.iItem = (int)i;
        item.pszText = (LPWSTR)pluginFiles[i].c_str();
        ListView_InsertItem(hwndList, &item);

        if (std::find(activePlugins.begin(), activePlugins.end(), pluginFiles[i]) != activePlugins.end()) {
            ListView_SetCheckState(hwndList, (int)i, TRUE);
        }
    }
}

LRESULT CALLBACK DataFilesWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
    {
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icex);

        hPluginList = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
            WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER,
            10, 10, 460, 400,
            hwnd, (HMENU)IDC_PLUGIN_LIST, NULL, NULL);

        g_hCommonFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (!g_hCommonFont)
            g_hCommonFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        if (hPluginList)
            SendMessage(hPluginList, WM_SETFONT, (WPARAM)g_hCommonFont, TRUE);

        ListView_SetExtendedListViewStyle(hPluginList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

        LVCOLUMN col = { LVCF_WIDTH, 0, 440 };
        ListView_InsertColumn(hPluginList, 0, &col);

        HWND hOk = CreateWindow(L"BUTTON", L"OK", WS_VISIBLE | WS_CHILD,
            200, 420, 80, 30,
            hwnd, (HMENU)IDC_OK_BUTTON, NULL, NULL);

        HWND hCancel = CreateWindow(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            290, 420, 80, 30,
            hwnd, (HMENU)IDC_CANCEL_BUTTON, NULL, NULL);

        if (hOk) SendMessage(hOk, WM_SETFONT, (WPARAM)g_hCommonFont, TRUE);
        if (hCancel) SendMessage(hCancel, WM_SETFONT, (WPARAM)g_hCommonFont, TRUE);

        PopulatePluginList(hPluginList);
        return 0;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OK_BUTTON:
            SaveActivePlugins();
            DestroyWindow(hwnd);
            return 0;
        case IDC_CANCEL_BUTTON:
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_DESTROY:
        if (g_hCommonFont && g_hCommonFont != (HFONT)GetStockObject(DEFAULT_GUI_FONT)) {
            DeleteObject(g_hCommonFont);
            g_hCommonFont = NULL;
        }
        if (!g_fontFilePath.empty()) {
            g_fontFilePath.clear();
            g_fontFaceName.clear();
        }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void OpenDataFilesWindow(HWND hParent) {
    dataPath = GetOblivionDataPath();
    if (dataPath.empty()) {
        MessageBox(hParent, L"Oblivion install path not found.", L"Error", MB_ICONERROR);
        return;
    }

    LoadPlugins();
    LoadActivePlugins();

    WNDCLASS wc = {};
    wc.lpfnWndProc = DataFilesWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"DataFilesWindow";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"DataFilesWindow", L"Data Files",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        hParent, NULL, GetModuleHandle(NULL), NULL);

    ShowWindow(hwnd, SW_SHOW);
}
