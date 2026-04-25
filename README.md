# processFKer

一个简单易用的 Manual Map 注入工具。

## 编译

使用 CMake 编译：
```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 使用方法

在命令行下执行：
```cmd
Injector-x64.exe <dll_path> [process_name]
```
- `<dll_path>`: 你要注入的 DLL 的绝对或相对路径
- `[process_name]`: （可选）目标进程的名称（例如：`csgo.exe`）。如果想之后再配置也可以不在命令行指定。
