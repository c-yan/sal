#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <imm.h>
#include "sal.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static HANDLE hMutex;
static HICON hIcon;
static HMENU hLMenu, hRMenu;
static char **cmds;
static char **params;

static void chop(char s[]) {
    int i = strlen(s) - 1;
    if (s[i] == '\n') {
        s[i] = '\0';
    }
}

static int line_count(FILE *fp) {
    int i;
    char s[MAX_LEN];

    fseek(fp, 0, SEEK_SET);
    i = 0;
    while (fgets(s, MAX_LEN, fp) != 0) {
        i++;
    }
    fseek(fp, 0, SEEK_SET);
    return i;
}

static int read_cmds(void)
{
    int i;
    FILE *fp;
    char s[MAX_LEN], *p, caption[_MAX_PATH];
    HMENU hPopup;

    if ((fp = fopen(CMDS_FILE, "r")) == NULL) {
        return -1;
    }

    i = line_count(fp);
    cmds = (char**)calloc(i, sizeof(char *));
    params = (char**)calloc(i, sizeof(char *));

    i = 0;
    hPopup = NULL;
    while (fgets(s, MAX_LEN, fp) != 0) {
        if (s[0] == ';') continue;
        chop(s);
        p = strtok(s, "\t");
        if (p[0] == '*') {
            if (hPopup != NULL) {
                AppendMenu(hLMenu, MF_STRING | MF_POPUP, (UINT)hPopup, caption);
            }
            p = strtok(NULL, "\t");
            if (p == NULL) {
                hPopup = NULL;
            } else {
                strcpy(caption, p);
                hPopup = CreatePopupMenu();
            }
        } else {
            if (hPopup == NULL) {
                AppendMenu(hLMenu, MF_STRING, WM_APP_MENU + i, p);
            } else {
                AppendMenu(hPopup, MF_STRING, WM_APP_MENU + i, p);
            }
            p = strtok(NULL, "\t");
            cmds[i] = (char *)malloc(strlen(p) + 1);
            strcpy(cmds[i], p);
            p = strtok(NULL, "\t");
            if (p != NULL) {
                params[i] = (char *)malloc(strlen(p) + 1);
                strcpy(params[i], p);
            }
        }
        i++;
    }
    if (hPopup != NULL) {
        AppendMenu(hLMenu, MF_STRING | MF_POPUP, (UINT)hPopup, caption);
    }
    fclose(fp);
    return 0;
}

static void disable_ime(void)
{
    HMODULE hImm32;
    BOOL WINAPI (*pImmDisableIME)(DWORD);

    if ((hImm32 = LoadLibrary("imm32.dll")) == NULL) {
        return;
    }

    if ((pImmDisableIME = GetProcAddress(hImm32, "ImmDisableIME")) != NULL) {
        pImmDisableIME(0);
    }

    FreeLibrary(hImm32);
}

int WINAPI WinMain(HINSTANCE hIns, HINSTANCE hPrevIns, LPSTR lpszArgv, int nDefaultWindowMode)
{
    HWND hWnd;
    WNDCLASSEX wcl;
    MSG Msg;
    TIMECAPS tc;

    if ((hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, IDENT)) != NULL) {
        CloseHandle(hMutex);
        return 1;
    }
    hMutex = CreateMutex(FALSE, 0, IDENT);

    timeGetDevCaps(&tc , sizeof(TIMECAPS));
    timeBeginPeriod(tc.wPeriodMin);
    disable_ime();

    hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcl.hInstance = hIns;
    wcl.lpszClassName = IDENT;
    wcl.lpfnWndProc = WndProc;
    wcl.style = 0;
    wcl.hIcon = wcl.hIconSm = hIcon;
    wcl.hCursor = LoadCursor(NULL, IDI_APPLICATION);
    wcl.lpszMenuName = NULL;
    wcl.cbClsExtra = wcl.cbWndExtra = 0;
    wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcl.cbSize = sizeof(WNDCLASSEX);

    if (RegisterClassEx(&wcl) == 0) {
        return 1;
    }

    hWnd = CreateWindow(
        IDENT,
        NULL,
        WS_OVERLAPPEDWINDOW,
        GetSystemMetrics(SM_CXSCREEN) / 2,
        GetSystemMetrics(SM_CYSCREEN) / 2,
        0,
        0,
        NULL,
        NULL,
        hIns,
        NULL
    );

    ShowWindow(hWnd, SW_HIDE);

    while(GetMessage(&Msg, NULL, 0, 0)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    timeEndPeriod(tc.wPeriodMin);

    return Msg.wParam;
}

