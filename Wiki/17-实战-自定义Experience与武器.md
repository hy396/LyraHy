# 17 - 实战：自定义 Experience 与武器

> **一句话概括**：综合运用前 16 章所学，做一个完整的"我的玩法"——自定义 Experience + 自定义 PawnData + 自定义武器 + 自定义 HUD，全部放在一个独立 GameFeature 插件里，**不改 LyraGame 模块一行代码**。

## 前置知识

- [16 - 实战：第一个自定义 GameFeature](16-实战-第一个自定义GameFeature.md)
- 理论上应该前 15 章都读完

## 这一章读完你能干

- 独立设计并实现一个完整 Lyra 玩法扩展
- 在主菜单看到你的新 UserFacingExperience 按钮
- 玩家进入你的玩法：用你的角色、你的武器、你的 HUD
- 退出后干净卸载，不影响其他玩法

---

## 待撰写的内容提纲

### 总体目标：做个"能量武器模式"

- 独立插件 `EnergyWeaponMode`
- 玩家使用一把"能量步枪"（无弹药，有过热机制）
- HUD 显示过热进度条
- 专属关卡（或复用现有关卡）

### 步骤 1：搭骨架

- 建插件 `EnergyWeaponMode`
- 建 `LyraExperienceDefinition` 子类资产
- 建 `LyraPawnData` 资产

### 步骤 2：做武器

- 继承 `ULyraRangedWeaponInstance` → `UEnergyWeaponInstance`
- 新增 `Overheat` Attribute
- 新增 `GE_Weapon_Overheat` 每发热 5 点
- 新增 `GE_Weapon_Cooldown` 每秒冷却 10 点
- 开火 Ability：`HeatCost > 100` 时激活失败

### 步骤 3：做 HUD

- 新 Widget `W_OverheatBar`
- 订阅 `Attribute.Overheat` 变化
- 注册到 HUDLayout 的 ExtensionPoint

### 步骤 4：做 UserFacingExperience

- 引用步骤 1 的 Experience
- 设置图标和名字
- 在主菜单可见

### 步骤 5：跑一遍并调试

- 主菜单 → 点击你的按钮
- 验证角色/武器/HUD 都是新的
- 用 GameplayDebugger 看 Attribute
- 切回去 → 验证卸载干净

### 毕业作业

- 不看教程，给你的模式加"狙击镜"功能
- 需要：新 Ability + 新 CameraMode + 新 UI 元素
- 仍然不改 LyraGame 模块

---

**⚠️ 本章尚未撰写**。这是 Wiki 的"毕业论文"。能独立做完说明你真的学会了 Lyra。
