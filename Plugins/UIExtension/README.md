# UIExtension 插件

## 1. 插件用途与适用场景

本插件提供一套**基于 GameplayTag 的 UI 扩展机制**，让 UI 上某个位置可以"事后被塞东西进来"，而不需要提前知道要塞什么。

**核心是两个角色：**

| 角色 | 说明 | 典型使用者 |
|---|---|---|
| **扩展点（Extension Point）** | UI 上的一个"空位"，声明"我这里可以放东西" | HUD 布局、菜单页面 |
| **扩展（Extension）** | 被贡献出去的内容（通常是一个控件类） | GameFeature 插件、玩法模块 |

两者通过一个 **GameplayTag** 关联，谁先注册都不影响结果——系统会互相匹配并通知。

**要解决的问题：**
传统做法里，HUD 上要加一个活动入口图标，就得去改 HUD 蓝图本身。玩法越多，HUD 越臃肿，且不同玩法之间互相污染。

本插件的解法是：HUD 只声明"这里有个叫 `HUD.Extension.ActivityRow` 的空位"，各 GameFeature 插件各自往这个标签贡献自己的控件。玩法卸载时扩展自动消失，HUD 一行代码都不用改。

**典型适用场景：**

- HUD / 菜单需要被多个独立玩法模块动态填充
- 分屏 / 多人场景下，每个玩家的 UI 需要独立扩展（用 ContextObject 区分）
- 需要 UI 模块之间解耦、可独立开关

Lyra 里 `W_ShooterHUDLayout` 等界面就大量使用了本插件。

---

## 2. 目录结构

```
UIExtension/
├── UIExtension.uplugin                    # 插件描述文件（依赖 CommonGame）
├── README.md                              # 本文档
└── Source/
    ├── UIExtension.Build.cs
    ├── Public/                            # 对外公开
    │   ├── UIExtensionSystem.h            # 核心：子系统、句柄、结构体、委托
    │   └── Widgets/
    │       └── UIExtensionPointWidget.h   # 扩展点控件（消费端）
    └── Private/
        ├── UIExtensionSystem.cpp
        ├── UIExtensionModule.cpp          # 模块入口
        ├── LogUIExtension.h / .cpp        # 日志分类 LogUIExtension
        └── Widgets/
            └── UIExtensionPointWidget.cpp
```

> `CanContainContent = false`，纯 C++ 插件，不含内容资产。

---

## 3. 核心类 / 模块及其职责

### 3.1 模块

| 模块名 | 类型 | 职责 |
|---|---|---|
| `UIExtension` | Runtime | 提供全部扩展机制。`FUIExtensionModule` 的 `StartupModule` / `ShutdownModule` 均为空，无自定义初始化逻辑。 |

### 3.2 核心类型（`UIExtensionSystem.h`）

**枚举：**

| 类型 | 取值 | 含义 |
|---|---|---|
| `EUIExtensionPointMatch` | `ExactMatch` / `PartialMatch` | 标签匹配规则。精确 / 连子标签一起收 |
| `EUIExtensionAction` | `Added` / `Removed` | 回调里的动作：新增还是移除 |

**结构体：**

| 类型 | 职责 |
|---|---|
| `FUIExtension` | 一条"扩展"（贡献方数据）：目标标签、优先级、上下文对象、数据（控件类或 UObject） |
| `FUIExtensionPoint` | 一个"扩展点"（消费方槽位）：监听标签、上下文、匹配规则、允许的数据类、回调 |
| `FUIExtensionRequest` | 回调参数：句柄、标签、优先级、数据、上下文对象 |
| `FUIExtensionHandle` | 扩展句柄，用于反注册 / 判有效性（蓝图可用） |
| `FUIExtensionPointHandle` | 扩展点句柄，同上 |

**委托：**

| 委托 | 用途 |
|---|---|
| `FExtendExtensionPointDelegate` | C++ 版扩展点回调 `(Action, Request)` |
| `FExtendExtensionPointDynamicDelegate` | 蓝图版（动态委托），供 `K2_` 系列函数使用 |

**子系统：**

| 类 | 职责 |
|---|---|
| `UUIExtensionSubsystem` | World 子系统。维护 `ExtensionPointMap` 和 `ExtensionMap` 两张表，负责匹配与通知 |

**蓝图函数库：**

| 类 | 职责 |
|---|---|
| `UUIExtensionHandleFunctions` | 蓝图里对扩展句柄做 `Unregister` / `IsValid` |
| `UUIExtensionPointHandleFunctions` | 蓝图里对扩展点句柄做同样操作 |

### 3.3 扩展点控件（`UIExtensionPointWidget`）

