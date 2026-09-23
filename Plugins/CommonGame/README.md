# CommonGame 插件说明

> 本文为中文说明文档。插件源码中的注释也已补充为中文。

## 一、插件用途与适用场景

`CommonGame` 是 Epic 官方（Lyra 示例工程）提供的一套**通用游戏框架层**，位于"引擎基础"与"具体游戏逻辑"之间。它解决的是几个几乎所有游戏都会遇到、但每个项目都容易各写各的的问题：

1. **UI 怎么组织？** —— 多层 UI（菜单、游戏内 HUD、弹窗、过场）叠在一起，谁盖谁、怎么推入弹出，需要一个统一模型。
2. **分屏怎么做？** —— 本地多个玩家，每个人该有自己的一套 UI 和视口。
3. **"玩家控制器/状态/角色什么时候才存在？"** —— 这三样不是同时就位的，调用方总在判断时序。
4. **弹确认框、异步加载界面、挂起输入** —— 这些样板代码每次都要重写。
5. **在线身份与会话邀请** —— 玩家从平台好友列表点进游戏，该怎么接管？

适合任何使用 **CommonUI + CommonUser + ModularGameplayActors** 体系的 UE 项目，尤其是需要分屏或复杂 UI 分层的项目。

## 二、目录结构

```
Plugins/CommonGame/
├── CommonGame.uplugin
├── README.md
└── Source/
    ├── CommonGame.Build.cs
    ├── Public/
    │   ├── CommonGameInstance.h            增强版 GameInstance（在线身份 + 会话邀请）
    │   ├── CommonLocalPlayer.h             增强版 LocalPlayer（三件套就位通知）
    │   ├── CommonPlayerController.h        增强版 PlayerController
    │   ├── CommonPlayerInputKey.h          键位提示控件（[E] 图标、长按进度）
    │   ├── CommonUIExtensions.h            UI 静态工具库（推内容、挂起输入）
    │   ├── GameUIManagerSubsystem.h        UI 总管理器（持有 UI 策略）
    │   ├── GameUIPolicy.h                  UI 策略（分屏模式、根布局类）
    │   ├── PrimaryGameLayout.h             根 UI 布局（图层栈）
    │   ├── Actions/                        三个蓝图异步节点
    │   │   ├── AsyncAction_CreateWidgetAsync.h
    │   │   ├── AsyncAction_PushContentToLayerForPlayer.h
    │   │   └── AsyncAction_ShowConfirmation.h
    │   └── Messaging/                      对话框体系
    │       ├── CommonMessagingSubsystem.h
    │       └── CommonGameDialog.h
    └── Private/
        ├── CommonGameModule.cpp
        ├── LogCommonGame.h/.cpp
        └── ...（各 Public 头文件对应的实现）
```

## 三、核心类与职责

### 3.1 UI 分层体系（本插件的核心）

```
UGameUIManagerSubsystem（GameInstance 子系统，抽象）
    │  持有并转发事件
    ▼
UGameUIPolicy（抽象，决定"每人一套什么样的 UI"）
    │  为每个玩家创建
    ▼
UPrimaryGameLayout（单个玩家的根布局）
    │  内含若干
    ▼
UI.Layer.Menu / UI.Layer.GameMenu / UI.Layer.Modal ...（每个图层是一个控件栈）
```

| 类 | 职责 |
|---|---|
| `UGameUIManagerSubsystem` | UI 总管理器。**抽象**，游戏必须继承它并指定 `DefaultUIPolicyClass`。只负责持有策略 + 转发玩家增减事件 |
| `UGameUIPolicy` | UI 策略。决定分屏模式（`PrimaryOnly` / `SingleToggle` / `Simultaneous`）、根布局类、布局何时加入/移出视口 |
| `UPrimaryGameLayout` | 单个玩家的根布局。把图层标签（GameplayTag）映射到控件栈容器，提供"往某图层推控件"的接口 |
| `UCommonUIExtensions` | 静态工具库：查输入设备类型、往图层推/弹内容、挂起/恢复输入 |

### 3.2 玩家三件套的时序问题

| 类 | 职责 |
|---|---|
| `UCommonLocalPlayer` | 为 PlayerController / PlayerState / Pawn 各提供一个委托，并提供 `CallAndRegister_OnXXXSet()` —— **已有就立刻回调，没有就注册等着** |
| `ACommonPlayerController` | 在上述三者各自就位时通知所属 LocalPlayer。派生自 `AModularPlayerController` |

### 3.3 在线与会话

| 类 | 职责 |
|---|---|
| `UCommonGameInstance` | 接收 CommonUser 的通知（系统消息、权限变化、登录结果）；实现完整的"被邀请加入会话"流程 |

### 3.4 对话框

| 类 | 职责 |
|---|---|
| `UCommonMessagingSubsystem` | LocalPlayer 级子系统，"弹确认框"的统一入口 |
| `UCommonGameDialogDescriptor` | 对话框的数据描述（标题 + 正文 + 按钮组合） |
| `UCommonGameDialog` | 对话框控件基类（抽象，游戏继承做外观） |

### 3.5 异步节点（蓝图可用）

| 类 | 职责 |
|---|---|
| `UAsyncAction_CreateWidgetAsync` | 异步加载控件类并创建实例，完成后回调 |
| `UAsyncAction_PushContentToLayerForPlayer` | 异步加载并直接推入指定图层，带 BeforePush / AfterPush 两个回调 |
| `UAsyncAction_ShowConfirmation` | 弹确认框并等待玩家选择，蓝图里能写出线性逻辑 |

