#include <windows.h>

#define MAX_LEN          _MAX_PATH
#define WM_NOTIFICATION  (WM_APP + 100)
#define WM_QUIT_MENU     (WM_APP + 200)
#define WM_RELOAD_MENU   (WM_APP + 300)
#define WM_CONFIG_MENU   (WM_APP + 400)
#define WM_APP_MENU      (WM_APP + 500)
#define IDENT            "_SAL_IDENT_"
#define APP_TITLE        "Simple Launcher"
#define CMDS_FILE        "cmds.txt"
