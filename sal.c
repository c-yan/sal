#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "sal.h"

#ifdef PERIOD
#include <mmsystem.h>
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HANDLE hMutex;
HICON hIcon;
HMENU hMenu;
char **cmds;

static void chop(char s[]){
	int i = strlen(s) - 1;
	if (s[i] == '\n'){
		s[i] = '\0';
	}
}

static int read_cmds(void)
{
	int i;
	FILE *fp;
	char s[MAX_LEN], *p, caption[FILENAME_MAX];
	HMENU hPopup;
	
	if ((fp = fopen(CMDS_FILE, "r")) == NULL){
		return -1;
	}
	
	i = 0;
	while (fgets(s, MAX_LEN, fp) != 0){
		i++;
	}
	cmds = (char**)malloc(sizeof(char *) * i);
	
	fseek(fp, 0, SEEK_SET);
	i = 0;
	hPopup = NULL;
	while (fgets(s, MAX_LEN, fp) != 0){
		chop(s);
		p = strtok(s, "\t");
		if (p[0] == '*'){
			if (hPopup != NULL){
				AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hPopup, caption);
			}
			p = strtok(NULL, "\t");
			if (p == NULL){
				hPopup = NULL;
			} else {
				strcpy(caption, p);
				hPopup = CreatePopupMenu();
			}
		} else {
			if (hPopup == NULL){
				AppendMenu(hMenu, MF_STRING, WM_APP_MENU + i, p);
			} else {
				AppendMenu(hPopup, MF_STRING, WM_APP_MENU + i, p);
			}
			p = strtok(NULL, "\t");
			cmds[i] = (char *)malloc(strlen(p) + 1);
			strcpy(cmds[i], p);
		}
		i++;
	}
	if (hPopup != NULL){
		AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hPopup, caption);
	}
	fclose(fp);
	return 0;
}

int WINAPI WinMain(HINSTANCE hIns, HINSTANCE hPrevIns, LPSTR lpszArgv, int nDefaultWindowMode)
{
	HWND hWnd;
	WNDCLASSEX wcl;
	MSG Msg;
	
	if ((hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, IDENT)) != NULL) {
		CloseHandle(hMutex);
		return 1;
	}
	hMutex = CreateMutex(FALSE, 0, IDENT);
	
#ifdef PERIOD
	timeBeginPeriod(1);
#endif
	
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
	
	hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		IDENT,
		NULL,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		HWND_DESKTOP,
		NULL,
		hIns,
		NULL
	);
	
	ShowWindow(hWnd, SW_HIDE);
	//ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	
	while(GetMessage(&Msg, NULL, 0, 0)){
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	
#ifdef PERIOD
	timeEndPeriod(1);
#endif
	
	return Msg.wParam;
}

static void extract_filepath(char *s)
{
	int i;
	for(i = strlen(s); i > 0; i--){
		if (s[i] == '\\') break;
	}
	s[i + 1] = '\0';
}

static void exec_cmd(int i)
{
	STARTUPINFO SI;
	PROCESS_INFORMATION PI;
	char s[FILENAME_MAX];
	
	strcpy(s, cmds[i]);
	extract_filepath(s);
	//MessageBox(NULL, s, "", MB_OK);
	GetStartupInfo(&SI);
	CreateProcess(cmds[i], "", NULL, NULL, FALSE, 0, NULL, s, &SI, &PI);
	//MessageBox(NULL, cmds[i], "", MB_OK);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT p;
	static NOTIFYICONDATA nIcon;
	static UINT WM_TASKBAR_CREATED;
	
	switch (uMsg){
	case WM_TASKTRAY:
		if(lParam == WM_LBUTTONDOWN){
			SetForegroundWindow(hWnd);
			GetCursorPos(&p);
			TrackPopupMenu(hMenu, TPM_RIGHTALIGN, p.x, p.y, 0, hWnd, NULL);
		}
		break;
	case WM_COMMAND:
		if(LOWORD(wParam) == WM_QUIT_MENU){
			SendMessage(hWnd, WM_DESTROY, 0, 0);
		} else {
			exec_cmd(LOWORD(wParam) - WM_APP_MENU);
		}
		break;
	case WM_CREATE:
		WM_TASKBAR_CREATED = RegisterWindowMessage("TaskbarCreated");
		nIcon.cbSize = sizeof(NOTIFYICONDATA);
		nIcon.hWnd = hWnd;
		nIcon.uID = 1;
		nIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nIcon.uCallbackMessage = WM_TASKTRAY;
		nIcon.hIcon = (HICON)CopyImage(hIcon, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_COPYFROMRESOURCE);
		strcpy(nIcon.szTip, APP_TITLE);
		Shell_NotifyIcon(NIM_ADD, &nIcon);
		hMenu = CreatePopupMenu();
		read_cmds();
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hMenu, MF_STRING, WM_QUIT_MENU, "Quit");
		SetMenu(hWnd, hMenu);
		break;
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &nIcon);
		DestroyIcon(nIcon.hIcon);
		DestroyMenu(hMenu);
		ReleaseMutex(hMutex);
		PostQuitMessage(0);
		break;
	default:
		if((WM_TASKBAR_CREATED != 0) && (uMsg == WM_TASKBAR_CREATED)){
			Shell_NotifyIcon(NIM_ADD, &nIcon);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}
