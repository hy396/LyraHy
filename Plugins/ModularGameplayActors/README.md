# ModularGameplayActors 插件

## 1. 插件用途与适用场景

本插件提供一组**可被 GameFeature 插件动态扩展的 Actor 基类**，是 Epic 官方「模块化 Gameplay（Modular Gameplay）」方案的一部分。

**要解决的问题：**
传统做法里，想给角色 / 玩家控制器 / GameState 增加新功能，只能去改这些类的源码或蓝图基类。一旦项目要做 DLC、活动玩法、限时模式，就会出现「为了一个临时玩法去动公共基类」的尴尬。

本插件的解法是把这些 Actor 做成**空壳基类**：自身几乎不含游戏逻辑，只在生命周期的关键时机把自己登记到 `UGameFrameworkComponentManager`，并广播「Actor 就绪」事件。之后 GameFeature 插件就可以在运行时**动态往这些 Actor 上挂组件**，完全不需要修改基类代码。

**典型适用场景：**

- 需要按 GameFeature 动态装卸玩法的项目（Lyra 就是这么做的）
- 需要支持 DLC / 赛季 / 限时模式的游戏
- 希望不同玩法模块之间互不干扰、可独立开关

**一句话总结：** 它本身不提供任何玩法，提供的是「让玩法可以被热插拔」的骨架。

---

## 2. 目录结构

```
ModularGameplayActors/
├── ModularGameplayActors.uplugin          # 插件描述文件
├── README.md                              # 本文档
└── Source/
    └── ModularGameplayActors/             # 唯一的运行时模块
        ├── ModularGameplayActors.Build.cs # 模块构建规则
        ├── Public/                        # 对外公开的头文件（7 个）
        │   ├── ModularAIController.h
        │   ├── ModularCharacter.h
        │   ├── ModularGameMode.h
        │   ├── ModularGameState.h
        │   ├── ModularPawn.h
        │   ├── ModularPlayerController.h
        │   └── ModularPlayerState.h
        └── Private/                       # 实现文件（8 个）
            ├── ModularAIController.cpp
            ├── ModularCharacter.cpp
            ├── ModularGameMode.cpp
            ├── ModularGameState.cpp
            ├── ModularGameplayActorsModule.cpp   # 模块入口
            ├── ModularPawn.cpp
            ├── ModularPlayerController.cpp
            └── ModularPlayerState.cpp
```

> 说明：本插件 `CanContainContent = false`，**不含任何内容资产**，是纯 C++ 插件。

---

## 3. 核心类 / 模块及其职责

### 3.1 模块

| 模块名 | 类型 | 职责 |
|---|---|---|
| `ModularGameplayActors` | Runtime | 提供全部 Modular 基类。模块入口用 `IMPLEMENT_MODULE(FDefaultModuleImpl, ...)`，没有自定义启动/关闭逻辑。 |

### 3.2 核心类

所有类的核心机制完全一致，都是这三步：

```
PreInitializeComponents  →  AddGameFrameworkComponentReceiver(this)        登记为组件接收者
BeginPlay（或更合适的时机） →  SendGameFrameworkComponentExtensionEvent(
                                 NAME_GameActorReady)                      广播“Actor 已就绪”
EndPlay                  →  RemoveGameFrameworkComponentReceiver(this)     注销，避免悬空引用
```

| 类 | 继承自 | 职责与特殊之处 |
|---|---|---|
| `AModularPawn` | `APawn` | 标准三步。 |
| `AModularCharacter` | `ACharacter` | 标准三步。最常用的一个，Lyra 的角色就派生自它。 |
| `AModularAIController` | `AAIController` | 标准三步。 |
| `AModularPlayerState` | `APlayerState` | 标准三步；**额外**把 `Reset()` 转发给所有 `UPlayerStateComponent`，并重写 `CopyProperties()` 按「同类型 + 同名」把组件属性搬到目标 PlayerState（用于玩家重连 / 换座位）。 |
| `AModularPlayerController` | `APlayerController` | 只有「登记 / 注销」两步；**就绪事件改在 `ReceivedPlayer()` 派发**（而非 BeginPlay），因为 PlayerController 必须等到真正分配了 Player 之后才可用。此外把 `ReceivedPlayer()` 和 `PlayerTick()` 转发给所有 `UControllerComponent`。 |
| `AModularGameStateBase` | `AGameStateBase` | 标准三步。 |
| `AModularGameState` | `AGameState` | 标准三步；**额外**重写 `HandleMatchHasStarted()`，比赛开始时转发给所有 `UGameStateComponent`。 |
| `AModularGameModeBase` | `AGameModeBase` | 无生命周期重写，只在构造函数里**预设好整套配套类**。 |
| `AModularGameMode` | `AGameMode` | 同上，配 `AModularGameState`。 |

