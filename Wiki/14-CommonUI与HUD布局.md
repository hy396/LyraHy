# 14 - CommonUI 与 HUD 布局

> **一句话概括**：CommonUI 是 UE 官方的现代 UI 框架，提供"可激活 Widget 栈 + 自动输入路由"；Lyra 的 HUDLayout 把 HUD 拆成可扩展的"槽位"，由 GameFeatures 动态填充。

## 前置知识

- [06 - GameplayTags 与消息总线](06-GameplayTags与消息总线.md)
- [10 - Player 三剑客与数据流](10-Player三剑客与数据流.md)

## 这一章读完你能答

1. `CommonActivatableWidget` 和普通 `UUserWidget` 有什么差别？
2. Lyra 的 HUD 扩展点是怎么实现的？
3. UI 如何通过 GameplayMessage 订阅属性变化？
4. 主菜单（FrontEnd）的 Widget 栈长什么样？

---

## 待撰写的内容提纲

### 一、CommonUI 核心概念

- `UCommonActivatableWidget`：可激活 / 可退出 / 可堆叠
- `UCommonActivatableWidgetStack`：垂直堆叠（Menu stack）
- `UCommonActivatableWidgetQueue`：队列（对话框）
- 输入路由：UI 层吸收输入 vs 传给游戏

### 二、LyraActivatableWidget

- 附加支持：InputModeOverride / GameLayerName
- 自动推 CameraMode（按下菜单时切到 UI 相机）

### 三、HUD 扩展点（UIExtensionSubsystem）

- `RegisterExtensionAsWidgetForContext`：把 Widget 注册到某个 "ExtensionPoint"
- HUDLayout 里声明 ExtensionPoint（Tag）
- GameFeature 激活时自动填充

### 四、UI 与游戏数据的连接

- GameplayMessage 订阅：UI 监听血量变化消息，自动刷新
- Attribute 变化：监听 `OnAttributeChanged`
- 避免直接 tick + 拉数据

### 五、FrontEnd 主菜单

- `L_LyraFrontEnd` 关卡
- `W_LyraFrontEnd` 根 Widget
- UserFacingExperience 列表

### 六、动手练习

- 写一个 `W_MyHUDIndicator`，订阅 `Lyra.Elimination` 消息显示连杀数
- 用 `GameFeatureAction_AddWidget` 在某个 Feature 激活时加上
- 实现一个 ESC 菜单（入栈 → Pop 返回）

---

**⚠️ 本章尚未撰写**。
