#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "DataFilesWindow.h"

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <cstdint>
#include <cwctype>
#include <fstream>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

#define IDC_PLUGIN_LIST      1001
#define IDC_OK_BUTTON        1002
#define IDC_CANCEL_BUTTON    1003
#define IDC_RESET_BUTTON     1004
#define IDC_SORT_BUTTON      1005
#define IDC_DESCRIPTION_EDIT 1006
#define IDT_HALL_OF_FAME   2001

void PopulatePluginList(HWND hwndList);

namespace {

static std::vector<std::wstring> pluginFiles;
static std::vector<std::wstring> activePlugins;

static HWND hPluginList = NULL;
static HWND hPluginName = NULL;
static HWND hHallOfFame = NULL;
static HWND hCreatedBy = NULL;
static HWND hDescription = NULL;
static HWND hCreatedOn = NULL;
static HWND hModifiedOn = NULL;

static std::wstring dataPath;
static HFONT g_hCommonFont = NULL;
static HFONT g_hTitleFont = NULL;
static HFONT g_hMetaFont = NULL;

static const std::vector<std::wstring> kHallOfFame = {
    L"Daggers", L"Alenet", L"llde", L"ponyrider0", L"gbr", L"shademe"
};
static int gHallOfFameIndex = -1;
static uint32_t gHallOfFameRngState = 0;

static uint32_t NextRandomU32() {
    if (gHallOfFameRngState == 0) {
        gHallOfFameRngState = (uint32_t)GetTickCount() ^ (uint32_t)GetCurrentProcessId() ^ 0xA341316Cu;
    }
    gHallOfFameRngState ^= gHallOfFameRngState << 13;
    gHallOfFameRngState ^= gHallOfFameRngState >> 17;
    gHallOfFameRngState ^= gHallOfFameRngState << 5;
    return gHallOfFameRngState;
}

static void UpdateHallOfFameTitle(HWND hwnd) {
    if (!hwnd || kHallOfFame.empty()) return;

    int nextIndex = 0;
    if (kHallOfFame.size() > 1) {
        do {
            nextIndex = (int)(NextRandomU32() % (uint32_t)kHallOfFame.size());
        } while (nextIndex == gHallOfFameIndex);
    }

    gHallOfFameIndex = nextIndex;
    const std::wstring text = kHallOfFame[(size_t)gHallOfFameIndex] + L" Oblivion: Data Files";
    SetWindowTextW(hwnd, text.c_str());
}

static std::wstring GetPluginDisplayName(const std::wstring& pluginFileName) {
    const wchar_t* ext = PathFindExtensionW(pluginFileName.c_str());
    if (!ext || ext == pluginFileName.c_str()) {
        return pluginFileName;
    }
    return pluginFileName.substr(0, static_cast<size_t>(ext - pluginFileName.c_str()));
}

static std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == L'\\') return a + b;
    return a + L"\\" + b;
}

static bool EqualsNoCase(const std::wstring& a, const std::wstring& b) {
    return _wcsicmp(a.c_str(), b.c_str()) == 0;
}

static std::wstring ToLowerCopy(const std::wstring& value) {
    std::wstring lowered = value;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](wchar_t c) {
        return (wchar_t)towlower(c);
    });
    return lowered;
}

static bool IsPluginFile(const std::wstring& fileName) {
    const wchar_t* ext = PathFindExtensionW(fileName.c_str());
    if (!ext || !*ext) return false;
    return _wcsicmp(ext, L".esm") == 0 || _wcsicmp(ext, L".esp") == 0;
}

static bool IsActivePlugin(const std::wstring& fileName) {
    return std::find_if(activePlugins.begin(), activePlugins.end(),
        [&](const std::wstring& active) { return EqualsNoCase(active, fileName); }) != activePlugins.end();
}

static std::wstring FormatDate(const FILETIME& ft) {
    FILETIME localFt = {};
    SYSTEMTIME st = {};
    if (!FileTimeToLocalFileTime(&ft, &localFt) || !FileTimeToSystemTime(&localFt, &st)) {
        return L"Unknown";
    }

    wchar_t buf[64] = {};
    swprintf_s(buf, L"%02d/%02d/%04d", st.wMonth, st.wDay, st.wYear);
    return buf;
}

