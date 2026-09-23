# CommonLoadingScreen 插件说明

> 本文为中文说明文档。插件源码中的注释也已补充为中文。

## 一、插件用途与适用场景

`CommonLoadingScreen` 解决的是"什么时候该显示加载界面"这个看似简单、实则很麻烦的问题。

难点在于：**"加载完了"这件事没有一个单一的判定点**。切关卡时在加载、流式加载贴图时在加载、你的某个异步子系统在初始化时也算在加载——这些系统彼此并不认识，谁也说不清"现在是不是全都好了"。

本插件给出的方案是 **"每帧轮询 + 谁都能举手"**：

- `ULoadingScreenManager` 每帧问一遍"现在该显示加载界面吗"
- 任何系统都可以实现 `ILoadingProcessInterface` 并注册进来，只要有一个说"我还在忙"，界面就不收
- 切关卡这种引擎层面的加载由管理器自己感知（`PreLoadMap` / `PostLoadMap`）

这样就不需要任何系统知道其它系统的存在。

另外它还提供了**引擎启动阶段**的加载画面（第二个模块），让游戏一启动就有画面，而不是先黑屏。

## 二、目录结构

```
Plugins/CommonLoadingScreen/
├── CommonLoadingScreen.uplugin
├── README.md
└── Source/
    ├── CommonLoadingScreen/                 【模块一】运行时加载界面（Default 阶段）
    │   ├── CommonLoadingScreen.Build.cs
    │   ├── Public/
    │   │   ├── LoadingScreenManager.h       加载界面总控（核心）
    │   │   ├── LoadingProcessInterface.h    "我还在忙"接口
    │   │   └── LoadingProcessTask.h/.cpp    蓝图可用的加载占位任务
    │   └── Private/
    │       ├── CommonLoadingScreenModule.cpp
    │       ├── CommonLoadingScreenSettings.h/.cpp   配置项（同时是控制台变量）
    │       └── LoadingScreenManager.cpp
    └── CommonStartupLoadingScreen/          【模块二】引擎启动画面（PreLoadingScreen 阶段）
        ├── CommonStartupLoadingScreen.Build.cs
        └── Private/
            ├── CommonStartupLoadingScreen.cpp     模块入口（注册启动画面）
            ├── CommonPreLoadScreen.h/.cpp         启动画面对象
            └── SCommonPreLoadingScreenWidget.h/.cpp  启动画面 Slate 控件（纯黑底）
```

## 三、核心类与职责

### 3.1 模块一：运行时加载界面

| 类 | 职责 |
|---|---|
| `ULoadingScreenManager` | **核心**。GameInstance 子系统 + 可 Tick 对象。每帧判断该显示还是该隐藏，并负责创建/销毁控件、屏蔽/恢复输入、调整性能设置 |
| `ILoadingProcessInterface` | "我还在忙，别收加载界面"接口。任何 UObject 都能实现并注册 |
| `ULoadingProcessTask` | 蓝图可用的加载占位任务——创建即占着，调 `Unregister()` 释放 |
| `UCommonLoadingScreenSettings` | 配置项。因为派生自 `UDeveloperSettingsBackedByCVars`，**每项同时也是控制台变量** |

### 3.2 模块二：引擎启动画面

| 类 | 职责 |
|---|---|
| `FCommonPreLoadScreen` | 引擎极早期（UObject/UMG 都还没就绪）显示的画面，只能用纯 Slate |
| `SCommonPreLoadingScreenWidget` | 启动画面的 Slate 控件（目前是纯黑底，可替换成自己的 Logo） |
| `FCommonStartupLoadingScreenModule` | 模块入口，负责把启动画面注册到 `FPreLoadScreenManager` |

### 3.3 两个模块的关系

```
游戏启动 ──► CommonStartupLoadingScreen（PreLoadingScreen 阶段，纯 Slate 黑底）
              │  引擎初始化完成
              ▼
           CommonLoadingScreen（Default 阶段，UMG 加载界面接管后续所有加载）
```

## 四、依赖关系

**插件依赖**：无（`Plugins` 列表为空）。

**模块一 `CommonLoadingScreen`**（Public 仅 Core）：

| 私有依赖 | 用途 |
|---|---|
| CoreUObject / Engine | 子系统、世界上下文 |
| Slate / SlateCore | 加载界面控件 |
| UMG | 加载界面用 UMG 用户控件 |
| InputCore | 加载期间屏蔽玩家输入 |
| PreLoadScreen | 与引擎的 PreLoadScreen 机制对接 |
| RenderCore | 加载期间的性能设置调整 |
| DeveloperSettings | 配置类需要 |

