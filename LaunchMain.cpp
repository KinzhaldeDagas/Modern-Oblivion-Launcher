#include <windows.h>
#include <windowsx.h>
#include <uxtheme.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <string>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
#include "resource.h"
#include "OptionsWindow.h"
#include "DataFilesWindow.h"

using namespace Gdiplus;

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 550
#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 35
#define BUTTON_SPACING 20
#define EXTRA_LEFT_SHIFT 32
#define CSE_BUTTON_EXTRA_WIDTH 16

enum ButtonIDs {
    ID_BUTTON_PLAY = 1,
    ID_BUTTON_LAUNCH_CSE,
    ID_BUTTON_OPTIONS,
    ID_BUTTON_DATAFILES,
    ID_BUTTON_TECHSUPPORT,
    ID_BUTTON_EXIT
};

enum class ConstructionSetLaunchMode {
    None,
    Batch,
    Exe
};

static ULONG_PTR g_gdiplusToken = 0;
static Image* g_pBackground = nullptr;
static Image* g_pButtonBg = nullptr;
static HFONT  g_hCustomFont = nullptr;
static std::wstring g_fontFaceName;
static std::wstring g_fontFilePath;
static HINSTANCE g_hInstance = nullptr;
static HWND g_hLaunchCseButton = nullptr;
static ConstructionSetLaunchMode g_csLaunchMode = ConstructionSetLaunchMode::None;
static std::wstring g_csLaunchPath;
static std::wstring g_csLaunchWorkingDir;

static std::unordered_map<HWND, bool> g_btnHot;

static LRESULT CALLBACK HoverBtnSubProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
    UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (msg) {
    case WM_NCDESTROY:
        g_btnHot.erase(hWnd);
        RemoveWindowSubclass(hWnd, HoverBtnSubProc, uIdSubclass);
        break;
    case WM_MOUSEMOVE: {
        if (!g_btnHot[hWnd]) {
            g_btnHot[hWnd] = true;
            TRACKMOUSEEVENT tme = { sizeof(tme) };
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
            InvalidateRect(hWnd, nullptr, TRUE);
        }
        break;
    }
    case WM_MOUSELEAVE:
        if (g_btnHot[hWnd]) {
            g_btnHot[hWnd] = false;
            InvalidateRect(hWnd, nullptr, TRUE);
        }
        break;
    }
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}


HGLOBAL LoadResourceData(WORD r, const wchar_t* t, DWORD& outSize) {
    HMODULE mod = GetModuleHandle(nullptr);
    HRSRC res = FindResource(mod, MAKEINTRESOURCE(r), t);
    if (!res) return nullptr;
    DWORD size = SizeofResource(mod, res);
    HGLOBAL loaded = LoadResource(mod, res);
    if (!loaded) return nullptr;
    void* ptr = LockResource(loaded);
    if (!ptr) return nullptr;
    HGLOBAL mem = GlobalAlloc(GMEM_FIXED, size);
    if (mem) {
        memcpy(mem, ptr, size);
        outSize = size;
    }
    return mem;
}

Image* LoadPNGFromResource(WORD r) {
    DWORD dataSize = 0;
    HGLOBAL data = LoadResourceData(r, RT_RCDATA, dataSize);
    if (!data) return nullptr;
    IStream* stm = nullptr;
    if (CreateStreamOnHGlobal(data, TRUE, &stm) != S_OK) return nullptr;
    Image* img = Image::FromStream(stm);
    stm->Release();
    if (img && img->GetLastStatus() != Ok) {
        delete img;
        img = nullptr;
    }
    return img;
}

HFONT LoadCustomFontAndCreateHFONT(WORD r, float fontSize) {
    (void)r;
    (void)fontSize;
    return nullptr;
}

static std::wstring JoinPath(const std::wstring& a, const std::wstring& b) {
    if (a.empty()) return b;
    if (b.empty()) return a;
    if (a.back() == L'\\') return a + b;
    return a + L"\\" + b;
}