static void SetCommonFont(HWND hwnd) {
    if (hwnd && g_hCommonFont) {
        SendMessageW(hwnd, WM_SETFONT, (WPARAM)g_hCommonFont, TRUE);
    }
}

static HFONT CreateUiFont(int pointSize, int weight) {
    HDC hdc = GetDC(NULL);
    const int dpiY = hdc ? GetDeviceCaps(hdc, LOGPIXELSY) : 96;
    if (hdc) ReleaseDC(NULL, hdc);

    LOGFONTW lf = {};
    lf.lfHeight = -MulDiv(pointSize, dpiY, 72);
    lf.lfWeight = weight;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcscpy_s(lf.lfFaceName, L"Segoe UI");
    return CreateFontIndirectW(&lf);
}

static std::wstring GetPluginsTxtPath() {
    wchar_t appData[MAX_PATH] = {};
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appData))) {
        return L"";
    }

    const std::wstring oblivionDir = JoinPath(appData, L"Oblivion");
    SHCreateDirectoryExW(NULL, oblivionDir.c_str(), NULL);
    return JoinPath(oblivionDir, L"Plugins.txt");
}


struct PluginHeaderMetadata {
    std::wstring author;
    std::wstring description;
    std::vector<std::wstring> masters;
};

static uint16_t ReadU16LE(const unsigned char* p) {
    return (uint16_t)(p[0] | (uint16_t(p[1]) << 8));
}

