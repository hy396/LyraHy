# 08 - Lyra 的 GAS 扩展

> **一句话概括**：Lyra 在标准 GAS 上加了几个"独家发明"——AbilitySet 打包授予、TagRelationshipMapping 解耦、ActivationGroup 互斥组——每个都是提高大型项目 GAS 可维护性的关键。

## 前置知识

- [07 - GAS 六大概念速成](07-GAS六大概念速成.md)

## 这一章读完你能答

1. `ULyraAbilitySet` 解决了什么问题？
2. `AbilityTagRelationshipMapping` 为什么比把 Tag 写在 Ability 里更好？
3. `ELyraAbilityActivationGroup` 里的 Independent / Exclusive 什么时候用？
4. `ULyraAbilitySystemComponent` 重写了父类哪些方法，为什么？

---

## 待撰写的内容提纲

### 一、LyraAbilitySystemComponent 的扩展

- `GrantedTags` 机制
- `AbilityInputCache`：存住按键、等 Ability 就绪后触发
- `AbilityTagRelationshipMapping` 注入点
- `NotifyAbilityActivated` 事件

### 二、LyraGameplayAbility 基类规定

- `ActivationPolicy`: `OnInputTriggered` / `OnInputHeld` / `WhileInputActive` / `OnSpawn`
- `ActivationGroup`: `Independent` / `Exclusive_Replaceable` / `Exclusive_Blocking`
- Cost / Cooldown 的封装
- `FailureTags` —— 激活失败时发的 Tag
- `K2_GetSourceObject` 等蓝图化

### 三、LyraAttributeSet 家族

- `LyraHealthSet`: Health / MaxHealth / Healing / Damage (meta)
- `LyraCombatSet`: BaseDamage / BaseHeal
- meta attribute 的设计（不需要复制，只是传递数值）
- `PreAttributeChange` / `PostGameplayEffectExecute` 的重写点

### 四、LyraAbilitySet：打包授予

- 数据结构：Ability + Effect + AttributeSet 列表
- `GiveToAbilitySystem` 的流程
- `FLyraAbilitySet_GrantedHandles` 句柄回收

### 五、LyraAbilityTagRelationshipMapping：解耦的艺术

- 问题：能力互相阻塞/依赖，怎么避免在每个能力里硬编码 Tag
- 方案：把关系移到数据资产
- 运行时：`GetRelationshipActivationTagRequirements` 动态查询

### 六、动手练习

- 读完整的开火能力：找 `Ability_RangedWeapon_*` 类和它们引用的 Effect
- 写一个"吼叫" Ability：进入 Independent Group，激活时广播一个 Cue
- 用 TagRelationshipMapping 实现"冲刺时不能开火"

---

**⚠️ 本章尚未撰写**。这章需要和 11 章武器系统配合读，因为开火是 GAS 最完整的例子。
