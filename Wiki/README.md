# LyraStarterGame 学习 Wiki

> 一份把 Lyra 这个"大怪兽"拆开揉碎喂到嘴边的中文学习手册。
> 读者假设：懂 UE 基本概念（Actor/Pawn/Component）、会 C++ 基本语法。
> 目标：3 个月后能独立用 Game Feature 扩展 Lyra，并把它的架构抽象出来用到自己的项目。

---

## 这份 Wiki 为什么存在

直接打开 Lyra 的 C++ 工程，你会看到几十个 `Lyra*` 开头的类、十几个插件、满屏的 `GameplayTag`，**不知道从哪里开始读**。官方文档讲了"有什么"但没讲"为什么这样设计"，网上的教程要么讲蓝图操作（跳过了架构），要么直接硬上 GAS（跳过了 Experience）。

这份 Wiki 的目标是回答**"作为一个读者，我该按什么顺序读什么代码，每段代码为什么这么写"**。

每一章的格式都是：

1. **一句话概括** —— 这一章到底在讲什么。
2. **前置知识** —— 没读过哪一章就别读这章。
3. **故事讲解** —— 用白话把这个系统的"剧情"讲一遍。
4. **代码逐行剖析** —— 挑最关键的几段 Lyra 源码，标注每行的意图。
5. **动手练习** —— 不做不算读完。
6. **自检清单** —— 能答上来才算真懂。
7. **延伸阅读** —— 想更深的人看这里。

---

## 阅读顺序

**严格按编号阅读**。跳着读会在后面撞墙——Lyra 的系统是层层叠起来的。

### 第一部分：入门与全景（1-2 周）

- [00 - 准备工作与学习心法](00-准备工作与学习心法.md) — 怎么把项目跑起来，读代码该带什么姿势。
- [01 - 项目全景与术语表](01-项目全景与术语表.md) — Experience / PawnData / GameFeature / Ability / InitState，**先把话听懂**。
- [02 - 启动链：从 InitGame 到 Pawn 生成](02-启动链-从InitGame到Pawn生成.md) — 追一次"点击 Play 到可以操作"的完整路径。

### 第二部分：架构骨架（3-5 周）

- [03 - Experience 系统详解](03-Experience系统详解.md) — Lyra 的"游戏配置入口"。
- [04 - GameFeatures 与 ModularGameplay](04-GameFeatures与ModularGameplay.md) — 为什么 Lyra 能做到"不改一行代码换玩法"。
- [05 - InitState 生命周期契约](05-InitState生命周期契约.md) — Pawn 怎么"逐步醒来"的。
- [06 - GameplayTags 与消息总线](06-GameplayTags与消息总线.md) — Lyra 的"神经系统"。

### 第三部分：Gameplay Ability System（6-9 周）

- [07 - GAS 六大概念速成](07-GAS六大概念速成.md) — ASC / Attribute / Effect / Ability / Task / Cue。
- [08 - Lyra 的 GAS 扩展](08-Lyra的GAS扩展.md) — AbilitySet / TagRelationshipMapping 这些"独家发明"。

### 第四部分：角色、玩家、装备、武器（10-13 周）

- [09 - Pawn / Character / HeroComponent 三层](09-Pawn-Character-Hero三层.md) — 继承 vs 组合的选择。
- [10 - Player 三剑客与数据流](10-Player三剑客与数据流.md) — Controller / State / LocalPlayer 各管什么。
- [11 - 装备与武器系统](11-装备武器系统.md) — 从装上到开火的完整链路。

### 第五部分：表现层（14-16 周）

- [12 - Enhanced Input 与 LyraInputConfig](12-EnhancedInput与LyraInputConfig.md) — 把按键和能力缝合起来。
- [13 - 相机模式堆栈](13-相机模式堆栈.md) — Lyra 做得最精巧的小系统之一。
- [14 - CommonUI 与 HUD 布局](14-CommonUI与HUD布局.md) — 现代 UE UI 的正确打开方式。

### 第六部分：进阶（17-18 周）

- [15 - 网络复制要点](15-网络复制要点.md) — 属性同步、RPC、预测。

### 第七部分：实战（贯穿全程）

- [16 - 实战：第一个自定义 GameFeature](16-实战-第一个自定义GameFeature.md) — 做一个只加日志的最小插件。
- [17 - 实战：自定义 Experience 与武器](17-实战-自定义Experience与武器.md) — 综合运用前面所有章节。

### 附录

- [附录 A - 调试工具箱](附录A-调试工具箱.md) — GameplayDebugger / Visual Logger / Insights / stat 命令。
- [附录 B - 常见陷阱与FAQ](附录B-常见陷阱与FAQ.md) — 踩坑与避坑。

---

## 怎么用这份 Wiki

- **每天一小时，一周一章**：别贪快。Lyra 的每个概念都需要在脑子里发酵。
- **边读边开编辑器**：Wiki 里列的文件路径是可点击的（相对于项目根目录）。读到哪个类，就在 VS Code / Rider 里打开对照。
- **做不出练习不要跳过**：每章末尾的练习是检验理解的关键。做不出来说明前面没读懂，回去重读。
- **遇到读不懂的段落**：标记下来，读完整章再回来看。很多概念是"循环定义"的，第二遍会突然明白。

---

## 项目结构速查（不要记，会用即可）

```
d:\111\LyraStarterGame\
├── Source\LyraGame\              # 主 C++ 模块
│   ├── AbilitySystem\           # GAS 扩展
│   ├── Character\               # Pawn / Character / HeroComponent
│   ├── Equipment\               # 装备系统
│   ├── GameFeatures\            # 自定义 GameFeatureAction
│   ├── GameModes\               # GameMode / Experience / WorldSettings ★入口
│   ├── Input\                   # Enhanced Input 配置
│   ├── Inventory\               # 物品栏
│   ├── Messages\                # GameplayMessage
│   ├── Player\                  # Controller / State / LocalPlayer
│   ├── UI\                      # HUD / Widget
│   └── Weapons\                 # 武器实例
├── Source\LyraEditor\            # 编辑器扩展（暂不管）
├── Plugins\GameFeatures\         # 游戏玩法插件（ShooterCore 等）
├── Plugins\...                   # 其他 Lyra 自带插件
├── Content\                      # 资源（Map/Blueprint/Texture）
├── Config\                       # 配置 (.ini)
└── Wiki\                         # 你现在在这里 ★
```

---

## 版本说明

- **项目版本**：LyraStarterGame (UE 5.7)
- **Wiki 版本**：v1.0 (2026-04)
- **作者**：Claude Code 协作生成，阅读时如发现代码与 Wiki 描述不一致，**以代码为准**，并视为 Wiki 待修订点。

---

开始吧：[00 - 准备工作与学习心法](00-准备工作与学习心法.md) →
