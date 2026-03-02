#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "DataFilesWindow.h"
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <winreg.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")

#define IDC_PLUGIN_LIST      1001
#define IDC_OK_BUTTON        1002
#define IDC_CANCEL_BUTTON    1003
#define IDC_RESET_BUTTON     1004
#define IDC_DESCRIPTION_EDIT 1005

static std::vector<std::wstring> pluginFiles;
static std::vector<std::wstring> activePlugins;
static HWND hPluginList = NULL;
static HWND hPluginName = NULL;
static HWND hCreatedBy = NULL;
static HWND hDescription = NULL;
static HWND hCreatedOn = NULL;
static HWND hModifiedOn = NULL;
static std::wstring dataPath;

static HFONT g_hCommonFont = NULL;

static std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == L'\\') return a + b;
    return a + L"\\" + b;
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

    std::sort(pluginFiles.begin(), pluginFiles.end(), [](const std::wstring& a, const std::wstring& b) {
        return _wcsicmp(a.c_str(), b.c_str()) < 0;
    });
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

static void SetCommonFont(HWND hwnd) {
    if (hwnd && g_hCommonFont) {
        SendMessage(hwnd, WM_SETFONT, (WPARAM)g_hCommonFont, TRUE);
    }
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
    SetWindowTextW(hPluginName, plugin.c_str());

    const std::wstring extension = PathFindExtensionW(plugin.c_str()) ? PathFindExtensionW(plugin.c_str()) : L"";
    std::wstring createdBy = L"Created by: " + (extension == L".esm" ? std::wstring(L"Bethesda Softworks") : std::wstring(L"Mod Author"));
    SetWindowTextW(hCreatedBy, createdBy.c_str());

    std::wstring pluginPath = JoinPath(dataPath, plugin);
    WIN32_FIND_DATA fd = {};
    HANDLE hFind = FindFirstFileW(pluginPath.c_str(), &fd);
    std::wstring createdOn = L"Created on: Unknown";
    std::wstring modifiedOn = L"Last modified: Unknown";
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        createdOn = L"Created on: " + FormatDate(fd.ftCreationTime);
        modifiedOn = L"Last modified: " + FormatDate(fd.ftLastWriteTime);
    }

    SetWindowTextW(hCreatedOn, createdOn.c_str());
    SetWindowTextW(hModifiedOn, modifiedOn.c_str());

    std::wstring desc;
    desc += L"\r\n";
    desc += L"No description data available for this plugin.\r\n\r\n";
    desc += L"Master Files Required:\r\n";
    desc += (extension == L".esm") ? L"(none)" : L"Oblivion.esm";
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

LRESULT CALLBACK DataFilesWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
    {
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&icex);

        g_hCommonFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        HWND hDataFilesLabel = CreateWindowW(L"STATIC", L"Data Files:", WS_VISIBLE | WS_CHILD,
            18, 18, 100, 22, hwnd, NULL, NULL, NULL);
        SetCommonFont(hDataFilesLabel);

        HWND hLeftFrame = CreateWindowW(L"BUTTON", L"", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
            18, 42, 348, 474, hwnd, NULL, NULL, NULL);
        SetCommonFont(hLeftFrame);

        hPluginList = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, NULL,
            WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_NOCOLUMNHEADER,
            28, 56, 338, 434,
            hwnd, (HMENU)IDC_PLUGIN_LIST, NULL, NULL);
        SetCommonFont(hPluginList);

        ListView_SetExtendedListViewStyle(hPluginList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
        LVCOLUMNW col = {};
        col.mask = LVCF_WIDTH;
        col.cx = 318;
        ListView_InsertColumn(hPluginList, 0, &col);

        hPluginName = CreateWindowW(L"STATIC", L"", WS_VISIBLE | WS_CHILD,
            378, 44, 288, 24, hwnd, NULL, NULL, NULL);
        SetCommonFont(hPluginName);

        hCreatedBy = CreateWindowW(L"STATIC", L"Created by: Unknown", WS_VISIBLE | WS_CHILD,
            378, 70, 288, 24, hwnd, NULL, NULL, NULL);
        SetCommonFont(hCreatedBy);

        hDescription = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            378, 98, 288, 316,
            hwnd, (HMENU)IDC_DESCRIPTION_EDIT, NULL, NULL);
        SetCommonFont(hDescription);

        hCreatedOn = CreateWindowW(L"STATIC", L"Created on: Unknown", WS_VISIBLE | WS_CHILD,
            378, 426, 288, 24, hwnd, NULL, NULL, NULL);
        SetCommonFont(hCreatedOn);

        hModifiedOn = CreateWindowW(L"STATIC", L"Last modified: Unknown", WS_VISIBLE | WS_CHILD,
            378, 452, 288, 24, hwnd, NULL, NULL, NULL);
        SetCommonFont(hModifiedOn);

        HWND hReset = CreateWindowW(L"BUTTON", L"Reset to Defaults", WS_VISIBLE | WS_CHILD,
            18, 530, 288, 36,
            hwnd, (HMENU)IDC_RESET_BUTTON, NULL, NULL);
        SetCommonFont(hReset);

        HWND hCancel = CreateWindowW(L"BUTTON", L"Cancel", WS_VISIBLE | WS_CHILD,
            372, 530, 136, 36,
            hwnd, (HMENU)IDC_CANCEL_BUTTON, NULL, NULL);
        SetCommonFont(hCancel);

        HWND hOk = CreateWindowW(L"BUTTON", L"OK", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            524, 530, 136, 36,
            hwnd, (HMENU)IDC_OK_BUTTON, NULL, NULL);
        SetCommonFont(hOk);

        PopulatePluginList(hPluginList);
        SelectFirstPlugin();
        return 0;
    }
    case WM_NOTIFY:
    {
        NMHDR* hdr = reinterpret_cast<NMHDR*>(lParam);
        if (hdr && hdr->idFrom == IDC_PLUGIN_LIST && hdr->code == LVN_ITEMCHANGED) {
            NMLISTVIEW* lv = reinterpret_cast<NMLISTVIEW*>(lParam);
            if (lv && (lv->uNewState & LVIS_SELECTED) && lv->iItem >= 0) {
                UpdatePluginDetails(lv->iItem);
            }
        }
        break;
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
        case IDC_RESET_BUTTON:
            ResetToDefaults();
            return 0;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_hCommonFont = NULL;
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

    WNDCLASSW wc = {};
    wc.lpfnWndProc = DataFilesWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszClassName = L"DataFilesWindow";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, L"DataFilesWindow", L"Oblivion: Data Files",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 702, 624,
        hParent, NULL, GetModuleHandle(NULL), NULL);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}
