# 09 - Pawn / Character / HeroComponent 三层

> **一句话概括**：Lyra 把"玩家角色"拆成三层——最底是通用 Pawn，中间是会走路会动画的 Character，上面是只给真人玩家用的 HeroComponent——让 Bot 和玩家共享底座、分化上层。

## 前置知识

- [05 - InitState 生命周期契约](05-InitState生命周期契约.md)
- [08 - Lyra 的 GAS 扩展](08-Lyra的GAS扩展.md)

## 这一章读完你能答

1. `ALyraPawn` / `ALyraCharacter` / `ULyraHeroComponent` 三者职责分别是什么？
2. 为什么 ASC 不挂在 Pawn 上，而挂在 PlayerState 上？
3. `PawnData` 是怎么被消费的？从 Experience 传到 Pawn 中间经过哪些手？
4. Bot 和真人玩家在启动时走的代码路径有什么差别？

---

## 待撰写的内容提纲

### 一、继承层级

```
APawn (UE)
  ↓
AModularPawn (ModularGameplay 插件)
  ↓
ALyraPawn  ← 含 PawnExtensionComponent，支持 Bot
  ↓
ACharacter (UE)    [分支合并]
  ↓
AModularCharacter
  ↓
ALyraCharacter  ← 玩家/NPC 通用角色，含 CharacterMovement
```

### 二、PawnData 的分发路径

- Experience.DefaultPawnData → GameMode.GetPawnDataForController
- → SpawnDefaultPawnAtTransform → PawnExtComp.SetPawnData
- → InitState 推进时，PawnData 被 HeroComponent / AbilitySystem 读取

### 三、LyraHeroComponent：玩家专属功能

- 绑定 Enhanced Input → Ability
- 相机模式切换
- 为什么 Bot 不需要它

### 四、ASC 在 PlayerState 上的设计

- 玩家死亡重生时 ASC 数据延续（冷却、Buff）
- Bot 的 ASC 也挂 PlayerState（`ALyraPlayerBotState`）
- 代价：Pawn 访问 ASC 要多一层 `GetAbilitySystemComponent()`

### 五、死亡与重生

- 死亡 Ability 的触发
- `ULyraHealthComponent` 的作用
- 玩家重生：保留 PlayerState → 重新 SpawnPawn
- Bot 重生：另一套流程

### 六、动手练习

- 自定义一个 PawnData：换模型、换默认 AbilitySet
- 写一个新的 `UMyExtensionComponent` 跟进 InitState 打印
- 切换 Experience 的 DefaultPawnData，观察 Pawn 类变化

---

**⚠️ 本章尚未撰写**。
