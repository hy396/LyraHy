# LyraRPG

> 基于 **Unreal Engine 5.6 Lyra Starter Game** 的 RPG 玩法扩展工程
> 自研 `RPGCore` Game Feature 插件（对标 Lyra 内置的 `ShooterCore`），走 **GAS + Modular Gameplay** 路线实现 RPG 玩法框架。

| 项目 | 说明 |
| --- | --- |
| 引擎版本 | Unreal Engine 5.6 |
| 基础框架 | Epic Lyra Starter Game |
| 开发语言 | C++ / Blueprint |
| 方向 | Gameplay 玩法开发 |
| 简历项目名 | **LyraRPG —— 基于 Lyra 的 RPG 玩法框架（UE 5.6 / C++）** |

---

## ⚠️ 关于授权

本项目基于 **Epic Games 的 Lyra Starter Game** 二次开发，Lyra 原始代码与内容版权归 Epic Games 所有，
遵循 [Unreal Engine EULA](https://www.unrealengine.com/eula)。

- 本仓库**不包含**任何 Epic 商城付费资产；
- 本仓库的原创部分为 `Plugins/GameFeatures/RPGCore/` 及相关 RPG 内容与代码改动；
- 如果你 Fork 或参考本工程，请在自己的 README 中同样注明 Lyra 来源。

---

## 我做了什么（与原生 Lyra 的差异）

这一节是重点 —— 只看 Lyra 本体没有意义，以下内容是本工程的**增量工作**。

### 1. `RPGCore` Game Feature 插件（原创，对标 ShooterCore）

按 Lyra 官方 `ShooterCore` 的组织方式，从零搭了一个 RPG 玩法插件，
用 **Game Feature + Modular Gameplay** 的方式挂载，不侵入 Lyra 核心代码：

```
Plugins/GameFeatures/RPGCore/
├── RPGCore.uplugin                       # 插件定义（BuiltInInitialFeatureState = Active）
├── Content/RPGCore.uasset                # Game Feature Data 资产
├── Content/Experiences/B_RPGExperience.uasset   # RPG 玩法体验（Experience Definition）
├── Content/Game/HeroPawnData.uasset      # RPG 主角 PawnData
├── Content/Game/Characters/B_Hero_Hy.uasset     # RPG 主角蓝图
├── Content/Maps/L_RPGGym.umap           # RPG 玩法测试关卡
└── Resources/Icon128.png
```

**设计要点**：Lyra 的 Experience 系统按
`Unloaded → Loading(资产) → LoadingGameFeatures(插件) → ExecutingActions → Loaded`
的流水线加载；`RPGCore` 作为 Game Feature 在其中被激活，
通过 Experience 的 Action 列表注入 RPG 的 PawnData、输入与能力集，
从而做到**玩法可插拔**——关掉插件即回到原生射击玩法。

### 2. RPG 输入配置

基于 Lyra 的 `ULyraInputConfig` + Enhanced Input 体系，新增 RPG 专用输入：

- `Content/Input/InputData_Hero_RPG.uasset`
- `Content/Input/Mappings/IMC_Default_RPG.uasset`

走 Lyra 的 `ULyraHeroComponent` 绑定链路（PawnExtension → Hero 的 InitState 门控就绪后才绑定输入并授予技能），
而不是自己另起一套输入系统。

### 3. RPG 角色与战斗动画资产

`Content/AHyTest/Character/Liki/` 下整理了一套 RPG 战斗动画：
连招攻击（Combo Attack 01~04）、喝药水（Drink Potion）、受击死亡（Hit Death）等，
并转成 Lyra 可用的 AnimSequence / Montage。

### 4. 第三方插件 VRM4U 适配 UE 5.6

UE 5.6 的 UHT 强制开启 `NativePointerMemberBehavior=Disallow`（硬编码在 UBT 里，改 ini 关不掉），
VRM4U 这个第三方插件的 UPROPERTY 裸指针成员全部报错。完成迁移：

- **98 处** UPROPERTY 裸指针成员 → `TObjectPtr<T>`（含容器内的 `TArray<UTexture2D*>`，UHT 对容器同样报错）；
- 保留 `class` 前缀（`TObjectPtr<class UFoo>`，Epic 官方有 889 处同款写法），避免丢失隐式前向声明；
- 修复由此产生的 C++ 副作用：模板推导 `T*`、 `auto*`、 `UObject*&` 引用绑定三类问题；
- 8 个模块全部编译链接通过（`Result: Succeeded`，0 error）。

原始文件备份在 `Saved/TObjectPtrBackup*/`。

### 5. 核心代码中文注释工程

对 Lyra 核心骨架与 8 个插件逐文件补充中文注释，**只加注释、不改逻辑**：

- 8 个插件（ModularGameplayActors、UIExtension、GameSubtitles、GameSettings、CommonGame、AsyncMixin、CommonLoadingScreen、CommonUser）+ 各自 README；
- P0 核心骨架：`Source/LyraGame/` 下的 GameModes、System、Character、Player 四个目录。

---

## 技术栈

| 领域 | 用到的技术 |
| --- | --- |
| 玩法框架 | Game Feature / Modular Gameplay、Lyra Experience 加载流水线 |
| 技能与属性 | Gameplay Ability System（ASC / GA / GE / AS） |
| 角色初始化 | `IGameFrameworkInitStateInterface` 的 InitState 门控（PawnExtension → Hero） |
| 输入 | Enhanced Input + `ULyraInputConfig` 能力-输入绑定 |
| 网络 | Replication Graph（NotRouted / RelevantAllConnections / Spatialize_* 路由策略） |
| UI | CommonUI + UIExtension（插件化解耦 UI 扩展点） |
| 脚本 | UnLua（Lua 热更新，第三方插件） |

---

## 目录结构

```
LyraHy/
├── Source/LyraGame/            # Lyra 游戏模块（已补中文注释）
│   ├── GameModes/              # Experience 定义、GameMode、GameState
│   ├── System/                 # AssetManager、GameInstance、ReplicationGraph
│   ├── Character/              # Pawn / Hero / HealthComponent
│   └── Player/                 # PlayerController / PlayerState / LocalPlayer
├── Plugins/
│   ├── GameFeatures/
│   │   ├── ShooterCore/        # E̶p̶i̶c̶ 官方射击玩法插件（对标对象）
│   │   └── RPGCore/            # ★ 自研 RPG 玩法插件
│   ├── VRM4U/                  # 第三方 VRM 插件（已适配 UE 5.6）
│   ├── KawaiiPhysics/          # 第三方物理插件
│   └── UnLua/                  # 第三方 Lua 脚本插件
└── Content/
    ├── AHyTest/                # RPG 角色动画测试资产
    └── Input/                  # RPG 输入配置
```

---

## 如何运行

1. 用 **Unreal Engine 5.6** 打开 `LyraHy.uproject`（源码版引擎）；
2. 生成工程文件后编译 `LyraEditor` / `Development` / `Win64`；
3. 打开 `Plugins/GameFeatures/RPGCore/Content/Maps/L_RPGGym.umap` 即可体验 RPG 玩法。

> **编译提示**：若出现大量 `Access is denied` / `LNK1136: 无效或损坏的文件`，
> 这是 UBA（Unreal Build Accelerator）的问题，加 `-NoUBA` 参数编译即可：
> ```
> Engine\Build\BatchFiles\Build.bat LyraEditor Win64 Development -Project="LyraHy.uproject" -NoUBA
> ```

---

## 路线图

- [ ] 为 `RPGCore` 增加 C++ 运行时模块（目前是纯 Content 插件）
- [ ] 技能系统：基于 GAS 实现 RPG 技能（主动技 / 被动技 / 冷却与消耗）
- [ ] 属性与成长：等级、经验、属性点分配
- [ ] 装备与背包：基于 Lyra `Inventory` / `Equipment` 体系扩展 RPG 装备位
- [ ] 任务与对话（复用 Lyra 的 CommonConversation）

---

## 许可与免责声明

- Lyra 原始代码与内容：© Epic Games，遵循 [Unreal Engine EULA](https://www.unrealengine.com/eula)；
- VRM4U：© Haruyoshi Yamamoto，MIT License；
- UnLua / KawaiiPhysics：遵循各自上游许可；
- 本工程原创部分（`RPGCore` 插件、RPG 相关内容与代码改动）：作者 HuanYu。
