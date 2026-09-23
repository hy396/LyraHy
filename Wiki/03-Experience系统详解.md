# 03 - Experience 系统详解

> **一句话概括**：把 `ULyraExperienceDefinition` 这个"游戏配置清单"的每个字段拆开看，理解 ActionSet 怎么递归组合、GameFeatures 怎么被拉起。

## 前置知识

- [02 - 启动链：从 InitGame 到 Pawn 生成](02-启动链-从InitGame到Pawn生成.md)

## 这一章读完你能答

1. `ULyraExperienceDefinition` 有哪几个关键字段，每个字段起什么作用？
2. `ExperienceActionSet` 为什么存在，它和直接在 Experience 里写 Actions 有什么差别？
3. `ULyraUserFacingExperienceDefinition` 和 `ULyraExperienceDefinition` 是什么关系？
4. 为什么 Lyra 把 Experience 设计成 `UPrimaryDataAsset` 而不是蓝图？

---

## 待撰写的内容提纲

### 一、ULyraExperienceDefinition 字段逐个讲

- `GameFeaturesToEnable: TArray<FString>` —— 插件 URL 列表
- `DefaultPawnData: ULyraPawnData*` —— 默认角色档案
- `Actions: TArray<UGameFeatureAction*>` —— Experience 自身的 Action（不依赖插件）
- `ActionSets: TArray<ULyraExperienceActionSet*>` —— 引用其他 ActionSet 做组合

### 二、ExperienceActionSet：组合而非重复

- 让多个 Experience 共享一组 Actions
- 典型场景：所有射击类 Experience 都需要的基础能力/UI

### 三、UserFacingExperience：玩家菜单看到的条目

- `ULyraUserFacingExperienceDefinition` 包一层 Experience
- 附加展示信息：图标、名字、描述
- 负责创建 `CommonSession_HostSessionRequest`，驱动地图切换

### 四、ExperienceManagerComponent 的加载状态机

- `ELyraExperienceLoadState` 枚举逐个解析
- `LoadingGameFeatures` / `ExecutingActions` / `Deactivating`

### 五、资产扫描与 Primary Asset 注册

- `LyraAssetManager` 里 Experience 的类型注册
- `DefaultGame.ini` 里的 PrimaryAssetTypesToScan

### 六、动手练习

- 新建一个只改 `DefaultPawnData` 的 Experience 子类
- 在 Experience 里加一个 `GameFeatureAction_AddAbilities` 给角色加一个新能力

---

**⚠️ 本章尚未撰写**。看完 02 章如果觉得 Wiki 风格合适，告诉我"继续写 03"，我会按上面的提纲补全。