static std::wstring GetExeDir() {
    wchar_t buf[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
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
    // Expected: oblivionfont.ttf next to the .sln.
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

static uint16_t ReadU16BE(const uint8_t* p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}
static uint32_t ReadU32BE(const uint8_t* p) {
    return (uint32_t)((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

static std::wstring ExtractFamilyNameFromTTF(const std::wstring& ttfPath) {
    std::ifstream f(ttfPath, std::ios::binary);
    if (!f) return L"";
    f.seekg(0, std::ios::end);
    const std::streamoff sz = f.tellg();
    if (sz <= 0 || sz > (50ll * 1024ll * 1024ll)) return L"";
    f.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes((size_t)sz);
    f.read((char*)bytes.data(), sz);
    if (!f) return L"";
    if (bytes.size() < 12) return L"";

    const uint16_t numTables = ReadU16BE(&bytes[4]);
    if (bytes.size() < 12ull + (size_t)numTables * 16ull) return L"";

    uint32_t nameOffset = 0, nameLength = 0;
    for (uint16_t i = 0; i < numTables; i++) {
        const size_t off = 12ull + (size_t)i * 16ull;
        const uint32_t tag = ReadU32BE(&bytes[off + 0]);
        const uint32_t to = ReadU32BE(&bytes[off + 8]);
        const uint32_t tl = ReadU32BE(&bytes[off + 12]);
        // 'name'
        if (tag == 0x6E616D65u) {
            nameOffset = to;
            nameLength = tl;
            break;
        }
    }
    if (!nameOffset || nameOffset + nameLength > bytes.size()) return L"";
    if (nameLength < 6) return L"";

    const uint8_t* nt = bytes.data() + nameOffset;
    const uint16_t count = ReadU16BE(nt + 2);
    const uint16_t stringOffset = ReadU16BE(nt + 4);
    if (6ull + (size_t)count * 12ull > nameLength) return L"";
    if ((size_t)stringOffset > nameLength) return L"";

    auto pick = [&](uint16_t wantedNameId) -> std::wstring {
        for (uint16_t i = 0; i < count; i++) {
            const uint8_t* r = nt + 6 + (size_t)i * 12;
            const uint16_t platformID = ReadU16BE(r + 0);
            const uint16_t encodingID = ReadU16BE(r + 2);
            const uint16_t languageID = ReadU16BE(r + 4);
            const uint16_t nameID = ReadU16BE(r + 6);
            const uint16_t len = ReadU16BE(r + 8);
            const uint16_t roff = ReadU16BE(r + 10);

            if (platformID != 3) continue; // Windows
            if (!(encodingID == 0 || encodingID == 1 || encodingID == 10)) continue;
            if (languageID != 0x0409) continue; // en-US
            if (nameID != wantedNameId) continue;

            const size_t sbase = nameOffset + (size_t)stringOffset + (size_t)roff;
            if (sbase + len > bytes.size()) continue;
            if (len < 2) continue;

            std::wstring out;
            out.reserve(len / 2);
            for (size_t j = 0; j + 1 < len; j += 2) {
                const uint8_t b0 = bytes[sbase + j];
                const uint8_t b1 = bytes[sbase + j + 1];
                const wchar_t wc = (wchar_t)((b0 << 8) | b1);
                if (wc == 0) continue;
                out.push_back(wc);
            }
            return out;
        }
        return L"";
        };

    std::wstring fam = pick(1); // Font Family
    if (!fam.empty()) return fam;
    fam = pick(4); // Full name
    return fam;
}

static HFONT LoadOblivionFontFromFile(float pt, bool bold) {
    g_fontFilePath = FindFontFileNearSolutionRoot();
    if (g_fontFilePath.empty()) return nullptr;

    g_fontFaceName = ExtractFamilyNameFromTTF(g_fontFilePath);
    if (g_fontFaceName.empty()) g_fontFaceName = L"Oblivion";

    AddFontResourceExW(g_fontFilePath.c_str(), FR_PRIVATE, nullptr);

    HDC hdc = GetDC(nullptr);
    const int logpx = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(nullptr, hdc);
    const int height = -MulDiv((int)(pt + 0.5f), logpx, 72);

    LOGFONTW lf = {};
    lf.lfHeight = height;
    lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    lf.lfQuality = CLEARTYPE_QUALITY;
    wcsncpy_s(lf.lfFaceName, g_fontFaceName.c_str(), _TRUNCATE);
    return CreateFontIndirectW(&lf);
}

static std::wstring ReadOblivionPathFromReg(const wchar_t* subKey) {
    std::wstring result;
    HKEY hKey = nullptr;
    LONG lr = RegOpenKeyExW(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ | KEY_WOW64_32KEY, &hKey);
    if (lr == ERROR_SUCCESS) {
        wchar_t pathBuf[MAX_PATH];
        DWORD bufSize = sizeof(pathBuf);
        if (RegQueryValueExW(hKey, L"Installed Path", nullptr, nullptr, (LPBYTE)pathBuf, &bufSize) == ERROR_SUCCESS) {
            result = pathBuf;
            if (!result.empty() && result.back() != L'\\') {
                result.push_back(L'\\');
            }
        }
        RegCloseKey(hKey);
    }
    return result;
}

static std::wstring GetOblivionInstallPath() {
    std::wstring path = ReadOblivionPathFromReg(L"SOFTWARE\\Wow6432Node\\Bethesda Softworks\\Oblivion");
    if (!path.empty()) return path;
    path = ReadOblivionPathFromReg(L"SOFTWARE\\Bethesda Softworks\\Oblivion");
    return path;
}

static std::wstring GetLocalDir() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpecW(exePath);
    std::wstring localDir = exePath;
    if (!localDir.empty() && localDir.back() != L'\\') {
        localDir.push_back(L'\\');
    }
    return localDir;
}

static void LaunchObseLoader() {
    std::wstring basePath = GetOblivionInstallPath();
    if (basePath.empty()) {
        basePath = GetLocalDir();
    }
    std::wstring obseExe = basePath + L"obse_loader.exe";
    ShellExecuteW(nullptr, L"open", obseExe.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

static std::wstring ResolveOblivionRootPath() {
    std::wstring basePath = GetOblivionInstallPath();
    if (basePath.empty()) {
        basePath = GetLocalDir();
    }
    return basePath;
}

static void DetectConstructionSetLaunchTarget() {
    g_csLaunchMode = ConstructionSetLaunchMode::None;
    g_csLaunchPath.clear();
    g_csLaunchWorkingDir.clear();

    const std::wstring rootPath = ResolveOblivionRootPath();
    const std::wstring oblivionExe = rootPath + L"Oblivion.exe";
    if (!PathFileExistsW(oblivionExe.c_str())) {
        return;
    }

    const std::wstring cseBatch = rootPath + L"Launch CSE.bat";
    const std::wstring csExe = rootPath + L"TESConstructionSet.exe";

    if (PathFileExistsW(cseBatch.c_str())) {
        g_csLaunchMode = ConstructionSetLaunchMode::Batch;
        g_csLaunchPath = cseBatch;
        g_csLaunchWorkingDir = rootPath;
        return;
    }

    if (PathFileExistsW(csExe.c_str())) {
        g_csLaunchMode = ConstructionSetLaunchMode::Exe;
        g_csLaunchPath = csExe;
        g_csLaunchWorkingDir = rootPath;
    }
}

static void LaunchConstructionSet() {
    if (g_csLaunchMode == ConstructionSetLaunchMode::None || g_csLaunchPath.empty()) {
        return;
    }

    if (g_csLaunchMode == ConstructionSetLaunchMode::Batch) {
        std::wstring params = L"/C \"\"" + g_csLaunchPath + L"\"\"";
        ShellExecuteW(
            nullptr,
            L"open",
            L"cmd.exe",
            params.c_str(),
            g_csLaunchWorkingDir.empty() ? nullptr : g_csLaunchWorkingDir.c_str(),
            SW_SHOWNORMAL
        );
        return;
    }

    ShellExecuteW(
        nullptr,
        L"open",
        g_csLaunchPath.c_str(),
        nullptr,
        g_csLaunchWorkingDir.empty() ? nullptr : g_csLaunchWorkingDir.c_str(),
        SW_SHOWNORMAL
    );
}

void DrawButtonItem(const DRAWITEMSTRUCT* ds) {
    int bx = ds->rcItem.left;
    int by = ds->rcItem.top;
    int bw = ds->rcItem.right - bx;
    int bh = ds->rcItem.bottom - by;
    HDC dc = ds->hDC;

    const bool hot = (g_btnHot.find(ds->hwndItem) != g_btnHot.end()) ? g_btnHot[ds->hwndItem] : false;
    const bool down = (ds->itemState & ODS_SELECTED) != 0;

    // Base pass: repaint whatever is under the button so we never leave a white/opaque text backdrop.
    // Since the main window background is a scaled image, redraw it aligned into the button DC.
    if (g_pBackground) {
        HWND parent = GetParent(ds->hwndItem);
        RECT prc = {};
        GetClientRect(parent ? parent : GetDesktopWindow(), &prc);
        const int ww = prc.right - prc.left;
        const int hh = prc.bottom - prc.top;

        RECT br = {};
        GetWindowRect(ds->hwndItem, &br);
        POINT tl = { br.left, br.top };
        if (parent) ScreenToClient(parent, &tl);

        Graphics base(dc);
        base.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        // Draw the full scaled background shifted so the correct subsection lands in the button.
        base.DrawImage(g_pBackground, (REAL)-tl.x, (REAL)-tl.y, (REAL)ww, (REAL)hh);
    }
    else {
        RECT rc = ds->rcItem;
        FillRect(dc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
    }

    // Draw the button background ONLY when hovered (or pressed), at 88% opacity.
    if (g_pButtonBg && (hot || down)) {
        Graphics gfx(dc);
        gfx.SetInterpolationMode(InterpolationModeHighQualityBicubic);

        const float a = down ? 0.78f : 0.88f; // slightly lower when pressed
        ColorMatrix cm = {
            1,0,0,0,0,
            0,1,0,0,0,
            0,0,1,0,0,
            0,0,0,a,0,
            0,0,0,0,1
        };
        ImageAttributes ia;
        ia.SetColorMatrix(&cm, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

        Rect dest(bx, by, bw, bh);
        gfx.DrawImage(
            g_pButtonBg,
            dest,
            0, 0,
            (INT)g_pButtonBg->GetWidth(),
            (INT)g_pButtonBg->GetHeight(),
            UnitPixel,
            &ia
        );
    }

    SetBkMode(dc, TRANSPARENT);
    const bool disabled = (ds->itemState & ODS_DISABLED) != 0;
    SetTextColor(dc, disabled ? RGB(122, 122, 122) : RGB(96, 57, 19));

    wchar_t txt[128] = {};
    GetWindowText(ds->hwndItem, txt, 128);

    RECT rc = ds->rcItem;
    if (down) {
        rc.left++; rc.right++; rc.top++; rc.bottom++;
    }
    rc.left += 4;
    rc.right -= 4;
    DrawTextW(dc, txt, -1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST:
        return HTCAPTION;  // Makes the window draggable by treating the client area as the title bar.
    case WM_CREATE: {
        g_hCustomFont = LoadOblivionFontFromFile(18.0f, false);
        DetectConstructionSetLaunchTarget();

        int xPos = WINDOW_WIDTH - BUTTON_WIDTH - 128 - EXTRA_LEFT_SHIFT;
        int totalBtns = 6;
        int totalHeight = totalBtns * BUTTON_HEIGHT + (totalBtns - 1) * BUTTON_SPACING;
        int startY = (WINDOW_HEIGHT - totalHeight) / 2;
        auto makeBtn = [&](const wchar_t* text, int id, int idx) {
            HWND hBtn = CreateWindowW(
                L"BUTTON",
                text,
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                xPos,
                startY + idx * (BUTTON_HEIGHT + BUTTON_SPACING),
                BUTTON_WIDTH,
                BUTTON_HEIGHT,
                hWnd,
                (HMENU)(INT_PTR)id,
                g_hInstance,
                nullptr
            );
            if (g_hCustomFont && hBtn) {
                SendMessage(hBtn, WM_SETFONT, (WPARAM)g_hCustomFont, TRUE);
            }
            SetWindowTheme(hBtn, L"", L"");
            SetWindowSubclass(hBtn, HoverBtnSubProc, 1, 0);
            };
        makeBtn(L"Play", ID_BUTTON_PLAY, 0);
        const wchar_t* cseText = (g_csLaunchMode == ConstructionSetLaunchMode::Batch) ? L"Launch CSE" : L"Launch CS";
        g_hLaunchCseButton = CreateWindowW(
            L"BUTTON",
            cseText,
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            xPos - (CSE_BUTTON_EXTRA_WIDTH / 2),
            startY + 1 * (BUTTON_HEIGHT + BUTTON_SPACING),
            BUTTON_WIDTH + CSE_BUTTON_EXTRA_WIDTH,
            BUTTON_HEIGHT,
            hWnd,
            (HMENU)(INT_PTR)ID_BUTTON_LAUNCH_CSE,
            g_hInstance,
            nullptr
        );
        if (g_hCustomFont && g_hLaunchCseButton) {
            SendMessage(g_hLaunchCseButton, WM_SETFONT, (WPARAM)g_hCustomFont, TRUE);
        }
        SetWindowTheme(g_hLaunchCseButton, L"", L"");
        SetWindowSubclass(g_hLaunchCseButton, HoverBtnSubProc, 1, 0);
        if (g_csLaunchMode == ConstructionSetLaunchMode::None && g_hLaunchCseButton) {
            EnableWindow(g_hLaunchCseButton, FALSE);
        }

        makeBtn(L"Options", ID_BUTTON_OPTIONS, 2);
        makeBtn(L"Data Files", ID_BUTTON_DATAFILES, 3);
        makeBtn(L"Support", ID_BUTTON_TECHSUPPORT, 4);
        makeBtn(L"Exit", ID_BUTTON_EXIT, 5);
        return 0;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_BUTTON_PLAY:
            LaunchObseLoader();
            return 0;
        case ID_BUTTON_LAUNCH_CSE:
            LaunchConstructionSet();
            return 0;
        case ID_BUTTON_OPTIONS:
            OpenOptionsWindow(hWnd);
            return 0;
        case ID_BUTTON_DATAFILES:
            OpenDataFilesWindow(hWnd);
            return 0;
        case ID_BUTTON_TECHSUPPORT:
            ShellExecute(nullptr, L"open", L"https://next.nexusmods.com/profile/DaggerfallTeam", nullptr, nullptr, SW_SHOWNORMAL);
            return 0;
        case ID_BUTTON_EXIT:
            PostQuitMessage(0);
            return 0;
        }
        break;
    }
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* ds = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        if (ds->CtlType == ODT_BUTTON) {
            DrawButtonItem(ds);
            return TRUE;
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hWnd, &ps);
        if (g_pBackground) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            int ww = rc.right - rc.left;
            int hh = rc.bottom - rc.top;
            Graphics gx(dc);
            gx.DrawImage(g_pBackground, (REAL)0, (REAL)0, (REAL)ww, (REAL)hh);
        }
        else {
            FillRect(dc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow) {
    g_hInstance = hInst;
    GdiplusStartupInput gdipInput;
    GdiplusStartup(&g_gdiplusToken, &gdipInput, nullptr);

    g_pBackground = LoadPNGFromResource(IDR_BACKGROUND_PNG);
    g_pButtonBg = LoadPNGFromResource(IDR_BUTTON_PNG);

    const wchar_t clsName[] = L"ModernOblivionLauncher";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = clsName;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LAUNCHER_ICON));
    RegisterClass(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    HWND hWnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        clsName,
        L"Oblivion-Style Launcher",
        WS_POPUP,
        (sw - WINDOW_WIDTH) / 2,
        (sh - WINDOW_HEIGHT) / 2,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        nullptr,
        nullptr,
        hInst,
        nullptr
    );
    if (!hWnd) {
        MessageBox(nullptr, L"Window creation failed!", L"Error", MB_ICONERROR);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_pBackground) { delete g_pBackground; g_pBackground = nullptr; }
    if (g_pButtonBg) { delete g_pButtonBg;   g_pButtonBg = nullptr; }
    if (g_hCustomFont) { DeleteObject(g_hCustomFont); g_hCustomFont = nullptr; }

    if (!g_fontFilePath.empty()) {
        RemoveFontResourceExW(g_fontFilePath.c_str(), FR_PRIVATE, nullptr);
        g_fontFilePath.clear();
        g_fontFaceName.clear();
    }

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}
