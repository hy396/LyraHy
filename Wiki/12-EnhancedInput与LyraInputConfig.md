# 12 - Enhanced Input 与 LyraInputConfig

> **一句话概括**：UE 5 的 Enhanced Input 加上 Lyra 的 `LyraInputConfig`，把"按键 → GameplayTag → Ability"这条链用数据资产串起来，换键无需改代码。

## 前置知识

- [10 - Player 三剑客与数据流](10-Player三剑客与数据流.md)

## 这一章读完你能答

1. Enhanced Input 的 IA / IMC / Trigger / Modifier 分别做什么？
2. `LyraInputConfig` 如何把 InputAction 和 GameplayTag 配对？
3. 动态切换输入映射（切武器、进 UI）是怎么做到的？
4. 键位重映射是在哪里实现的？

---

## 待撰写的内容提纲

### 一、Enhanced Input 基础

- InputAction（IA）：一个抽象动作，如 "Fire" / "Jump"
- InputMappingContext（IMC）：键位映射表
- Trigger：什么情况下触发（Pressed / Held / Combo）
- Modifier：值修改器（死区、曲线、反转）

### 二、LyraInputConfig

- 数据资产结构：Native Actions / Ability Actions
- Ability Actions 的特殊语义：每个 IA 绑一个 Ability Tag

### 三、LyraInputComponent 的绑定逻辑

- `BindAbilityActions` 怎么读 InputConfig
- Ability Input Press/Release 如何转发到 ASC

### 四、LyraHeroComponent 中的绑定时机

- 必须等 InitState 到 `DataInitialized` 才能绑
- 通过 `GameFeatureAction_AddInputBinding` 动态追加

### 五、IMC 的动态切换

- `GameFeatureAction_AddInputContextMapping` 的作用
- 车辆 vs 步行 vs UI 时不同的 IMC
- 优先级机制

### 六、Enhanced Input User Settings（重映射）

- Lyra 支持玩家重映射按键
- 配置持久化

### 七、动手练习

- 给 Ability 绑一个新按键（不改代码，改 InputConfig）
- 用 `GameFeatureAction_AddInputBinding` 在某个 Feature 激活时追加一个按键
- 实现一个只在"蹲下"状态下有效的按键

---

**⚠️ 本章尚未撰写**。