**模块二 `CommonStartupLoadingScreen`**（ClientOnly，PreLoadingScreen 阶段）：

| 私有依赖 | 用途 |
|---|---|
| CoreUObject / Engine | 基础 |
| Slate / SlateCore | 只能用纯 Slate（此时 UMG 还不可用） |
| MoviePlayer | 播放启动影片（预留能力） |
| PreLoadScreen | 引擎的早期加载画面机制 |
| DeveloperSettings | 配置读取 |

**被谁依赖**：本工程中被 Lyra 的经验（Experience）加载流程使用——加载 Experience 时会创建一个 `ULoadingProcessTask` 占住界面，加载完再释放。

## 五、接入与使用要点

### 5.1 基本配置

1. 在项目设置里找到 **Common Loading Screen**（或 `Config/DefaultGame.ini`）。
2. 把 `LoadingScreenWidget` 指向你的加载界面 UMG 控件类。
3. 其余参数按需调整（见下表）。

| 配置项 | 默认 | 说明 |
|---|---|---|
| `LoadingScreenWidget` | — | 加载界面控件类（软引用） |
| `LoadingScreenZOrder` | 10000 | 视口层级，默认保证盖在所有游戏 UI 之上 |
| `HoldLoadingScreenAdditionalSecs` | 2.0 | 加载完成后**额外多挂几秒**，给纹理流送时间，避免满屏模糊贴图 |
| `LoadingScreenHeartbeatHangDuration` | 0.0 | 超过多少秒没收掉就判定为"卡死"并报警（0 = 不检测） |
| `LogLoadingScreenHeartbeatInterval` | 5.0 | 每隔几秒打一次日志说明"谁在拖着界面不放" |
| `LogLoadingScreenReasonEveryFrame` | 0 | 每帧打印显示/隐藏原因（调试用，日志很吵） |
| `ForceLoadingScreenVisible` | false | 强制一直显示（调试用） |
| `HoldLoadingScreenAdditionalSecsEvenInEditor` | false | 编辑器里是否也应用"额外几秒" |
| `ForceTickLoadingScreenEvenInEditor` | true | 编辑器里是否也让管理器每帧 Tick |

### 5.2 C++ 里占住加载界面

```cpp
// 方式一：实现接口
class UMySubsystem : public UWorldSubsystem, public ILoadingProcessInterface
{
    virtual bool ShouldShowLoadingScreen(FString& OutReason) const override
    {
        if (bStillWorking)
        {
            OutReason = TEXT("正在初始化我的系统");
            return true;
        }
        return false;
    }
};

// 方式二（更常用）：直接用占位任务
ULoadingProcessTask* Task =
    ULoadingProcessTask::CreateLoadingScreenProcessTask(this, TEXT("正在加载关卡数据"));
// ... 干完活之后：
Task->Unregister();
```

### 5.3 蓝图里占住加载界面

```
创建 "Create Loading Screen Process Task"  →  保存返回的 Task 引用
        ... 异步做事 ...
调用 Task 的 "Unregister"
```

### 5.4 几个容易踩的坑

1. **忘了 `Unregister()` —— 这是本插件最常见的 bug**。加载界面会永久挂着，游戏看起来"卡死"，但其实只是没人告诉它加载完了。

2. **"额外多挂几秒"默认在编辑器里不生效** —— 为了加快迭代。想验证它，打开 `HoldLoadingScreenAdditionalSecsEvenInEditor`。

3. **`ForceTickLoadingScreenEvenInEditor` 上方那句英文注释是错的** —— Epic 原版是复制粘贴遗留，写的仍是"额外多挂几秒"，但这项实际管的是"编辑器里要不要每帧 Tick"。已在源码注释中标明。

4. **排查"加载界面不退"的正确姿势** ——
   - 把 `LogLoadingScreenHeartbeatInterval` 设成非 0（比如 5），日志会直接告诉你**是谁**在拖着不放；
   - 或者开 `LogLoadingScreenReasonEveryFrame` 看每帧的原因；
   - 也可以直接在控制台输 `CommonLoadingScreen.AlwaysShow 0` 确认不是被强制显示。

5. **启动画面只能用纯 Slate** —— 它运行在 UObject/UMG 就绪之前。想放自己的 Logo，需要改 `SCommonPreLoadingScreenWidget::Construct()`，用 Slate 画或者走 `MoviePlayer` 播片。

6. **专用服务器不会加载任何启动画面资源** —— 但命令行工具（Cook）会，这是故意的，好让 Cook 把资源收集进去。

7. **`LoadingScreenZOrder` 要足够高** —— 如果加载界面被游戏 UI 盖住，多半是这个值设小了（默认 10000 一般来说够用）。
