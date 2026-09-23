# GameSubtitles 插件

## 1. 插件用途与适用场景

本插件提供一套**游戏内字幕（Subtitle）的播放与显示方案**，主要面向**过场动画 / 媒体视频**的字幕需求。

它补上了引擎自带字幕系统缺的两块：

1. **字幕播放器** —— 能跟着媒体播放进度，从字幕资源里查出"此刻该显示哪条字幕"
2. **可配置的字幕显示控件** —— 字号、颜色、描边、背景透明度都能分档配置，并能在运行时全局切换

**要解决的问题：**
引擎自带的 `FSubtitleManager` 只负责"把文本显示出来"，但**不负责**"什么时候该显示哪条"。而 UMediaPlayer 播放视频时也没有现成的字幕轨道支持。本插件把这两者之间的桥接补上了。

**典型适用场景：**

- 播放过场动画视频（MediaPlayer）时需要同步字幕
- 需要玩家在设置里调整字幕的字号 / 颜色 / 背景
- 需要字幕支持富文本标签

**不适用：** 对话系统的字幕（那种一般走 Dialogue 插件或直接 UI 逻辑），本插件是围绕"媒体时间轴"设计的。

---

## 2. 目录结构

```
GameSubtitles/
├── GameSubtitles.uplugin
├── README.md
├── Config/
│   └── Tags/                              # 插件自带的 GameplayTag 配置（启动时注册）
└── Source/
    ├── GameSubtitles.Build.cs
    ├── Public/
    │   ├── SubtitleDisplayOptions.h        # 样式配置数据资产 + 4 个档位枚举
    │   ├── SubtitleDisplaySubsystem.h      # 字幕设置子系统 + FSubtitleFormat
    │   ├── Players/
    │   │   └── MediaSubtitlesPlayer.h      # 字幕播放器（驱动端）
    │   └── Widgets/
    │       ├── SubtitleDisplay.h           # UMG 字幕控件（蓝图用）
    │       └── SSubtitleDisplay.h          # Slate 字幕控件（底层）
    └── Private/
        ├── GameSubtitlesModule.cpp         # 模块入口（注册 Tag 目录）
        ├── SubtitleDisplaySubsystem.cpp
        ├── Players/MediaSubtitlesPlayer.cpp
        └── Widgets/
            ├── SubtitleDisplay.cpp
            └── SSubtitleDisplay.cpp
```

---

## 3. 核心类 / 模块及其职责

### 3.1 模块

| 模块名 | 类型 | 职责 |
|---|---|---|
| `GameSubtitles` | Runtime | `StartupModule` 里把 `Config/Tags` 注册为 GameplayTag 的 ini 搜索路径——**这是必需的**，否则插件自带的标签不会被加载。`ShutdownModule` 为空。 |

### 3.2 数据与控制流

```
UMediaPlayer（视频播放）
      │  GetTime()  当前播放时间
      ▼
UMediaSubtitlesPlayer::Tick()
      │  每帧查询 SourceSubtitles (UOverlays)
      │  GetOverlaysForTime(CurrentTime)
      ▼
FSubtitleManager::SetMovieSubtitle(this, 文本数组)      ← 引擎全局字幕管理器
      │  OnSetSubtitleText 广播
      ▼
SSubtitleDisplay::HandleSubtitleChanged()   →  SRichTextBlock 显示文本
```

### 3.3 核心类一览

| 类 / 结构体 | 职责 |
|---|---|
| `UMediaSubtitlesPlayer` | **驱动端**。派生 `UObject` + `FTickableGameObject`，靠 Tick 推进。每帧按媒体播放器时间查字幕并推给 `FSubtitleManager`。`Play()` / `Stop()` 必须与媒体播放器的同名方法同时调用。 |
| `USubtitleDisplayOptions` | **样式配置资产**。把"档位 → 具体值"存成数组（字号数组、颜色数组、边框数组、透明度数组）。 |
| `FSubtitleFormat` | 玩家当前**选择的档位组合**（尺寸 / 颜色 / 描边 / 背景），不含具体数值。 |
| `USubtitleDisplaySubsystem` | GameInstance 子系统。保存当前 `FSubtitleFormat`，变化时广播 `DisplayFormatChangedEvent`。 |
| `USubtitleDisplay` | UMG 控件（蓝图用）。把"档位 + 选项资产"换算成 Slate 样式，包装底层控件。 |
| `SSubtitleDisplay` | 真正的 Slate 控件。订阅 `FSubtitleManager` 的 `OnSetSubtitleText` 显示字幕。 |

