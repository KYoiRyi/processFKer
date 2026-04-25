# processFKer

A reliable and easy-to-use Manual Map Injector for Windows.

## Advanced Usage

`FKer.exe` supports multiple operating modes to fit your workflow perfectly:

### Standard Injection (Running Process)
Inject into an already running process:
```cmd
FKer.exe <dll_path> [process_name]
```
*Example: `FKer.exe cheat.dll csgo.exe`*

### Launch & Inject Mode (Suspended Injection)
Launch an executable in a suspended state, inject the DLL, and then resume the main thread. This is highly useful for initializing your hooks before any original game code bounds are executed.
```cmd
FKer.exe <dll_path> -launch <exe_path>
```
*Example: `FKer.exe cheat.dll -launch "C:\Games\game.exe"`*

### UI / Interactive Mode
Simply run or drag-and-drop the DLL onto `FKer.exe`. The application will interactively prompt you for either a process name or a launch path. 

## Building from source
This project can be easily compiled via CMake:
```cmd
mkdir build
cd build
cmake .. -A x64
cmake --build . --config Release
```
