# 04 - GameFeatures 与 ModularGameplay

> **一句话概括**：UE 原生的"可插拔插件"机制，Lyra 靠它做到"玩法变更 = 插件切换，代码不动"。

## 前置知识

- [03 - Experience 系统详解](03-Experience系统详解.md)

## 这一章读完你能答

1. `UGameFeatureData` / `UGameFeatureAction` / `UGameFrameworkComponentManager` 三者的关系。
2. `GameFeatureAction_AddAbilities` 是怎么把能力"注入"到符合条件的 Pawn 上的？
3. `ModularGameplay` 插件的核心概念（`ReceiverClass` / `Extension Events`）。
4. 为什么 Lyra 自己定义了 `GameFeatureAction_WorldActionBase`？

---

## 待撰写的内容提纲

### 一、GameFeatures 插件的"一生"

- 插件状态机：`Installed → Registered → Loaded → Active`
- 每个阶段对应的回调

### 二、GameFeatureAction 家族

- Lyra 自定义的 Actions：
  - `GameFeatureAction_AddAbilities`
  - `GameFeatureAction_AddInputBinding`
  - `GameFeatureAction_AddInputContextMapping`
  - `GameFeatureAction_AddWidget`
  - `GameFeatureAction_SplitscreenConfig`
  - `GameFeatureAction_WorldActionBase`（基类）
- 每个 Action 的 `OnGameFeatureActivating` / `OnGameFeatureDeactivating` 做什么

### 三、ModularGameplay：组件级的动态注入

- `UGameFrameworkComponentManager` —— 中央注册处
- "Receiver" 概念：哪类 Actor 可以接受动态组件
- Extension Events：组件就绪后广播事件
- Lyra 如何和 InitState 状态机配合

### 四、动手练习：最小 GameFeature 插件

从零建一个插件 `MyFirstFeature`：

1. 新建 `Plugins/GameFeatures/MyFirstFeature/MyFirstFeature.uplugin`
2. 写一个 `UMyPrintLogAction : UGameFeatureAction`
3. 在 Activating 时打印日志、Deactivating 时也打印
4. 在 Experience 里引用它，观察日志顺序

### 五、动手练习：给现有角色加一个新能力

- 用 `GameFeatureAction_AddAbilities` 给 `ALyraCharacter` 加一个新 Ability（比如"喊叫一声"）
- 不改 LyraGame 模块代码
- 验证：切到不包含这个 Feature 的 Experience 后，能力自动消失

---

**⚠️ 本章尚未撰写**。这是整个 Wiki 最关键的一章（和 02 并列），写起来会占 5000+ 字。告诉我"继续"我就开工。
