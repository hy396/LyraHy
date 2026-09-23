# GameSettings 插件说明

> 本文为中文说明文档。插件源码中的注释也已补充为中文。

## 一、插件用途与适用场景

`GameSettings` 是 Epic 官方（Lyra 示例工程）提供的一套**通用游戏设置界面框架**。它解决的是一类很常见但又很琐碎的问题：

> 游戏有几十上百项设置（画质、音量、键位、语言、辅助功能……），
> 需要在 UI 上列出来、支持搜索、支持"改了但先不生效"、支持"一键恢复默认"、
> 还要能按平台/账号/玩家身份动态隐藏某些项——怎么组织才不写成一坨？

本插件给出的答案是：**把"设置项"抽象成对象树，把"值"和"表现"彻底分离，把"可见性"交给可插拔的编辑条件。**

典型适用场景：

- 需要多层级（分组 → 子页面 → 具体项）的设置界面
- 需要"改了值但点保存才生效"，且能一键取消
- 需要按平台特性（有无手柄、有无触屏）动态隐藏/禁用设置项
- 需要键位重绑定界面（"请按任意键"、按键冲突提示）
- 需要把玩家的设置改动上报到数据分析

**注意**：本插件只提供**框架与控件基类**，不包含任何具体设置项。具体设置项由游戏工程（如 Lyra 的 `Source/LyraGame/Settings/`）继承本插件的类来实现。

## 二、目录结构

```
Plugins/GameSettings/
├── GameSettings.uplugin
├── README.md
└── Source/
    ├── GameSettings.Build.cs
    ├── Public/
    │   ├── GameSetting.h                      设置项基类
    │   ├── GameSettingCollection.h            分组 / 子页面
    │   ├── GameSettingValue.h                 带数值的设置项基类
    │   ├── GameSettingValueDiscrete.h         离散值（几选一）
    │   ├── GameSettingValueScalar.h           连续值（滑块）
    │   ├── GameSettingValueDiscreteDynamic.h  离散值（反射版）
    │   ├── GameSettingValueScalarDynamic.h    连续值（反射版）
    │   ├── GameSettingAction.h                按钮型设置项
    │   ├── GameSettingFilterState.h           过滤状态 + 编辑状态 + 编辑条件
    │   ├── GameSettingRegistry.h              设置注册表
    │   ├── GameSettingRegistryChangeTracker.h 改动跟踪器
    │   ├── DataSource/                        数据源（值存哪、怎么读写）
    │   │   ├── GameSettingDataSource.h
    │   │   └── GameSettingDataSourceDynamic.h
    │   ├── EditCondition/                     内置编辑条件
    │   │   ├── WhenCondition.h                内联 lambda 条件
    │   │   ├── WhenPlatformHasTrait.h         按平台特性
    │   │   └── WhenPlayingAsPrimaryPlayer.h   按是否主玩家
    │   └── Widgets/
    │       ├── GameSettingScreen.h            设置界面（最外层）
    │       ├── GameSettingPanel.h             面板（列表 + 详情）
    │       ├── GameSettingListView.h          列表视图
    │       ├── GameSettingListEntry.h         列表条目控件（5 种）
    │       ├── GameSettingDetailView.h        右侧详情面板
    │       ├── GameSettingDetailExtension.h   详情区扩展区块
    │       ├── GameSettingVisualData.h        外观配置资产
    │       ├── IGameSettingActionInterface.h  具名动作处理接口
    │       └── Misc/
    │           ├── GameSettingPressAnyKey.h   "请按任意键"界面
    │           ├── GameSettingRotator.h       带默认值标记的切换器
    │           └── KeyAlreadyBoundWarning.h   按键冲突提示界面
    └── Private/
        ├── GameSettingsModule.cpp
        ├── Registry/
        ├── Widgets/
        │   └── Responsive/                    响应式流式布局面板
        │       ├── GameResponsivePanel.h/.cpp
        │       ├── GameResponsivePanelSlot.h/.cpp
        │       └── SGameResponsivePanel.h/.cpp
        └── ...（各 Public 头文件对应的实现）
```

