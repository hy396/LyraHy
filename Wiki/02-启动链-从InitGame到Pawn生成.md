# 02 - 启动链：从 InitGame 到 Pawn 生成

> **一句话概括**：追一次"点击 Play → 玩家可操控角色"之间发生的所有事，这是读 Lyra 的第一条完整航线。

## 前置知识

- [00 - 准备工作与学习心法](00-准备工作与学习心法.md)
- [01 - 项目全景与术语表](01-项目全景与术语表.md)（**强烈要求先读**）

## 这一章读完你能答

1. 点击 PIE 后的那一瞬间，Lyra 按什么顺序做了哪些事？
2. Experience 是在哪里、按什么优先级被选中的？
3. 为什么 Pawn 不是在 `InitGame` 里直接生成，而要"等 Experience 加载"？
4. `HandleStartingNewPlayer` 和 `OnExperienceLoaded` 这两个时间点，哪个先哪个后，为什么？

---

## 一、先看总时序图

下面这张图是本章的"主角"。你读下面每一段时，都回头对照它的哪一步。

```
时间
  │
  │  [PIE 开始，UWorld 初始化完毕]
  │
  ▼
┌─────────────────────────────────────────────────────┐
│ 1. AGameModeBase::InitGame (父类调用)                │
│    ↓                                                 │
│ 2. ALyraGameMode::InitGame                           │
│    - Super::InitGame(...)                            │
│    - SetTimerForNextTick(HandleMatchAssignmentIfNot- │
│      ExpectingOne)  ← 延迟一帧                        │
└─────────────────────────────────────────────────────┘
  │
  ▼  (下一帧)
┌─────────────────────────────────────────────────────┐
│ 3. HandleMatchAssignmentIfNotExpectingOne            │
│    按优先级确定 ExperienceId：                         │
│    ① 匹配系统分配                                     │
│    ② URL Options (?Experience=...)                   │
│    ③ 开发者设置 (仅 PIE)                              │
│    ④ 命令行 (-Experience=...)                        │
│    ⑤ WorldSettings.DefaultGameplayExperience         │
│    ⑥ 默认兜底 (B_LyraDefaultExperience)              │
│    ↓                                                 │
│ 4. OnMatchAssignmentGiven(ExperienceId)              │
│    - ExperienceComponent->SetCurrentExperience(Id)   │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│ 5. ULyraExperienceManagerComponent::                 │
│    SetCurrentExperience → StartExperienceLoad        │
│    - AssetManager 加载 Experience 资产               │
│    - 加载 Experience 声明的所有 GameFeature 插件      │
│    - 执行每个插件的 GameFeatureAction                 │
│      (加能力、加 UI、加输入、加组件...)                 │
└─────────────────────────────────────────────────────┘
  │
  ▼  (全部加载完成后)
┌─────────────────────────────────────────────────────┐
│ 6. OnExperienceFullLoadCompleted                     │
│    - LoadState = Loaded                              │
│    - 触发三个回调:                                    │
│      OnExperienceLoaded_HighPriority                 │
│      OnExperienceLoaded           ← GameMode 监听     │
│      OnExperienceLoaded_LowPriority                  │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│ 7. ALyraGameMode::OnExperienceLoaded                 │
│    - 遍历所有 PlayerController                       │
│    - 对每个 PC 调用 RestartPlayer → 生成 Pawn         │
│      (通过 SpawnDefaultPawnAtTransform_Impl)          │
│    - 设置 PawnData、调用 PawnExtension init state     │
└─────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────┐
│ 8. LyraPawnExtensionComponent InitState 走完          │
│    Spawned → DataAvailable → DataInitialized →       │
│    GameplayReady                                     │
│    (中间 HeroComponent/ASC/Input 依次就绪)             │
└─────────────────────────────────────────────────────┘
  │
  ▼
  [玩家可以动了 🎮]
```

好，我们现在把每一步拆开讲。

---

## 二、第 1-2 步：InitGame 立即返回，延迟一帧

