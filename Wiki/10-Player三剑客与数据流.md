# 10 - Player 三剑客与数据流

> **一句话概括**：PlayerController / PlayerState / LocalPlayer 三个类看起来都叫"玩家"，各管一块——本章把它们的职责和数据流向彻底讲清。

## 前置知识

- [09 - Pawn / Character / HeroComponent 三层](09-Pawn-Character-Hero三层.md)

## 这一章读完你能答

1. PC / PS / LocalPlayer 分别是"谁的"，什么时候创建，什么时候销毁？
2. 玩家信息（名字、等级、选的英雄）应该放在哪里？
3. `LyraPlayerSpawningManagerComponent` 做什么用？
4. `ALyraPlayerBotController` 和 `ALyraPlayerController` 有什么差别？

---

## 待撰写的内容提纲

### 一、三者的定位

- `LocalPlayer`：本地客户端的"人"——鼠标、手柄、账号、存档
- `PlayerController`：玩家对游戏世界的"手柄"——输入、Pawn possess、HUD
- `PlayerState`：网络同步的"数据卡"——分数、队伍、ASC、PawnData

### 二、生命周期对比表

| 事件 | LocalPlayer | PC | PS |
|---|---|---|---|
| 登录 | 已存在 | 创建 | 创建 |
| 换地图 | 保留 | 重建 | 重建（或保留） |
| 死亡重生 | 不变 | 不变 | 不变 |
| 断线重连 | 重建 | 重建 | 重建 |

### 三、Lyra 的具体类

- [ALyraPlayerController.h](../Source/LyraGame/Player/LyraPlayerController.h)
- [ALyraPlayerState.h](../Source/LyraGame/Player/LyraPlayerState.h)
- [ULyraLocalPlayer.h](../Source/LyraGame/Player/LyraLocalPlayer.h)
- `ALyraPlayerBotController.h` —— AI 专用 Controller

### 四、数据流：按键如何到达 Ability

1. 用户按键
2. LocalPlayer.EnhancedInputLocalPlayerSubsystem 分发
3. PC.InputComponent (LyraInputComponent)
4. LyraHeroComponent 已绑定的 AbilityTag
5. PS.AbilitySystemComponent.TryActivateAbilityByTag

### 五、PlayerSpawningManager

- 为什么 GameMode 不直接选出生点
- 队伍感知的选点算法
- 死亡后的重生规则

### 六、动手练习

- 在 PS 上加一个 `ReplicatedSkillPoints` 字段，服务端修改，客户端 UI 刷新
- 查 LocalPlayer 子系统列表（Subsystems）
- 写一个 Subsystem 监听 PC 创建事件

---

**⚠️ 本章尚未撰写**。