## 三、核心类与职责

### 3.1 数据层：设置项对象树

| 类 | 职责 |
|---|---|
| `UGameSetting` | **所有设置项的基类**。持有显示名、描述、警告文本、标签；管理编辑条件与依赖；提供三个事件（值变更 / 被应用 / 编辑条件变化） |
| `UGameSettingValue` | 带数值的设置项基类。新增三个纯虚：`StoreInitial()`、`ResetToDefault()`、`RestoreToInitial()` |
| `UGameSettingValueDiscrete` | 离散值：从固定若干选项里选一个（画质、语言） |
| `UGameSettingValueScalar` | 连续值：滑块。内部维护"真实值"与"归一化值(0~1)"两套坐标 |
| `UGameSettingAction` | 按钮型：没有值，点了执行动作（恢复默认、查看名单） |
| `UGameSettingCollection` | 分组容器。本身也是 `UGameSetting`，可嵌套 |
| `UGameSettingCollectionPage` | **可导航进入的子页面**（比 Collection 多一个导航事件） |
| `UGameSettingRegistry` | 设置树的总根。持有顶层项、按过滤产出可见列表、转发事件、统一保存 |

### 3.2 状态层：可见性 / 可编辑性

| 类 | 职责 |
|---|---|
| `FGameSettingFilterState` | 过滤状态：搜索关键字 + 是否包含隐藏/禁用项 + 可选白名单 |
| `FGameSettingEditableState` | 某个设置项"当前能不能被用户动"的快照，**同时记录原因**（不是单纯 bool） |
| `FGameSettingEditCondition` | 编辑条件基类。运行时决定某项的可见/可编辑 |

`FGameSettingEditableState` 的几个动作值得记住：

- `Hide(DevReason)` —— 隐藏（只需给开发者看的理由）
- `Disable(Reason)` —— 禁用但仍可见（必须给玩家看的理由）
- `DisableOption(Option)` —— 只隐藏离散选项中的某一档
- `Kill(DevReason)` —— 彻底杀掉：隐藏 + 不可重置 + 排除出数据分析

### 3.3 改动管理

| 类 | 职责 |
|---|---|
| `FGameSettingRegistryChangeTracker` | 监听注册表，把所有被改过的项记进脏列表，支撑"有未保存改动 / 应用 / 取消"三件事 |

### 3.4 表现层：控件

| 类 | 职责 |
|---|---|
| `UGameSettingScreen` | 设置界面最外层。持有注册表 + 改动跟踪器，提供 `ApplyChanges()` / `CancelChanges()` |
| `UGameSettingPanel` | 左侧列表 + 右侧详情；维护导航栈以支持子页面进出 |
| `UGameSettingListView` | 设置项列表视图（条目控件必须派生自 `UGameSettingListEntryBase`） |
| `UGameSettingListEntryBase` 及其子类 | 5 种条目控件：基础 / 通用 / 离散 / 滑块 / 按钮 / 导航入口 |
| `UGameSettingDetailView` | 右侧详情面板（名称、描述、动态详情、警告、禁用原因、扩展区块） |
| `UGameSettingDetailExtension` | 详情区可插拔的自定义区块 |
| `UGameSettingVisualData` | **外观配置资产**：把"哪种设置项用哪个控件"做成数据配置，改外观不用改 C++ |

### 3.5 辅助组件

| 类 | 职责 |
|---|---|
| `UGameSettingPressAnyKey` | "请按任意键"界面，用输入预处理器抢先吃掉第一次按键 |
| `UGameSettingRotator` | 带"默认值标记"的左右切换器 |
| `UKeyAlreadyBoundWarning` | 改键位时的"该键已被占用"提示界面 |
| `UGameResponsivePanel` | 响应式流式布局面板：一行放不下自动换行 |

## 四、依赖关系

**插件依赖**（`GameSettings.uplugin`）：
- `CommonUI`（必需）—— UI 基础；`FWhenPlatformHasTrait` 用到的平台特性标签也来自它

**模块依赖**（`GameSettings.Build.cs`）：

