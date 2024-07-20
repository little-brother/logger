#define UNICODE
#define _UNICODE

#define IDM_EXIT         1000
#define IDM_ABOUT        1001
#define IDI_ICON         2000
#define IDT_TIMER        3000
#define WMU_SHELLNOTIFY  WM_USER + 1
#define WMU_SHOWBALLON   WM_USER + 2

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <shellapi.h>

LRESULT CALLBACK cbMainWindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc = {0};
	wc.lpfnWndProc = cbMainWindow;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("logger-logger-class");
	wc.style = CS_DBLCLKS;
	wc.cbSize = sizeof (WNDCLASSEX);
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindowEx(0, TEXT("logger-logger-class"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)
		return 0;

	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_ICON;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WMU_SHELLNOTIFY;
	nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	_tcscpy(nid.szTip, TEXT("Logger"));
	Shell_NotifyIcon(NIM_ADD, &nid);

	HMENU hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, IDM_ABOUT, TEXT("About"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, IDM_EXIT, TEXT("Exit"));
	SetProp(hWnd, TEXT("MENU"), hMenu);
	
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK cbMainWindow(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_DESTROY: {
			HMENU hMenu = GetProp(hWnd, TEXT("MENU"));
			DestroyMenu(hMenu);

			NOTIFYICONDATA nid = {0};
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = hWnd;
			nid.uID = IDI_ICON;
			Shell_NotifyIcon(NIM_DELETE, &nid);

			PostQuitMessage(0);
			return 0;
		}
		break;

		case WMU_SHELLNOTIFY: {
			if (wParam == IDI_ICON && lParam == WM_RBUTTONDOWN) {
				POINT p;
				GetCursorPos(&p);
				TrackPopupMenu(GetProp(hWnd, TEXT("MENU")), TPM_RIGHTALIGN, p.x, p.y, 0, hWnd, 0);
			}

			if (wParam == IDI_ICON && lParam == WM_LBUTTONDBLCLK) {
				SHELLEXECUTEINFO sei = {0};
				sei.cbSize = sizeof(SHELLEXECUTEINFO);
				sei.lpFile = TEXT("logger.txt");
				sei.lpVerb = TEXT("open");
				sei.nShow = SW_SHOW;
				ShellExecuteEx(&sei);
			}
		}
		break;

		// wParam = message
		case WMU_SHOWBALLON: {
			NOTIFYICONDATA nid = {0};
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = hWnd;
			nid.uID = IDI_ICON;
			nid.uFlags = NIF_INFO;
			_tcscpy(nid.szInfoTitle, TEXT("Logger"));
			_tcsncpy(nid.szInfo, (TCHAR*)wParam, 255);
			nid.dwInfoFlags = NIIF_INFO;
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		}
		break;

		case WM_COMMAND: {
			if (wParam == IDM_EXIT) 
				SendMessage(hWnd, WM_DESTROY, 0, 0);

			if (wParam == IDM_ABOUT) 
				SendMessage(hWnd, WMU_SHOWBALLON, (WPARAM)TEXT("The simple logger\nhttps://github.com/little-brother/logger"), 0);
		}
		break;

		case WM_TIMER: {
			TCHAR key16[128];
			_sntprintf(key16, 127, TEXT("TIMER%i"), wParam - IDT_TIMER);
			TCHAR* msg16 = (TCHAR*)GetProp(hWnd, key16);
			if (msg16) {
				SendMessage(hWnd, WMU_SHOWBALLON, (WPARAM)msg16, 0);

				FILE* f = _tfopen (TEXT("logger.txt"), TEXT("a+"));
				if (f) {
					int len = WideCharToMultiByte(CP_UTF8, 0, msg16, -1, NULL, 0, 0, 0);
					char* msg8 = (char*)calloc (len, sizeof(char));
					WideCharToMultiByte(CP_UTF8, 0, msg16, -1, msg8, len, 0, 0);
					fwrite(msg8, 1, strlen(msg8) + 1, f);
					free(msg8);
					fclose(f);				
				}
				free(msg16);
			}
			RemoveProp(hWnd, key16);
		}
		break;
	
		case WM_COPYDATA: {
			COPYDATASTRUCT* data = (COPYDATASTRUCT*)lParam;

			int id = (int)data->dwData;
			TCHAR key16[128];
			_sntprintf(key16, 127, TEXT("TIMER%i"), id);
			TCHAR* msg16 = (TCHAR*)GetProp(hWnd, key16);
			if (msg16)
				free(msg16);
			RemoveProp(hWnd, key16);
			KillTimer (hWnd, IDT_TIMER + id);

			if (data->lpData) {
				int len = _tcslen(data->lpData) + 256;
				msg16 = calloc(len + 1, sizeof(TCHAR));
				_sntprintf(msg16, len, TEXT("ID: %i\n%ls\n"), id, (TCHAR*)data->lpData);
				SetProp(hWnd, key16, (HANDLE)msg16);

				SetTimer(hWnd, IDT_TIMER + id, 200, NULL);
			} 
		}
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}