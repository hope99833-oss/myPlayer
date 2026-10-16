# myPlayer

基于 Qt + FFmpeg + SDL2 的视频播放器。

## 开发环境

| 项目 | 版本 |
|---|---|
| Visual Studio | 2026（平台工具集 v145） |
| Qt | 5.15.2 MSVC2026 64bit（Qt VS Tools） |
| FFmpeg | 共享库构建（avcodec-63 / avformat-63 / avutil-61 / avfilter-12 / avdevice-63 / swresample-7 / swscale-10） |
| SDL2 | 2.x |

## 目录结构

```
myPlayer/
├── .gitignore
├── player/
│   ├── player.slnx              # 解决方案
│   └── player/
│       ├── main.cpp / player.cpp / player.h / player.ui / player.qrc
│       ├── player.vcxproj(.filters)
│       ├── include/             # 第三方头文件（已提交）
│       │   ├── libavcodec/ libavdevice/ libavfilter/ libavformat/
│       │   ├── libavutil/ libswresample/ libswscale/   # FFmpeg
│       │   └── sdl2/                                    # SDL2
│       ├── lib/                 # 导入库（*.lib / *.dll.a 未提交，需自行放置）
│       └── bin/                 # 运行时 DLL / 工具（未提交，需自行放置）
```

## 第三方依赖放置位置

> 以下二进制文件被 `.gitignore` 忽略，**换机克隆后需自行恢复**。

| 依赖 | 头文件 | 链接库 | 运行时 DLL |
|---|---|---|---|
| FFmpeg | `player/player/include/libav*` | `player/player/lib/{avcodec,avdevice,avfilter,avformat,avutil,swresample,swscale}.lib` | `player/player/bin/{avcodec-63,avdevice-63,avfilter-12,avformat-63,avutil-61,swresample-7,swscale-10}.dll` |
| SDL2 | `player/player/include/sdl2` | `player/player/lib/SDL2.lib` | `player/player/bin/SDL2.dll` |

`bin/` 下还有 `ffmpeg.exe`、`ffplay.exe`、`ffprobe.exe` 三个命令行工具（播放器本身不依赖）。

## 构建配置

Debug / Release 两个配置均已配好：

- **头文件目录**：`$(ProjectDir)include`（FFmpeg）+ `$(ProjectDir)include\sdl2`（SDL2）
- **库目录**：`$(ProjectDir)lib`
- **链接库**：
  - FFmpeg：`avcodec.lib; avdevice.lib; avfilter.lib; avformat.lib; avutil.lib; swresample.lib; swscale.lib`
  - SDL2：`SDL2.lib`（**不链接** `SDL2main.lib`，已定义 `SDL_MAIN_HANDLED`，由 Qt 提供程序入口）
- **构建后事件**：自动把 `bin\*.dll` 拷贝到输出目录（`$(OutDir)`），保证 DLL 与 exe 同目录。

## 构建方式

用 Visual Studio 打开 `player/player.slnx`，选择 Debug|x64 或 Release|x64 直接生成即可。首次生成前请确保上述第三方库/DLL 已放回对应目录。
