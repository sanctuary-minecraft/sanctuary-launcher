// SPDX-License-Identifier: GPL-3.0-only
#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>
#include <wchar.h>

static BOOL valid_version(const wchar_t* version)
{
    int separators = 0;
    BOOL digit = FALSE;
    for (; *version; ++version) {
        if (*version >= L'0' && *version <= L'9') {
            digit = TRUE;
        } else if (*version == L'.' && digit && separators < 2) {
            ++separators;
            digit = FALSE;
        } else {
            return FALSE;
        }
    }
    return digit && separators == 2;
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR arguments, int show)
{
    (void)instance;
    (void)previous;
    (void)show;
    wchar_t directory[MAX_PATH];
    const DWORD length = GetModuleFileNameW(NULL, directory, ARRAYSIZE(directory));
    if (length == 0 || length >= ARRAYSIZE(directory)) return 1;
    wchar_t* filename = wcsrchr(directory, L'\\');
    if (!filename) return 1;
    *filename = L'\0';

    wchar_t settings[MAX_PATH];
    wchar_t executable[MAX_PATH];
    wchar_t version[64];
    if (FAILED(StringCchPrintfW(settings, ARRAYSIZE(settings), L"%ls\\current.ini", directory))) return 1;
    const DWORD versionLength = GetPrivateProfileStringW(L"Launcher", L"Version", L"", version, ARRAYSIZE(version), settings);
    if (versionLength > 0 && versionLength < ARRAYSIZE(version) - 1 && valid_version(version)
        && SUCCEEDED(StringCchPrintfW(executable, ARRAYSIZE(executable), L"%ls\\versions\\%ls\\sanctuary-launcher.exe", directory, version))
        && (INT_PTR)ShellExecuteW(NULL, L"open", executable, arguments, directory, SW_SHOWNORMAL) > 32) {
        return 0;
    }

    const BOOL french = PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_FRENCH;
    MessageBoxW(NULL,
                french ? L"Le Launcher Sanctuary ne démarre pas. Relancez son installateur."
                       : L"Sanctuary Launcher could not start. Run its installer again.",
                L"Sanctuary Launcher", MB_OK | MB_ICONERROR);
    return 1;
}
