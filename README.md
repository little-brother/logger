Logger is a proof-of-concept tool to debug. The main idea is: the app send a message to logger before execution of a suspicious code and send an empty message when the code finishes. The logger waits the second message and if it doesn't come during 200ms, the logger writes the original message into an output file. So only uncompleted operations will be logged to the file. The logger doesn't have UI except an icon in a tray.

### How it works
Before sending messages to a logger, an app should find a window of the logger by `logger-logger-class`. After that the app can send strings to the logger by `WM_COPYDATA`.<br>
```
hLoggerWnd = FindWindow(TEXT("logger-logger-class"), NULL);

if (hLoggerWnd == 0)
	return;

COPYDATASTRUCT data = {0};
data.dwData = <ID>;
data.cbData = <data length in bytes>;
data.lpData = <UTF16LE-string aka wchar_t>;
SendMessage(hLoggerWnd, WM_COPYDATA, (WPARAM)hAppWnd, (LPARAM)&data);
```
Messages with different ids are processed separately.

### How to build by mingw64
```
set PATH=%drive%:\mingw64\mingw64\bin;%PATH%
gcc -Wl,--kill-at -static ./src/main.c -o logger.exe -m64 -mwindows -s -Os -lshlwapi 
```