static void extractpath(char *result, char *path)
{
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];

    _splitpath(path, drive, dir, fname, ext);
    _makepath(result, drive, dir, "", "");
}

static void exec_cmd(int i)
{
    char cmd[_MAX_PATH], path[_MAX_PATH];

    _fullpath(cmd, cmds[i], _MAX_PATH);
    extractpath(path, cmd);
    ShellExecute(NULL, NULL, cmd, params[i], path, SW_SHOW);
}

static void add_icon(PNOTIFYICONDATA lpdata)
{
    while (Shell_NotifyIcon(NIM_ADD, lpdata) == FALSE) {
        Sleep(1000);
        if (Shell_NotifyIcon(NIM_MODIFY, lpdata) != FALSE) break;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    HMENU hMenu;
    static NOTIFYICONDATA nIcon;
    static UINT WM_TASKBAR_CREATED;

    switch (uMsg) {
    case WM_NOTIFICATION:
        hMenu = 0;
        if (lParam == WM_LBUTTONUP) {
            hMenu = hLMenu;
        } else if (lParam == WM_RBUTTONUP) {
            hMenu = hRMenu;
        }
        if (hMenu != 0) {
            SetForegroundWindow(hWnd);
            GetCursorPos(&pt);
            TrackPopupMenu(hMenu, TPM_RIGHTALIGN, pt.x, pt.y, 0, hWnd, NULL);
            PostMessage(hWnd, WM_NULL, 0, 0);
        }
        break;
    case WM_COMMAND:
        if(LOWORD(wParam) == WM_QUIT_MENU) {
            SendMessage(hWnd, WM_DESTROY, 0, 0);
        } else if(LOWORD(wParam) == WM_RELOAD_MENU) {
            DestroyMenu(hLMenu);
            hLMenu = CreatePopupMenu();
            // TODO: free cmds/params
            read_cmds();
        } else if(LOWORD(wParam) == WM_CONFIG_MENU) {
		    ShellExecute(NULL, NULL, CMDS_FILE, NULL, NULL, SW_SHOW);
        } else {
            exec_cmd(LOWORD(wParam) - WM_APP_MENU);
        }
        SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
        break;
    case WM_CREATE:
        WM_TASKBAR_CREATED = RegisterWindowMessage("TaskbarCreated");
        nIcon.cbSize = sizeof(NOTIFYICONDATA);
        nIcon.hWnd = hWnd;
        nIcon.uID = 1;
        nIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nIcon.uCallbackMessage = WM_NOTIFICATION;
        nIcon.hIcon = (HICON)CopyImage(hIcon, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_COPYFROMRESOURCE);
        strcpy(nIcon.szTip, APP_TITLE);
        add_icon(&nIcon);
        hLMenu = CreatePopupMenu();
        read_cmds();
        hRMenu = CreatePopupMenu();
        AppendMenu(hRMenu, MF_STRING, WM_CONFIG_MENU, "Config");
        AppendMenu(hRMenu, MF_STRING, WM_RELOAD_MENU, "Reload");
        AppendMenu(hRMenu, MF_STRING, WM_QUIT_MENU, "Quit");
        break;
    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nIcon);
        DestroyIcon(nIcon.hIcon);
        DestroyMenu(hLMenu);
        DestroyMenu(hRMenu);
        ReleaseMutex(hMutex);
        PostQuitMessage(0);
        break;
    default:
        if(uMsg == WM_TASKBAR_CREATED) {
            add_icon(&nIcon);
        }
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}