打开 [LyraGameMode.cpp:80-86](../Source/LyraGame/GameModes/LyraGameMode.cpp#L80)：

```cpp
void ALyraGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    // Wait for the next frame to give time to initialize startup settings
    GetWorld()->GetTimerManager().SetTimerForNextTick(
        this, &ThisClass::HandleMatchAssignmentIfNotExpectingOne);
}
```

**关键观察**：`InitGame` 自己几乎什么都没干，只调了父类，然后**延迟一帧**调用 `HandleMatchAssignmentIfNotExpectingOne`。

**为什么延迟一帧？**

注释 "Wait for the next frame to give time to initialize startup settings" 说的就是：`InitGame` 是 UE 框架里极早期的回调，此时命令行参数、URL 参数、`UGameInstance`、在线子系统可能还没完全就绪。延一帧能保证这些东西都初始化好了。

**这是 Lyra 架构的一个重要习惯**：宁可多一帧延迟，也要确保上下文完整。读 Lyra 代码你会多次遇到 `SetTimerForNextTick`、`CallOrRegister_*` 这类"异步地做一件事"的模式。

---

## 三、第 3 步：确定用哪个 Experience

这是**整个启动链最精妙的一步**。`HandleMatchAssignmentIfNotExpectingOne` 按优先级链决定"这局游戏到底玩哪个 Experience"。

打开 [LyraGameMode.cpp:88-165](../Source/LyraGame/GameModes/LyraGameMode.cpp#L88)。简化后的逻辑：

```cpp
void ALyraGameMode::HandleMatchAssignmentIfNotExpectingOne()
{
    FPrimaryAssetId ExperienceId;
    FString ExperienceIdSource;
    
    // 优先级 ②: URL Options 里传的 ?Experience=...
    if (!ExperienceId.IsValid() && UGameplayStatics::HasOption(OptionsString, TEXT("Experience")))
    {
        const FString ExperienceFromOptions = UGameplayStatics::ParseOption(OptionsString, TEXT("Experience"));
        ExperienceId = FPrimaryAssetId(
            FPrimaryAssetType(ULyraExperienceDefinition::StaticClass()->GetFName()),
            FName(*ExperienceFromOptions));
        ExperienceIdSource = TEXT("OptionsString");
    }
    
    // 优先级 ③: 开发者设置（只在 PIE 下生效）
    if (!ExperienceId.IsValid() && World->IsPlayInEditor())
    {
        ExperienceId = GetDefault<ULyraDeveloperSettings>()->ExperienceOverride;
        ExperienceIdSource = TEXT("DeveloperSettings");
    }
    
    // 优先级 ④: 命令行 -Experience=...
    if (!ExperienceId.IsValid())
    {
        FString ExperienceFromCommandLine;
        if (FParse::Value(FCommandLine::Get(), TEXT("Experience="), ExperienceFromCommandLine))
        {
            ExperienceId = FPrimaryAssetId::ParseTypeAndName(ExperienceFromCommandLine);
            // ...
            ExperienceIdSource = TEXT("CommandLine");
        }
    }
    
    // 优先级 ⑤: 从 WorldSettings 读
    if (!ExperienceId.IsValid())
    {
        if (ALyraWorldSettings* TypedWorldSettings = Cast<ALyraWorldSettings>(GetWorldSettings()))
        {
            ExperienceId = TypedWorldSettings->GetDefaultGameplayExperience();
            ExperienceIdSource = TEXT("WorldSettings");
        }
    }
    
    // 检查 AssetId 是否真的在 AssetManager 注册过
    if (ExperienceId.IsValid() && !AssetManager.GetPrimaryAssetData(ExperienceId, Dummy))
    {
        UE_LOG(LogLyraExperience, Error, TEXT("... falling back to the default"));
        ExperienceId = FPrimaryAssetId();  // 作废，走兜底
    }
    
    // 优先级 ⑥: 兜底
    if (!ExperienceId.IsValid())
    {
        // ... 专用服务器处理 ...
        ExperienceId = FPrimaryAssetId(FPrimaryAssetType("LyraExperienceDefinition"),
                                       FName("B_LyraDefaultExperience"));
        ExperienceIdSource = TEXT("Default");
    }
    
    OnMatchAssignmentGiven(ExperienceId, ExperienceIdSource);
}
```

### 为什么要这么多层优先级？

因为 Lyra 要同时支持：

| 场景 | 走哪个优先级 |
|---|---|
| 从主菜单点按钮进入一局 | ②（URL 参数）`?Experience=B_Experience_Elimination` |
| 编辑器里开发某个玩法 | ③（开发者设置）DevSettings 里指定 ExperienceOverride |
| 命令行启动专用服务器 | ④（命令行）`-Experience=B_Experience_ShooterGame` |
| 直接打开某个关卡测试 | ⑤（WorldSettings）关卡本身写了默认 Experience |
| 开发中忘了配置 | ⑥（兜底）永远有个默认值，不会崩溃 |

**设计精髓**：同一套代码支持"玩家使用"和"开发者调试"两种路径。开发时你在编辑器设 `ExperienceOverride`，上线时玩家走 URL Options，专用服务器走命令行。互不干扰。

### `ExperienceIdSource` 有什么用？

注意每个分支都设置了一个 `ExperienceIdSource` 字符串（"OptionsString" / "DeveloperSettings" / ...）。这**只用来打日志**：

```cpp
UE_LOG(LogLyraExperience, Log, TEXT("Identified experience %s (Source: %s)"), 
       *ExperienceId.ToString(), *ExperienceIdSource);
```

**设计细节**：调试一个"为什么启动了错的 Experience"的问题时，看日志就知道是哪个优先级起作用了。这种"每一步都留一个源头标记"的习惯，是大项目的基本素养。

---

## 四、第 4-5 步：交给 ExperienceComponent 加载

确定了 `ExperienceId` 之后：

```cpp
// LyraGameMode.cpp:289-303
void ALyraGameMode::OnMatchAssignmentGiven(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
    if (ExperienceId.IsValid())
    {
        UE_LOG(LogLyraExperience, Log, TEXT("Identified experience %s (Source: %s)"), ...);

        ULyraExperienceManagerComponent* ExperienceComponent = 
            GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
        check(ExperienceComponent);
        ExperienceComponent->SetCurrentExperience(ExperienceId);
    }
}
```

### 重要设计决策：ExperienceComponent 挂在 GameState 上

**为什么是 GameState，不是 GameMode？**

UE 的 `GameMode` **只存在于服务器**。如果你在多人游戏里把 Experience 信息放 GameMode 上，客户端就拿不到。

`GameState` 是**服务器和客户端都有**的，而且属性会自动同步。把 `ExperienceComponent` 挂在 GameState 上意味着：

- 服务器决定"用哪个 Experience"，写进 GameState。
- `CurrentExperience` 字段通过网络复制到所有客户端。
- 客户端收到 `OnRep_CurrentExperience`，自动触发加载。

**这是整个 Lyra 网络架构的基石**。多花一分钟想清楚这件事。

打开 [LyraExperienceManagerComponent.h:82-83](../Source/LyraGame/GameModes/LyraExperienceManagerComponent.h#L82)：

```cpp
UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
TObjectPtr<const ULyraExperienceDefinition> CurrentExperience;
```

`ReplicatedUsing` 是 UE 的网络同步特性：服务器改这个字段 → 客户端收到新值 → 自动调用 `OnRep_CurrentExperience()`。

### SetCurrentExperience 做了什么

打开 cpp（你可以用 F12 跳转到 `ULyraExperienceManagerComponent::SetCurrentExperience` 自己看），关键逻辑是：

1. 把 `CurrentExperience` 字段设置为解析出的 Experience 对象。
2. 调用 `StartExperienceLoad()`。

`StartExperienceLoad` 会：

1. 让 AssetManager **异步加载** Experience 资产本体（因为它可能还没加载）。
2. 加载完后，读 `Experience->GameFeaturesToEnable`（一个 `TArray<FString>`），为每个 GameFeature 插件调用 `GameFeaturePluginManager::LoadAndActivateGameFeaturePlugin(URL)`。
3. 每个插件激活后，它声明的 `GameFeatureAction` 会被执行（`OnGameFeatureActivating`），把能力、UI、输入注入当前世界。
4. 所有插件激活完成 → 调用 `OnExperienceFullLoadCompleted` → 广播三个优先级的 `OnExperienceLoaded` 委托。

### 为什么要三个优先级的回调？

看 [LyraExperienceManagerComponent.h:52-60](../Source/LyraGame/GameModes/LyraExperienceManagerComponent.h#L52)：

```cpp
CallOrRegister_OnExperienceLoaded_HighPriority(...)
CallOrRegister_OnExperienceLoaded(...)
CallOrRegister_OnExperienceLoaded_LowPriority(...)
```

因为**"Experience 加载完成"之后有一堆系统要做初始化**，而这些系统之间也有顺序依赖：

- **HighPriority**：最基础的设置，比如队伍系统、全局子系统。
- **Normal**：GameMode 生成玩家 Pawn。
- **LowPriority**：UI 刷新、音效预热之类的附加工作。

**小心**：看到 `CallOrRegister_*` 这个命名，它的含义是"**如果已经加载完了就立即调，否则注册委托等着加载完调**"。这又是 Lyra 处理"不知道加载有没有完成"的标准范式。

---

## 五、第 6-7 步：Experience 加载完 → 生成 Pawn

GameMode 在 `InitGameState` 里注册了自己的监听：

```cpp
// LyraGameMode.cpp:452-460
void ALyraGameMode::InitGameState()
{
    Super::InitGameState();

    // Listen for the experience load to complete	
    ULyraExperienceManagerComponent* ExperienceComponent = 
        GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
    check(ExperienceComponent);
    ExperienceComponent->CallOrRegister_OnExperienceLoaded(
        FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}
```

Experience 加载完后，回调过来：

```cpp
// LyraGameMode.cpp:305-321
void ALyraGameMode::OnExperienceLoaded(const ULyraExperienceDefinition* CurrentExperience)
{
    // Spawn any players that are already attached
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Cast<APlayerController>(*Iterator);
        if ((PC != nullptr) && (PC->GetPawn() == nullptr))
        {
            if (PlayerCanRestart(PC))
            {
                RestartPlayer(PC);  // 生成 Pawn！
            }
        }
    }
}
```

**关键理解**：Lyra 把"生成 Pawn"这件事**延迟到 Experience 加载完**。因为如果在那之前生成，Pawn 身上连能力、输入映射、UI 都不会有——一个啥都不能干的空壳。

### HandleStartingNewPlayer 的微妙处理

Experience 加载过程中，如果有新玩家登录了（多人游戏很常见），UE 会调用 `HandleStartingNewPlayer`。Lyra 的处理：

```cpp
// LyraGameMode.cpp:391-399
void ALyraGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
    // Delay starting new players until the experience has been loaded
    // (players who log in prior to that will be started by OnExperienceLoaded)
    if (IsExperienceLoaded())
    {
        Super::HandleStartingNewPlayer_Implementation(NewPlayer);
    }
}
```

**精髓**：如果 Experience 还没加载完，**直接不做任何事**。那个玩家会被 `OnExperienceLoaded` 里的循环扫到并生成 Pawn。

这两段代码一起构成了一个"**谁先到都不要紧**"的优雅模式：
- 玩家先到，Experience 后加载 → 走 `OnExperienceLoaded` 里的循环。
- Experience 先加载，玩家后到 → 走 `HandleStartingNewPlayer` 的正常路径。

两种情况殊途同归。**大量的 Lyra 代码都在处理这种"不确定顺序"的问题**，你会慢慢习惯。

---

## 六、Pawn 生成的细节：PawnData 和 PawnExtension

`RestartPlayer` 最终会调到 `SpawnDefaultPawnAtTransform_Implementation`：

```cpp
// LyraGameMode.cpp:345-383
APawn* ALyraGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Instigator = GetInstigator();
    SpawnInfo.ObjectFlags |= RF_Transient;
    SpawnInfo.bDeferConstruction = true;   // ★ 关键：延迟构造

    if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
    {
        if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
        {
            // ★ 在 FinishSpawning 之前，把 PawnData 塞给 PawnExtensionComponent
            if (ULyraPawnExtensionComponent* PawnExtComp = 
                ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
            {
                if (const ULyraPawnData* PawnData = GetPawnDataForController(NewPlayer))
                {
                    PawnExtComp->SetPawnData(PawnData);
                }
            }

            SpawnedPawn->FinishSpawning(SpawnTransform);  // ★ 现在才真正构造完
            return SpawnedPawn;
        }
    }
    return nullptr;
}
```

### 三个关键设计

**1. `bDeferConstruction = true` + `FinishSpawning`**

UE 的 `SpawnActor` 默认会立即调 `BeginPlay`。但这里用了延迟模式：先 Spawn 一个"半成品"，做一些手动设置（把 PawnData 塞进去），**然后才 `FinishSpawning`**，让 Pawn 正式进入 Play。

这样当 Pawn 的 `BeginPlay` 和各组件 `InitializeComponent` 执行时，它们**已经能看到 PawnData**，不用等下一帧。

**2. PawnClass 是从 PawnData 里读出来的**

```cpp
// LyraGameMode.cpp:332-343
UClass* ALyraGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    if (const ULyraPawnData* PawnData = GetPawnDataForController(InController))
    {
        if (PawnData->PawnClass)
        {
            return PawnData->PawnClass;
        }
    }
    return Super::GetDefaultPawnClassForController_Implementation(InController);
}
```

而 `GetPawnDataForController` 会先查 `PlayerState` 上的 `PawnData`（比如玩家选了某个英雄），查不到就用 Experience 的 `DefaultPawnData`。

**所以 Pawn 的类不是在 GameMode 构造时硬编码的**，而是从 Experience + PlayerState 动态决定的。换 Experience = 换 Pawn 类，不用改代码。

**3. PawnExtensionComponent 是"组件协调官"**

`PawnExtComp->SetPawnData(PawnData)` 一行看似简单，实际触发了一长串初始化链（InitState 状态机推进）。具体机制见 [05 - InitState 生命周期契约](05-InitState生命周期契约.md)，本章不展开。

你现在只需要知道：**PawnExtension 是 Pawn 身上的"总调度者"**，所有其他组件（HeroComponent、AbilitySystem、Input）的初始化都围绕它转。

---

## 七、完整调用链（精简版）

下面是你读完这一章应该能默写的调用链（背出来你就入门了）：

```
PIE 启动
  ↓
ALyraGameMode::InitGame
  ↓ (next tick)
HandleMatchAssignmentIfNotExpectingOne
  ↓
OnMatchAssignmentGiven
  ↓
ExperienceComponent->SetCurrentExperience(Id)
  ↓
StartExperienceLoad (异步)
  ├── AssetManager 加载 ExperienceDefinition
  ├── 加载 Experience->GameFeaturesToEnable 里的插件
  └── 每个插件激活，执行所有 GameFeatureAction
  ↓
OnExperienceFullLoadCompleted
  ↓
ALyraGameMode::OnExperienceLoaded  (注册时机: InitGameState)
  ↓
RestartPlayer(PC) 对每个现有玩家
  ↓
SpawnDefaultPawnAtTransform_Implementation
  ├── GetDefaultPawnClassForController  (读 PawnData.PawnClass)
  ├── SpawnActor (bDeferConstruction=true)
  ├── PawnExtComp->SetPawnData(PawnData)
  └── FinishSpawning
  ↓
Pawn BeginPlay + 各组件 InitializeComponent
  ↓
LyraPawnExtensionComponent 驱动 InitState 状态机
  NONE → Spawned → DataAvailable → DataInitialized → GameplayReady
  ↓
[玩家可以控制 🎮]
```

---

## 八、动手练习

**练习 02.1**（简单）：在 [LyraGameMode.cpp:80](../Source/LyraGame/GameModes/LyraGameMode.cpp#L80) 的 `InitGame` 里加一行 `UE_LOG(LogTemp, Warning, TEXT("InitGame called, MapName=%s"), *MapName);`，重新编译，在 PIE 时观察 Output Log。你应该能看到它在每次 PIE 开始时打印一次。

**练习 02.2**（中等）：在 `OnMatchAssignmentGiven` 的 `if (ExperienceId.IsValid())` 分支里，记录当前 Experience 的名字和来源。然后：
- 正常 PIE 一次，看 Source。
- 修改 `ULyraDeveloperSettings` 的 `ExperienceOverride`（编辑器菜单 Edit → Editor Preferences → 搜"Lyra Developer Settings"），再 PIE，看 Source 变了没。
- 在 `LyraStarterGame.uproject` 的启动命令里加 `-Experience=B_TopDownArenaExperience`，命令行启动，看是否走 CommandLine 分支。

**练习 02.3**（有挑战）：画一张 Mermaid 时序图，把"从 InitGame 到 Pawn 生成"的完整流程画出来。必须包含的角色：`GameMode` / `GameState` / `ExperienceComponent` / `AssetManager` / `GameFeaturesSubsystem` / `PawnExtensionComponent`。

把图贴到你的学习笔记里。如果画不出来，说明本章没读透，回去重读再画。

**练习 02.4**（思考题）：如果你想**中途切换 Experience**（比如一局游戏结束进入下一局），Lyra 现在的设计支持吗？需要改哪些地方？

<details>
<summary>提示</summary>

看 [LyraExperienceManagerComponent.h:26-27](../Source/LyraGame/GameModes/LyraExperienceManagerComponent.h#L26)，注意有个 `Deactivating` 状态。实际 Lyra 的 ExperienceManager 已经处理了卸载流程（`OnActionDeactivationCompleted` / `OnAllActionsDeactivated`）。真正难点在于：卸载时要把 GameFeature 激活时注入的东西**全部清理干净**——这就靠 `GameFeatureAction::OnGameFeatureDeactivating` 的对称性了。

练习要求你去读一遍 ExperienceManagerComponent.cpp，找出它处理 Experience 切换的代码。

</details>

---

## 九、常见疑问

### Q1：为什么 `HandleMatchAssignmentIfNotExpectingOne` 这么长的名字？

因为还有一个**反向路径**：如果 GameMode 正在"等待外部分配 Experience"（比如等匹配服务器告诉你"玩家要玩哪张图"），它就**不应该**自己决定 Experience。这个长名字在说："**只有在我没在等外部分配时，我才自己搞定**"。

现在的代码里这个分支逻辑不是特别显式，但名字留着这个意图。

### Q2：`Super::InitGame` 都做了啥？

标准 UE 流程：解析 URL Options、创建 GameSession、创建 GameState。你不用管这里面的细节，Lyra 的扩展都在 `Super` 返回之后发生。

### Q3：主菜单进入游戏时是怎么切关卡的？

主菜单关卡是 `L_LyraFrontEnd`。点击 "Play Lyra" 按钮会走 `CommonUserSubsystem` / `CommonSessionSubsystem` 的流程（`CommonGame` 插件），最终调 `UGameplayStatics::OpenLevel` 或类似 API 跳到目标关卡，并在 URL Options 里带上 `?Experience=...`。

这个流程在 [LyraUserFacingExperienceDefinition](../Source/LyraGame/GameModes/LyraUserFacingExperienceDefinition.h) 和 `CommonSession_HostSessionRequest` 里。下一章会稍微展开。

### Q4：如果我只想测一个 Experience，不用跑主菜单怎么办？

最快的方式：
1. 打开 Edit → Project Settings → Lyra → Developer Settings。
2. 找到 `Experience Override`，选你要测的 Experience。
3. 直接在随便一个玩法关卡（比如 `L_ExpanseTestMap`）里按 PIE。

这样会走第 3 步优先级 ③，跳过主菜单直接加载你指定的 Experience。

---

## 十、自检清单

- [ ] 我能默写"从 PIE 开始到 Pawn 可操作"的主要调用链（至少 8 步）。
- [ ] 我知道 `HandleMatchAssignmentIfNotExpectingOne` 里的六层优先级顺序。
- [ ] 我能说出"为什么 ExperienceComponent 挂在 GameState 上而不是 GameMode 上"。
- [ ] 我做完了练习 02.3（画完了时序图）。
- [ ] 我知道 `bDeferConstruction + FinishSpawning` 这个 pattern 的意义。
- [ ] 我知道为什么 Pawn 生成要等 Experience 加载。

---

## 下一章

→ [03 - Experience 系统详解](03-Experience系统详解.md)

我们专门扎进 `ULyraExperienceDefinition` 本身：它的数据结构长什么样、`ActionSets` 是怎么递归组合的、一个 Experience 如何通过 `GameFeaturesToEnable` 拉起一堆插件。

再下一章（04）会接着讲 Game Features 机制本身。这两章是 Lyra 架构的"心脏"。