### 3.6 其它

| 类 | 职责 |
|---|---|
| `UCommonPlayerInputKey` | 键位提示控件（"按 [E] 开门"里的 [E]）。自动跟随玩家输入设备切换图标，支持长按进度圈 |

## 四、依赖关系

**插件依赖**（`CommonGame.uplugin`）：
- `CommonUI` —— UI 体系基础
- `CommonUser` —— 在线身份、权限、会话（`UCommonGameInstance` 依赖它）
- `ModularGameplayActors` —— `ACommonPlayerController` 派生自 `AModularPlayerController`
- `OnlineFramework` —— 会话搜索结果等相关类型

**模块依赖**（`CommonGame.Build.cs`）均为 Public 依赖：

| 模块 | 用途 |
|---|---|
| Core / CoreUObject / Engine | UE 基础三件套 |
| InputCore | 键位提示控件 |
| Slate / SlateCore / UMG | UI 底层与 UMG 控件 |
| CommonInput / CommonUI | UI 与输入体系基础 |
| CommonUser | 在线身份、权限、会话 |
| GameplayTags | UI 图层标签（`UI.Layer.*`） |
| ModularGameplayActors | 可模块化扩展的 PlayerController 基类 |

**被谁依赖**：本工程里 `Source/LyraGame/` 的 UI 与前端流程（`LyraUIManagerSubsystem`、`LyraUIPolicy`、`LyraGameInstance`、`LyraLocalPlayer`、`LyraPlayerController`）均派生自本插件的类。

## 五、接入与使用要点

### 5.1 最小接入步骤

1. **UI 布局**：创建一个 `UPrimaryGameLayout` 的蓝图子类，在里面放好各个图层容器（每个是一个 `UCommonActivatableWidgetContainerBase`），并在构造时为每个容器调用一次 `RegisterLayer(UI.Layer.XXX, 容器)`。
2. **UI 策略**：创建一个 `UGameUIPolicy` 的蓝图子类，把 `LayoutClass` 指向上一步的布局类，设置好分屏模式。
3. **UI 管理器**：C++ 继承 `UGameUIManagerSubsystem`，在配置里把 `DefaultUIPolicyClass` 指向第 2 步的策略类。
4. **GameInstance / LocalPlayer / PlayerController**：分别继承 `UCommonGameInstance` / `UCommonLocalPlayer` / `ACommonPlayerController`，并在项目设置里替换掉引擎默认类。
5. **图层标签**：在 `Config/DefaultGameplayTags.ini` 里定义 `UI.Layer.*` 系列的标签。

### 5.2 常用代码片段

```cpp
// 往主玩家的菜单图层推一个界面（同步）
UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, TAG_UI_Layer_Menu, W_MenuClass);

// 异步推入 + 初始化
Layout->PushWidgetToLayerStackAsync<UMyWidget>(
    TAG_UI_Layer_GameMenu,
    /*bSuspendInputUntilComplete=*/true,
    W_MyWidgetClass,
    [](EAsyncWidgetLayerState State, UMyWidget* Widget)
    {
        if (State == EAsyncWidgetLayerState::Initialize && Widget)
        {
            Widget->SetMyData(...);   // 推入前初始化
        }
    });

// 等某个玩家的角色就位（不用自己判断时序）
if (UCommonLocalPlayer* LP = Cast<UCommonLocalPlayer>(LocalPlayer))
{
    LP->CallAndRegister_OnPlayerPawnSet(
        FPlayerPawnSetDelegate::CreateLambda([](UCommonLocalPlayer* LP, APawn* Pawn) {
            // 这里一定能拿到 Pawn
        }));
}
```

### 5.3 几个容易踩的坑

1. **`UGameUIManagerSubsystem` 是抽象类** —— 不能直接实例化，必须由游戏继承并指定 `DefaultUIPolicyClass`，否则 UI 完全不会出现且不报错。

2. **图层必须先 `RegisterLayer`** —— 忘了注册的话，`PushWidgetToLayerStack` 会静默返回 `nullptr`。排查"界面推不出来"时先检查这一项。

3. **`SuspendInputForPlayer` / `ResumeInputForPlayer` 必须成对** —— 挂起会返回一个令牌，恢复时必须用同一个令牌。内部是计数式的，可以嵌套挂起，但漏掉一次恢复会导致玩家输入永久失灵。

4. **分屏时每个玩家有各自的 `UPrimaryGameLayout`** —— 想对"所有玩家"做 UI 操作必须遍历，只取主玩家的那个（`GetPrimaryGameLayoutForPrimaryPlayer`）会漏掉其他人。

5. **`UGameInstance` / `ULocalPlayer` / `APlayerController` 的替换要在项目设置里做** —— 只写 C++ 子类不生效，必须在 `Project Settings → Maps & Modes`（或 `DefaultGame.ini`）里把它们设为默认类。

6. **`UCommonPlayerInputKey` 推荐用 `SetBoundAction` 而不是 `SetBoundKey`** —— 前者跟随玩家的自定义键位，后者写死某个键，玩家改了键位界面却不更新。

7. **异步推 UI 时记得处理 `Canceled` 状态** —— `EAsyncWidgetLayerState::Canceled` 时 Widget 是 `nullptr`，直接解引用会崩。

8. **对话框是 LocalPlayer 级的** —— 分屏时调用 `ShowConfirmation` 要指定对 LocalPlayer，否则会弹到错误的玩家屏幕上。