| 类型 | 模块 | 用途 |
|---|---|---|
| Public | Core / CoreUObject / Engine | UE 基础 |
| Public | InputCore | 键位选择 |
| Public | Slate / SlateCore / UMG | UI 底层与 UMG 控件 |
| Public | CommonInput / CommonUI | Lyra/CommonGame 体系的 UI 与输入基础 |
| Public | GameplayTags | 设置项标签、具名动作标签 |
| Private | ApplicationCore | 窗口/输入等平台层能力 |
| Private | PropertyPath | `FGameSettingDataSourceDynamic` 用它按属性路径做反射读写 |

**被谁依赖**：本工程里 `Source/LyraGame/Settings/` 大量使用本插件（Lyra 的各种具体设置项、键位设置、设置界面蓝图）。

## 五、接入与使用要点

### 5.1 实现一个设置界面（最简流程）

```
1. 继承 UGameSettingRegistry，在 OnInitialize() 里构建设置树
2. 继承 UGameSettingScreen，实现 CreateRegistry() 返回上一步的注册表
3. 做蓝图：放一个 Settings_Panel（类型是 UGameSettingPanel）
4. 配一份 UGameSettingVisualData 资产，指定每种设置项用哪个条目控件
```

### 5.2 添加一个具体设置项

以"离散值"为例：

```cpp
// 1) 创建设置项
UGameSettingValueDiscreteDynamic* Setting = NewObject<UGameSettingValueDiscreteDynamic>();
Setting->SetDevName("Resolution");               // 唯一标识，用于查找/导航
Setting->SetDisplayName(FText::FromString("分辨率"));
Setting->SetDescriptionRichText(...);

// 2) 给它接上数据源（值存在哪）
Setting->SetDataSource(MakeShared<FGameSettingDataSourceDynamic>(Path));

// 3) （可选）加编辑条件：只有主玩家能改
Setting->AddEditCondition(FWhenPlayingAsPrimaryPlayer::Get());

// 4) （可选）声明依赖：A 变了要刷新 B
Setting->AddEditDependency(AnotherSetting);

// 5) 挂到树上
CollectionPage->AddSetting(Setting);
```

### 5.3 几个容易踩的坑

1. **`DevName` 必须唯一** —— 它是设置项的身份证，注册表靠它查找、界面靠它导航定位。

2. **`StoreInitial()` 在 `Apply()` 之后也要再调一次** —— 否则玩家保存后再改再取消，会撤销到保存前的旧值。

3. **编辑条件里的 `AllowedDataClasses` 类比：`Disable()` 必须给玩家可见的理由文本，`Hide()` 必须给开发者理由**。两者都要求传参，不接受空。

4. **`UGameSettingAction` 默认不算"改动"** —— 如果你的动作确实改了设置，必须显式 `SetDoesActionDirtySettings(true)`，否则界面不会提示保存。

5. **`FGameSettingDataSourceDynamic` 靠属性路径字符串定位** —— 重命名 C++ 属性会导致它**静默失效**（不报错，只是读不到值）。改名时务必同步检查配置。

6. **`UGameSettingListView` 的条目控件必须派生自 `UGameSettingListEntryBase`**，否则数据绑定不上。

7. **设置界面关闭即等于放弃改动** —— `UGameSettingScreen::NativeOnDeactivated()` 里会自动走 `CancelChanges()`。如果希望关闭时提示玩家，需要重写这个行为。

8. **异步初始化**：如果某个设置项需要异步准备（如读平台配置），走 `Startup()` → `StartupComplete()` 流程；界面会等所有项 `IsReady()` 后才显示，不用自己写等待逻辑。

### 5.4 调试技巧

- 某项莫名不显示 → 检查它的编辑条件是否被 `Kill()` 了；`FGameSettingEditCondition::ToString()` 就是为这个准备的。
- 改了值但没生效 → 区分"改值"和"Apply"：多数设置项改值即刻生效，只有少数（分辨率等）需要 Apply。
- 数据分析里看不到某项 → 检查 `bReportAnalytics`，以及该项是否被 `HideFromAnalytics()` / `Kill()` 排除了。