static uint32_t ReadU32LE(const unsigned char* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

static std::wstring ReadWindows1252String(const std::vector<unsigned char>& bytes) {
    if (bytes.empty()) return L"";
    int needed = MultiByteToWideChar(1252, 0, reinterpret_cast<const char*>(bytes.data()), (int)bytes.size(), nullptr, 0);
    if (needed <= 0) return L"";
    std::wstring out((size_t)needed, L'\0');
    MultiByteToWideChar(1252, 0, reinterpret_cast<const char*>(bytes.data()), (int)bytes.size(), out.data(), needed);
    while (!out.empty() && (out.back() == L'\0' || out.back() == L'\r' || out.back() == L'\n')) out.pop_back();
    return out;
}

static bool ReadPluginHeaderMetadata(const std::wstring& pluginName, PluginHeaderMetadata* outMeta) {
    if (!outMeta) return false;
    *outMeta = {};

    std::ifstream f(JoinPath(dataPath, pluginName), std::ios::binary);
    if (!f) return false;

    // Oblivion/TES4 record headers are 20 bytes for this record type (TES4).
    std::vector<unsigned char> recordHeader(20);
    f.read(reinterpret_cast<char*>(recordHeader.data()), (std::streamsize)recordHeader.size());
    if (!f || recordHeader[0] != 'T' || recordHeader[1] != 'E' || recordHeader[2] != 'S' || recordHeader[3] != '4') {
        return false;
    }

    const uint32_t dataSize = ReadU32LE(&recordHeader[4]);
    if (dataSize == 0 || dataSize > (16u * 1024u * 1024u)) {
        return false;
    }

    std::vector<unsigned char> data(dataSize);
    f.read(reinterpret_cast<char*>(data.data()), (std::streamsize)data.size());
    if (!f) return false;

    size_t off = 0;
    while (off + 6 <= data.size()) {
        const unsigned char* sh = &data[off];
        const uint16_t subLen = ReadU16LE(&sh[4]);
        off += 6;
        if (off + subLen > data.size()) break;

        const std::vector<unsigned char> payload(data.begin() + off, data.begin() + off + subLen);
        off += subLen;

        const bool isCNAM = sh[0] == 'C' && sh[1] == 'N' && sh[2] == 'A' && sh[3] == 'M';
        const bool isSNAM = sh[0] == 'S' && sh[1] == 'N' && sh[2] == 'A' && sh[3] == 'M';
        const bool isMAST = sh[0] == 'M' && sh[1] == 'A' && sh[2] == 'S' && sh[3] == 'T';

        if (isCNAM) {
            outMeta->author = ReadWindows1252String(payload);
        } else if (isSNAM) {
            outMeta->description = ReadWindows1252String(payload);
        } else if (isMAST) {
            std::wstring master = ReadWindows1252String(payload);
            if (!master.empty()) outMeta->masters.push_back(master);
        }
    }

    return true;
}

static bool GetPluginFileInfo(const std::wstring& pluginName, WIN32_FILE_ATTRIBUTE_DATA* outFileInfo) {
    if (!outFileInfo) return false;
    const std::wstring pluginPath = JoinPath(dataPath, pluginName);
    return GetFileAttributesExW(pluginPath.c_str(), GetFileExInfoStandard, outFileInfo) == TRUE;
}

static void UpdatePluginDetails(int selectedIndex) {
    if (selectedIndex < 0 || selectedIndex >= (int)pluginFiles.size()) {
        SetWindowTextW(hPluginName, L"");
        SetWindowTextW(hCreatedBy, L"Created by: Unknown");
        SetWindowTextW(hDescription, L"");
        SetWindowTextW(hCreatedOn, L"Created on: Unknown");
        SetWindowTextW(hModifiedOn, L"Last modified: Unknown");
        return;
    }

    const std::wstring& plugin = pluginFiles[(size_t)selectedIndex];
    SetWindowTextW(hPluginName, GetPluginDisplayName(plugin).c_str());

    PluginHeaderMetadata meta = {};
    ReadPluginHeaderMetadata(plugin, &meta);

    const std::wstring author = meta.author.empty() ? L"Unknown" : meta.author;
    SetWindowTextW(hCreatedBy, (L"Created by: " + author).c_str());

    WIN32_FILE_ATTRIBUTE_DATA fileInfo = {};
    std::wstring createdOn = L"Created on: Unknown";
    std::wstring modifiedOn = L"Last modified: Unknown";
    if (GetPluginFileInfo(plugin, &fileInfo)) {
        createdOn = L"Created on: " + FormatDate(fileInfo.ftCreationTime);
        modifiedOn = L"Last modified: " + FormatDate(fileInfo.ftLastWriteTime);
    }

    SetWindowTextW(hCreatedOn, createdOn.c_str());
    SetWindowTextW(hModifiedOn, modifiedOn.c_str());

    const wchar_t* ext = PathFindExtensionW(plugin.c_str());
    const bool isMaster = ext && _wcsicmp(ext, L".esm") == 0;

    std::wstring desc;
    desc += L"\r\n";
    if (!meta.description.empty()) {
        desc += meta.description + L"\r\n\r\n";
    }
    else {
        desc += L"No description data available for this plugin.\r\n\r\n";
    }
    desc += L"Master Files Required:\r\n";
    if (!meta.masters.empty()) {
        for (size_t i = 0; i < meta.masters.size(); ++i) {
            desc += meta.masters[i];
            if (i + 1 < meta.masters.size()) desc += L"\r\n";
        }
    }
    else {
        desc += isMaster ? L"(none)" : L"Oblivion.esm";
    }

    SetWindowTextW(hDescription, desc.c_str());
}

static void SelectFirstPlugin() {
    if (!hPluginList || pluginFiles.empty()) {
        UpdatePluginDetails(-1);
        return;
    }

    ListView_SetItemState(hPluginList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    ListView_EnsureVisible(hPluginList, 0, FALSE);
    UpdatePluginDetails(0);
}

static void ResetToDefaults() {
    if (!hPluginList) return;

    const int count = ListView_GetItemCount(hPluginList);
    for (int i = 0; i < count; ++i) {
        ListView_SetCheckState(hPluginList, i, FALSE);
    }

    for (int i = 0; i < count; ++i) {
        wchar_t buffer[MAX_PATH] = {};
        ListView_GetItemText(hPluginList, i, 0, buffer, MAX_PATH);
        if (_wcsicmp(buffer, L"Oblivion.esm") == 0) {
            ListView_SetCheckState(hPluginList, i, TRUE);
            break;
        }
    }
}

static void SortPluginsSuggestedLoadOrder() {
    if (!hPluginList || pluginFiles.empty()) return;

    std::unordered_map<std::wstring, size_t> indexByName;
    indexByName.reserve(pluginFiles.size());
    for (size_t i = 0; i < pluginFiles.size(); ++i) {
        indexByName[ToLowerCopy(pluginFiles[i])] = i;
    }

    std::vector<PluginHeaderMetadata> metadata(pluginFiles.size());
    for (size_t i = 0; i < pluginFiles.size(); ++i) {
        ReadPluginHeaderMetadata(pluginFiles[i], &metadata[i]);
    }

    std::vector<int> order;
    order.reserve(pluginFiles.size());
    std::vector<unsigned char> state(pluginFiles.size(), 0);

    std::function<void(size_t)> visit = [&](size_t i) {
        if (state[i] == 2) return;
        if (state[i] == 1) return;
        state[i] = 1;

        for (const std::wstring& master : metadata[i].masters) {
            auto it = indexByName.find(ToLowerCopy(master));
            if (it != indexByName.end()) {
                visit(it->second);
            }
        }

        state[i] = 2;
        order.push_back((int)i);
    };

    std::vector<size_t> indices(pluginFiles.size());
    for (size_t i = 0; i < pluginFiles.size(); ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(), [](size_t a, size_t b) {
        const bool aIsOblivion = _wcsicmp(pluginFiles[a].c_str(), L"Oblivion.esm") == 0;
        const bool bIsOblivion = _wcsicmp(pluginFiles[b].c_str(), L"Oblivion.esm") == 0;
        if (aIsOblivion != bIsOblivion) return aIsOblivion;

        const std::wstring aExt = PathFindExtensionW(pluginFiles[a].c_str());
        const std::wstring bExt = PathFindExtensionW(pluginFiles[b].c_str());
        const bool aIsMaster = _wcsicmp(aExt.c_str(), L".esm") == 0;
        const bool bIsMaster = _wcsicmp(bExt.c_str(), L".esm") == 0;
        if (aIsMaster != bIsMaster) return aIsMaster;

        return _wcsicmp(pluginFiles[a].c_str(), pluginFiles[b].c_str()) < 0;
    });

    for (size_t i : indices) {
        visit(i);
    }

    std::vector<std::wstring> reordered;
    reordered.reserve(order.size());
    for (int idx : order) {
        reordered.push_back(pluginFiles[(size_t)idx]);
    }

    pluginFiles = std::move(reordered);
    PopulatePluginList(hPluginList);
    SelectFirstPlugin();
}

} // namespace

std::wstring GetOblivionDataPath() {
    HKEY hKey;
    wchar_t path[MAX_PATH] = {};
    DWORD size = sizeof(path);

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Bethesda Softworks\\Oblivion", 0, KEY_READ | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExW(hKey, L"Installed Path", NULL, NULL, (LPBYTE)path, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::wstring(path) + L"Data\\";
        }
        RegCloseKey(hKey);
    }

    return L"";
}