### 3.4 档位枚举

| 枚举 | 取值 |
|---|---|
| `ESubtitleDisplayTextSize` | ExtraSmall / Small / Medium / Large / ExtraLarge |
| `ESubtitleDisplayTextColor` | White / Yellow |
| `ESubtitleDisplayTextBorder` | None / Outline / DropShadow |
| `ESubtitleDisplayBackgroundOpacity` | Clear / Low / Medium / High / Solid |

这些枚举的值**直接用作 `USubtitleDisplayOptions` 里各数组的下标**。

---

## 4. 依赖的其他插件或模块

**插件依赖：** 无（`.uplugin` 的 Plugins 段为空）。

**模块依赖：**

| 模块 | 用途 |
|---|---|
| `Core` | 基础 |
| `Overlay` | `UOverlays` / `FOverlayItem`——字幕资源的数据结构 |
| `UMG` | 字幕控件的 UMG 包装 |
| `MediaAssets` | `UMediaPlayer`——字幕时间轴以它的播放时间为准 |
| `MediaUtils` | 媒体播放工具 |
| `GameplayTags` | 模块启动时注册插件自带标签目录 |
| `CoreUObject` / `Engine` / `Slate` / `SlateCore`（私有） | 常规依赖 |

---

## 5. 接入与使用要点

### 5.1 播放过场动画字幕

```cpp
// 1) 创建字幕播放器，设置字幕资源，绑定到媒体播放器
UMediaSubtitlesPlayer* SubPlayer = NewObject<UMediaSubtitlesPlayer>();
SubPlayer->SetSubtitles(MyOverlaysAsset);      // UOverlays 资产
SubPlayer->BindToMediaPlayer(MyMediaPlayer);   // UMediaPlayer

// 2) 播放时，务必与媒体播放器同时调用
MyMediaPlayer->OpenSource(MyMediaSource);
SubPlayer->Play();
MyMediaPlayer->Play();

// 3) 结束时同样要成对调用
SubPlayer->Stop();
MyMediaPlayer->Close();
```

**⚠️ 关键点：`Play()` / `Stop()` 必须与 `UMediaPlayer` 的 `Play()` / `Stop()` 在同一时刻调用**，否则字幕与画面会错位。

### 5.2 在 UI 上显示字幕

在需要显示字幕的界面里放一个 **SubtitleDisplay** 控件（UMG），指定：

- `Options`：指向一个 `USubtitleDisplayOptions` 数据资产
- `Format`：默认档位（运行时会被子系统的全局设置覆盖）
- `WrapTextAt`：超过该宽度自动换行，0 或负数表示不换行

### 5.3 让玩家在设置里改字幕样式

```cpp
USubtitleDisplaySubsystem* SubSys = USubtitleDisplaySubsystem::Get(LocalPlayer);

FSubtitleFormat NewFormat = SubSys->GetSubtitleDisplayOptions();
NewFormat.SubtitleTextSize = ESubtitleDisplayTextSize::Large;
SubSys->SetSubtitleDisplayOptions(NewFormat);   // 自动广播，所有字幕控件立即刷新
```

### 5.4 制作样式资产

创建一个继承自 `USubtitleDisplayOptions` 的数据资产，填写：

- `Font`：字幕字体
- `DisplayTextSizes[5]`：五个尺寸档位各自的字号
- `DisplayTextColors[2]`：两个颜色档位各自的颜色
- `DisplayBorderSize[3]`：三种描边档位各自的边框尺寸
- `DisplayBackgroundOpacity[5]`：五档背景不透明度
- `BackgroundBrush`：背景画刷

> 数组长度与枚举一一对应，下标即枚举值，填错长度会导致取值越界。

### 5.5 注意事项

1. **`UMediaSubtitlesPlayer` 靠 Tick 驱动**（`ETickableTickType::Always`），CDO 不 Tick。若媒体播放器已失效，Tick 里会自动 `Stop()`。
2. **销毁时会自动清空字幕**（`BeginDestroy` → `Stop()`），但仍建议在业务侧显式调用 `Stop()`。
3. **`ManualSubtitles` 参数**：为 true 时 `SSubtitleDisplay` 只显示手动设置的文本，忽略字幕管理器推来的自动字幕——做预览时很有用。
4. **析构必须取消订阅**：`SSubtitleDisplay` 在 `Construct` 里订阅了 `FSubtitleManager` 的委托，析构里 `RemoveAll(this)`，否则会回调到已销毁对象。
5. 本插件**不含内容资产**，所有字幕资源（`UOverlays`）与样式资产需自己在项目里创建。
