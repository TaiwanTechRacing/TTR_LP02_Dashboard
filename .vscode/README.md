# VS Code Build And Debug

配置方式與 `TTR_LP02_VCU` 相同,兩個專案的 task 名稱、preset 名稱、
資料夾結構都刻意保持一致。

## Clone

LVGL 是 submodule(釘在 v9.4.0),clone 時要一起拉下來:

```powershell
git clone --recurse-submodules https://github.com/TaiwanTechRacing/TTR_LP02_Dashboard.git
```

已經 clone 過但 `Drivers/lvgl` 是空的:

```powershell
git submodule update --init
```

沒做這一步的話,CMake 會因為找不到任何 LVGL 原始碼而建置失敗。

## Required Commands

These must work in PowerShell:

```powershell
cmake --version
ninja --version
```

The ARM GCC toolchain is discovered by `cmake/arm-none-eabi-gcc.cmake`.

Discovery order:

1. `PATH`
2. `ARM_NONE_EABI_TOOLCHAIN_PATH`
3. STM32CubeIDE installs under `C:\ST`

## Build

VS Code task:

```text
STM32: build debug
```

Command line:

```powershell
cmake --preset debug
cmake --build --preset debug
```

Output:

```text
build/debug/DashBoard_re_1010.elf
```

Force rebuild:

```text
STM32: clean build debug
```

## Flash

VS Code task:

```text
STM32: flash debug
```

The script auto-discovers `STM32_Programmer_CLI.exe` from:

1. `PATH`
2. `STM32CUBEPROGRAMMER_PATH`
3. Common ST install paths

## Debug

VS Code Run and Debug:

```text
STM32: debug DashBoard
```

Attach without reset/build:

```text
STM32: attach DashBoard
```

Registers, RAM view, variables, breakpoints, and Live Watch are handled by
Cortex-Debug.

## CubeMX

VS Code task:

```text
STM32: open CubeMX
```

The script opens `DashBoard_re_1010.ioc` and auto-discovers `STM32CubeMX.exe`.

**改 .ioc 之前請先看 `Core/User/bsp_sdram.h` 開頭的腳位表。** FMC / SDRAM
不在 .ioc 裡面,CubeMX 不知道那些腳已經被佔用,重新產生程式碼時可能會把
它們配給別的周邊。

## 與 STM32CubeIDE 的關係

`.cproject` / `.project` 保留著,CubeIDE 仍然可以開這個專案,兩套建置系統
可以並存:

| | 輸出位置 | 優化等級 |
|---|---|---|
| CMake / VS Code | `build/debug/` | `-O2` |
| STM32CubeIDE | `Debug/` | `-O2` |

兩邊的編譯選項刻意設成一致,燒進去的行為才會一樣。

一個差異值得知道:CMake 只收 `Drivers/lvgl/src/`,而 CubeIDE 是整個
`Drivers/` 遞迴掃描,會連 `Drivers/lvgl/tests/` 一起編 —— 那些檔案是給桌面
環境用的,這就是 CubeIDE 的 Release 組態一直建置失敗的原因。CMake 這邊
沒有這個問題,Debug 和 Release 都能建。
