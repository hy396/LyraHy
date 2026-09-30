# 05 - InitState 生命周期契约

> **一句话概括**：Lyra 用一个五态状态机统一协调 Pawn 身上所有组件的启动顺序，让新组件可以"插队"到流程里而不打破依赖。

## 前置知识

- [04 - GameFeatures 与 ModularGameplay](04-GameFeatures与ModularGameplay.md)

## 这一章读完你能答

1. `InitState` 的五个状态分别是什么含义？
2. `PawnExtensionComponent` 如何驱动其他组件前进？
3. 我要给 Pawn 加一个新组件时，它的 `CanChangeInitState` 应该返回什么？
4. 为什么 Lyra 不直接用 `BeginPlay` / `InitializeComponent` 来管初始化？

---

## 待撰写的内容提纲

### 一、为什么 BeginPlay 不够用

- UE 原生生命周期的问题：组件间没有协调机制
- HeroComponent 要在 ASC 就绪后才能绑按键 → 顺序问题
- 网络客户端 vs 服务端 vs 独立游戏：到达 BeginPlay 的时机不同

### 二、五个状态

```
NONE → Spawned → DataAvailable → DataInitialized → GameplayReady
```

每个状态的语义、谁有资格推进到下一状态。

### 三、ULyraPawnExtensionComponent 的核心

- `CanChangeInitState` / `HandleChangeInitState`
- 组件注册时机（`OnRegister`）
- 推进所有组件一起前进的循环
- `PawnData` 的两个触发时刻

### 四、跟随状态机的几个主力组件

- `ULyraHeroComponent`（玩家专属）
- `ULyraPawnExtensionComponent`（协调者本身）
- `ULyraAbilitySystemComponent`（挂在 PlayerState）

### 五、动手练习

- 写一个 `UMyPawnComponent` 实现 `IGameFrameworkInitStateInterface`
- 声明它依赖 `HeroComponent` 到 `DataInitialized` 才能前进
- 观察启动时日志，看推进顺序

### 六、排错指南

- 常见问题：InitState 卡住了
- 调试方法：Visual Logger、`GameFrameworkComponentManager.DebugShowComponentManager`
- 典型坑：忘了在 `CanChangeInitState` 里返回 true

---

**⚠️ 本章尚未撰写**。InitState 是 Lyra "写自定义组件" 时一定要懂的机制，写起来需要画一张组件依赖图。