### 3.3 关于「配对」

- `AModularGameModeBase` ↔ `AModularGameStateBase`
- `AModularGameMode` ↔ `AModularGameState`

两个 GameMode 的构造函数都已把 `GameStateClass / PlayerControllerClass / PlayerStateClass / DefaultPawnClass` 设成了对应的 Modular 版本，**不需要你在蓝图里再手动指定**。

> ⚠️ 源码里 `AModularGameState` 上方的原版英文注释写的是 “Pair this with a ModularGameState”，这是 Epic 的笔误，它实际应该搭配的是 `AModularGameMode`。

---

## 4. 依赖的其他插件或模块

**插件依赖（.uplugin 的 Plugins 段）：**

| 插件 | 必需性 | 说明 |
|---|---|---|
| `ModularGameplay` | 必需 | 引擎自带插件，提供 `UGameFrameworkComponentManager`——本插件的核心机制就来自这里。 |

**模块依赖（Build.cs）：**

| 模块 | 用途 |
|---|---|
| `Core` / `CoreUObject` / `Engine` | UE 基础三件套 |
| `ModularGameplay` | 提供 `UGameFrameworkComponentManager` |
| `AIModule` | `AModularAIController` 派生自 `AAIController`，需要它 |

无私有依赖、无动态加载模块。

---

## 5. 接入与使用要点

### 5.1 启用插件

在 `.uproject` 或编辑器插件列表中启用 `ModularGameplayActors`（它会自动带上 `ModularGameplay`）。本项目 `LyraHy.uproject` 中已启用。

### 5.2 让你的类派生自对应基类

把项目里原本直接继承引擎类的地方，改成继承 Modular 版本：

```cpp
// 之前
class AMyCharacter : public ACharacter { ... };

// 之后
#include "ModularCharacter.h"
class AMyCharacter : public AModularCharacter { ... };
```

蓝图同理：创建蓝图时父类选 `ModularCharacter` 而不是 `Character`。

### 5.3 配置 GameMode

让自己的 GameMode 继承 `AModularGameMode`（或 `AModularGameModeBase`），配套的 GameState / PlayerController / PlayerState / Pawn 会自动被设成 Modular 版本。

### 5.4 在 GameFeature 里挂组件

这是本插件真正的价值所在。在 GameFeature 的 Action 里监听 `NAME_GameActorReady`，然后往目标 Actor 上添加组件：

```cpp
UGameFrameworkComponentManager::AddComponentRequest(
    ActorClass,          // 例如 AModularCharacter::StaticClass()
    ComponentClass       // 你要挂上去的组件类
);
```

### 5.5 注意事项

1. **登记必须早于派发**：`AddGameFrameworkComponentReceiver` 在 `PreInitializeComponents` 里调用，就绪事件在 `BeginPlay` 里派发，顺序不能颠倒，否则组件挂不上。
2. **务必调用父类**：这些基类重写都保留了 `Super::` 调用，你自己派生时也请保持。
3. **PlayerController 是特例**：它的就绪事件在 `ReceivedPlayer()` 而非 `BeginPlay`，写 GameFeature 时别按惯性假设。
4. **销毁要注销**：`EndPlay` 里的 `RemoveGameFrameworkComponentReceiver` 不能省，否则 `UGameFrameworkComponentManager` 会持有悬空引用。
5. **本插件不能单独使用**：必须配合 `ModularGameplay` 插件才有意义。
