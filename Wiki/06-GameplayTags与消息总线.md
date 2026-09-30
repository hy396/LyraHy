# 06 - GameplayTags 与消息总线

> **一句话概括**：Lyra 的"神经系统"——GameplayTag 是各系统共享的词汇表，GameplayMessage 是系统之间互相"说话"的广播电台。

## 前置知识

- [05 - InitState 生命周期契约](05-InitState生命周期契约.md)

## 这一章读完你能答

1. GameplayTag 的层级结构有什么用？为什么不用普通字符串或枚举？
2. Lyra 在哪些地方"声明"了自己要用的 Tag？
3. GameplayMessageSubsystem 的发布/订阅 API 怎么用？
4. `LyraVerbMessage` 和 `LyraNotificationMessage` 什么区别？

---

## 待撰写的内容提纲

### 一、GameplayTag 基础

- `FGameplayTag` vs `FGameplayTagContainer` vs `FGameplayTagQuery`
- 层级匹配："Ability.Movement" 能匹配到 "Ability.Movement.Sprint"
- 为什么比字符串 / 枚举好

### 二、Lyra 的 Tag 组织

- [LyraGameplayTags.h](../Source/LyraGame/LyraGameplayTags.h) 的结构
- `Config/DefaultGameplayTags.ini` / `.ini` 文件里的 Tag 定义
- 命名约定：Status / Ability / InitState / GameplayEvent / Message

### 三、GameplayMessageRouter 插件

- `UGameplayMessageSubsystem` 的核心 API
- 发布：`BroadcastMessage(Tag, Message)`
- 订阅：`RegisterListener(Tag, Callback)`
- 匹配规则：`ExactMatch` / `PartialMatch`

### 四、Lyra 里的典型消息

- `LyraVerbMessage`：动词式事件（"玩家击杀敌人"）
- `LyraNotificationMessage`：通知（"补给送到了"）
- 用途：UI 刷新、数据统计、音效触发

### 五、动手练习

- 注册一个监听 `"Lyra.Elimination"` 消息的 Subsystem
- 调试 Tag 用 `ShowDebug AbilitySystem` 在 PIE 看 ASC 持有的 Tag
- 全局搜 `Status.Death` 这个 Tag，列出所有地方

### 六、设计原则

- 什么场景应该用 Tag，什么场景用 Message，什么场景用传统委托
- Tag 泛滥的陷阱

---

**⚠️ 本章尚未撰写**。
