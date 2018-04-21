#include <windows.h>

#define MAX_LEN       (FILENAME_MAX * 2)
#define WM_TASKTRAY   (WM_APP + 100)
#define WM_QUIT_MENU  (WM_APP + 200)
#define WM_APP_MENU   (WM_APP + 300)
#define IDENT         "_SAL_IDENT_"
#define APP_TITLE     "Simple Application Launcher"
#define CMDS_FILE     "cmds.txt"