`UUIExtensionPointWidget` 派生自 `UDynamicEntryBoxBase`，是**消费端**控件。放在 UI 布局里即声明一个可插拔位置。

它有两个关键委托，用于"贡献的是数据而非控件类"的场景：

| 委托 | 用途 |
|---|---|
| `FOnGetWidgetClassForData` | 把数据换算成该用哪个控件类显示；返回空表示忽略这份数据 |
| `FOnConfigureWidgetForData` | 控件创建后的初始化钩子 |

---

## 4. 依赖的其他插件或模块

**插件依赖：**

| 插件 | 必需性 | 说明 |
|---|---|---|
| `CommonGame` | 必需 | 提供 `UCommonLocalPlayer`，用于"分玩家的上下文"扩展 |

**模块依赖（Build.cs）：**

| 模块 | 用途 |
|---|---|
| `Core` / `CoreUObject` / `Engine` | UE 基础三件套 |
| `SlateCore` / `Slate` / `UMG` | 控件与 UI 底层 |
| `CommonUI` | Lyra/CommonGame 体系的 UI 基础 |
| `CommonGame` | `UCommonLocalPlayer`（分玩家上下文） |
| `GameplayTags` | 扩展点标签体系 |

无私有依赖。

---

## 5. 接入与使用要点

### 5.1 消费端：在 UI 里放一个扩展点

**蓝图方式**：在控件蓝图的面板里拖入 **UIExtensionPointWidget**，设置：

- `ExtensionPointTag`：例如 `HUD.Extension.ActivityRow`
- `ExtensionPointTagMatch`：一般用 `ExactMatch`；想连子标签一起收就用 `PartialMatch`
- `DataClasses`：**通常留空**，`UUserWidget::StaticClass()` 会被自动加入

**运行时行为（重要）**：一个该控件最多会注册 **3 个** 扩展点——

1. 无上下文
2. `LocalPlayer` 上下文
3. `PlayerState` 上下文（等玩家状态就绪后补注册）

这就是 `ExtensionPointHandles` 是数组的原因。

### 5.2 贡献端：注册一个扩展

**蓝图**：

```
Register Extension (Widget)
    ExtensionPointTag = HUD.Extension.ActivityRow
    WidgetClass       = W_MyActivityIcon
    Priority          = 0
```

**C++**：

```cpp
UUIExtensionSubsystem* Subsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
FUIExtensionHandle Handle = Subsystem->RegisterExtensionAsWidget(
    FGameplayTag::RequestGameplayTag("HUD.Extension.ActivityRow"),
    UMyActivityIcon::StaticClass(),
    /*Priority=*/ 0);
```

### 5.3 ⚠️ 五个必须知道的坑

1. **注册的是"控件类"，不是实例。**
   `RegisterExtensionAsWidget` 内部走 `RegisterExtensionAsData(Tag, nullptr, WidgetClass, Priority)`，传进去的是 `UClass`。创建/销毁控件是**消费端**（扩展点）的事。

2. **`AllowedDataClasses` 不能为空，否则注册直接失败。**
   注册扩展点时若传空数组，会返回无效句柄并打 Warning 日志。

3. **契约不匹配会被【静默丢弃】。**
   扩展的数据类必须继承自（或实现了）扩展点 `AllowedDataClasses` 中的某个类，且上下文对象要一致。不满足时**不报任何错**，扩展点永远收不到——这是排查"扩展没生效"时最先要查的地方。

4. **注册扩展点会立即回放已有扩展。**
   注册后系统会为"之前就已存在"的扩展同步补发一批 `Added`，所以回调必须能处理这初始的一批，不能假定只在将来异步触发。

5. **`Priority` 本系统不排序。**
   它只是透传给 `FUIExtensionRequest::Priority`，由消费方自己决定怎么用。惯例是数值越大越优先。

### 5.4 分玩家（本地分屏）场景

用带 `ForContext` 的版本注册，传入对应的 `LocalPlayer` 或 `PlayerState`：

```cpp
Subsystem->RegisterExtensionAsWidgetForContext(Tag, LocalPlayer, WidgetClass, Priority);
```

只有上下文对象相同的扩展点才会收到它。

### 5.5 一定要反注册

扩展或扩展点的持有者销毁时，必须调用 `UnregisterExtension` / `UnregisterExtensionPoint`（或句柄上的 `Unregister()`）。否则：

- 扩展点会继续收到回调，打给一个已销毁的对象
- 贡献出去的控件类会一直挂着，UI 上出现幽灵控件

蓝图里用 `UUIExtensionHandleFunctions::Unregister`。

### 5.6 调试

相关日志都在 **`LogUIExtension`** 分类下。注册被拒绝、句柄无效等情况会打 Warning。
把日志级别调到 Verbose 还能看到每次注册/注销的明细。