void LoadPlugins() {
    pluginFiles.clear();

    WIN32_FIND_DATAW findData = {};
    HANDLE hFind = FindFirstFileW((dataPath + L"*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                const std::wstring fileName = findData.cFileName;
                if (IsPluginFile(fileName)) {
                    pluginFiles.push_back(fileName);
                }
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }

    std::sort(pluginFiles.begin(), pluginFiles.end(),
        [](const std::wstring& a, const std::wstring& b) { return _wcsicmp(a.c_str(), b.c_str()) < 0; });
    pluginFiles.erase(std::unique(pluginFiles.begin(), pluginFiles.end(),
        [](const std::wstring& a, const std::wstring& b) { return EqualsNoCase(a, b); }),
        pluginFiles.end());
}

void LoadActivePlugins() {
    activePlugins.clear();

    const std::wstring pluginsTxt = GetPluginsTxtPath();
    if (pluginsTxt.empty()) return;

    std::wifstream inFile(pluginsTxt);
    if (!inFile) return;

    std::wstring line;
    while (std::getline(inFile, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        if (!line.empty()) {
            activePlugins.push_back(line);
        }
    }
}

void SaveActivePlugins() {
    const std::wstring pluginsTxt = GetPluginsTxtPath();
    if (pluginsTxt.empty() || !hPluginList) return;

    std::wofstream outFile(pluginsTxt, std::ios::trunc);
    if (!outFile) {
        MessageBoxW(NULL, L"Unable to write Plugins.txt", L"Oblivion: Data Files", MB_OK | MB_ICONERROR);
        return;
    }

    const int count = ListView_GetItemCount(hPluginList);
    for (int i = 0; i < count; ++i) {
        if (ListView_GetCheckState(hPluginList, i)) {
            wchar_t buffer[MAX_PATH] = {};
            ListView_GetItemText(hPluginList, i, 0, buffer, MAX_PATH);
            outFile << buffer << std::endl;
        }
    }
}

void PopulatePluginList(HWND hwndList) {
    if (!hwndList) return;

    ListView_DeleteAllItems(hwndList);

    for (size_t i = 0; i < pluginFiles.size(); ++i) {
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = (int)i;
        item.pszText = const_cast<LPWSTR>(pluginFiles[i].c_str());
        ListView_InsertItem(hwndList, &item);

        if (IsActivePlugin(pluginFiles[i])) {
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

        g_hCommonFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        g_hTitleFont = CreateUiFont(13, FW_NORMAL);
        g_hMetaFont = CreateUiFont(10, FW_NORMAL);

        const int leftX = 18;
        const int topY = 18;
        const int leftW = 348;
        const int listH = 498;

        const int rightX = 378;
        const int rightW = 288;
        const int pluginNameY = 22;
        const int authorY = 72;
        const int descriptionY = 84;
        const int descriptionH = 330;
        const int createdOnY = 424;
        const int modifiedOnY = 450;

        hPluginList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, NULL,
            WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER,
            leftX, topY, leftW, listH,
            hwnd, (HMENU)IDC_PLUGIN_LIST, NULL, NULL);

        hHallOfFame = CreateWindowW(L"STATIC", L"Hall of Fame:", WS_VISIBLE | WS_CHILD,
            378, 18, 288, 24, hwnd, NULL, NULL, NULL);

        hPluginName = CreateWindowW(L"STATIC", L"", WS_VISIBLE | WS_CHILD,
            rightX, pluginNameY, rightW, 26, hwnd, NULL, NULL, NULL);

        hCreatedBy = CreateWindowW(L"STATIC", L"Created by: Unknown", WS_VISIBLE | WS_CHILD,
            rightX, authorY, rightW, 22, hwnd, NULL, NULL, NULL);

        hDescription = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            rightX, descriptionY, rightW, descriptionH,
            hwnd, (HMENU)IDC_DESCRIPTION_EDIT, NULL, NULL);

        hCreatedOn = CreateWindowW(L"STATIC", L"Created on: Unknown", WS_VISIBLE | WS_CHILD,
            rightX, createdOnY, rightW, 22, hwnd, NULL, NULL, NULL);

        hModifiedOn = CreateWindowW(L"STATIC", L"Last modified: Unknown", WS_VISIBLE | WS_CHILD,
            rightX, modifiedOnY, rightW, 22, hwnd, NULL, NULL, NULL);

        HWND hReset = CreateWindowW(L"BUTTON", L"Reset to Defaults", WS_VISIBLE | WS_CHILD,
            18, 530, 178, 36,
            hwnd, (HMENU)IDC_RESET_BUTTON, NULL, NULL);

        HWND hSort = CreateWindowW(L"BUTTON", L"Sort", WS_VISIBLE | WS_CHILD,
            206, 530, 100, 36,
            hwnd, (HMENU)IDC_SORT_BUTTON, NULL, NULL);

        HWND hCancel = CreateWindowW(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            372, 530, 136, 36,
            hwnd, (HMENU)IDC_CANCEL_BUTTON, NULL, NULL);

        HWND hOk = CreateWindowW(L"BUTTON", L"OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            524, 530, 136, 36,
            hwnd, (HMENU)IDC_OK_BUTTON, NULL, NULL);

        if (!hPluginList || !hPluginName || !hCreatedBy ||
            !hDescription || !hCreatedOn || !hModifiedOn || !hReset || !hSort || !hCancel || !hOk) {
            MessageBoxW(hwnd, L"Failed to create Data Files controls.", L"Oblivion: Data Files", MB_OK | MB_ICONERROR);
            DestroyWindow(hwnd);
            return -1;
        }

        SetCommonFont(hPluginList);
        SetCommonFont(hDescription);
        if (g_hTitleFont) SendMessageW(hPluginName, WM_SETFONT, (WPARAM)g_hTitleFont, TRUE); else SetCommonFont(hPluginName);
        if (g_hMetaFont) {
            SendMessageW(hCreatedBy, WM_SETFONT, (WPARAM)g_hMetaFont, TRUE);
            SendMessageW(hCreatedOn, WM_SETFONT, (WPARAM)g_hMetaFont, TRUE);
            SendMessageW(hModifiedOn, WM_SETFONT, (WPARAM)g_hMetaFont, TRUE);
        } else {
            SetCommonFont(hCreatedBy);
            SetCommonFont(hCreatedOn);
            SetCommonFont(hModifiedOn);
        }
        SetCommonFont(hReset);
        SetCommonFont(hSort);
        SetCommonFont(hCancel);
        SetCommonFont(hOk);

        ListView_SetExtendedListViewStyle(hPluginList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
        LVCOLUMNW col = {};
        col.mask = LVCF_WIDTH;
        col.cx = 326;
        ListView_InsertColumn(hPluginList, 0, &col);

        PopulatePluginList(hPluginList);
        SelectFirstPlugin();

        gHallOfFameRngState = (uint32_t)GetTickCount() ^ (uint32_t)GetCurrentProcessId() ^ 0xC2B2AE35u;
        UpdateHallOfFameTitle(hwnd);
        SetTimer(hwnd, IDT_HALL_OF_FAME, 5000, NULL);
        return 0;
    }

    case WM_NOTIFY:
    {
        NMHDR* hdr = reinterpret_cast<NMHDR*>(lParam);
        if (hdr && hdr->idFrom == IDC_PLUGIN_LIST && hdr->code == LVN_ITEMCHANGED) {
            NMLISTVIEW* lv = reinterpret_cast<NMLISTVIEW*>(lParam);
            if (lv && lv->iItem >= 0 && (lv->uNewState & LVIS_SELECTED) && !(lv->uOldState & LVIS_SELECTED)) {
                UpdatePluginDetails(lv->iItem);
            }
        }
        break;
    }

    case WM_TIMER:
        if (wParam == IDT_HALL_OF_FAME) {
            UpdateHallOfFameTitle(hwnd);
            return 0;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OK_BUTTON:
            SaveActivePlugins();
            DestroyWindow(hwnd);
            return 0;
        case IDC_CANCEL_BUTTON:
            DestroyWindow(hwnd);
            return 0;
        case IDC_RESET_BUTTON:
            ResetToDefaults();
            return 0;
        case IDC_SORT_BUTTON:
            SortPluginsSuggestedLoadOrder();
            return 0;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_NCDESTROY:
        KillTimer(hwnd, IDT_HALL_OF_FAME);
        hPluginList = NULL;
        hHallOfFame = NULL;
        hPluginName = NULL;
        hCreatedBy = NULL;
        hDescription = NULL;
        hCreatedOn = NULL;
        hModifiedOn = NULL;
        if (g_hTitleFont) { DeleteObject(g_hTitleFont); g_hTitleFont = NULL; }
        if (g_hMetaFont) { DeleteObject(g_hMetaFont); g_hMetaFont = NULL; }
        g_hCommonFont = NULL;
        gHallOfFameIndex = -1;
        gHallOfFameRngState = 0;
        break;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void OpenDataFilesWindow(HWND hParent) {
    dataPath = GetOblivionDataPath();
    if (dataPath.empty()) {
        MessageBoxW(hParent, L"Oblivion install path not found.", L"Error", MB_ICONERROR);
        return;
    }

    LoadPlugins();
    LoadActivePlugins();

    WNDCLASSW wc = {};
    wc.lpfnWndProc = DataFilesWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = L"DataFilesWindow";

    ATOM cls = RegisterClassW(&wc);
    if (cls == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        MessageBoxW(hParent, L"Unable to register Data Files window class.", L"Error", MB_ICONERROR);
        return;
    }

    HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, L"DataFilesWindow", L"Oblivion: Data Files",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 702, 624,
        hParent, NULL, GetModuleHandleW(NULL), NULL);

    if (!hwnd) {
        MessageBoxW(hParent, L"Unable to create Data Files window.", L"Error", MB_ICONERROR);
        return;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}
