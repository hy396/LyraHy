# 07 - GAS 六大概念速成

> **一句话概括**：不讲 Lyra，先把 GAS（Gameplay Ability System）本身的 ASC / Attribute / GameplayEffect / GameplayAbility / AbilityTask / GameplayCue 这六件事讲透。

## 前置知识

- [06 - GameplayTags 与消息总线](06-GameplayTags与消息总线.md)

## 这一章读完你能答

1. GAS 六大概念分别做什么？它们如何协作？
2. 为什么不能直接 `Health -= 10`？
3. "瞬时 / 持续 / 无限" 三种 Effect 分别在什么时候用？
4. AbilityTask 和普通异步协程有什么区别？

---

## 待撰写的内容提纲

### 一、GAS 是什么，不是什么

- GAS **是**：一套属性复制 + 能力执行 + 效果应用的统一框架
- GAS **不是**：一个完整的战斗系统（战斗还得你自己搭）

### 二、六大概念

1. **AbilitySystemComponent (ASC)** —— 挂在某个 Actor 上的"总控"
2. **Attribute / AttributeSet** —— 可复制数值属性（带基础值和当前值）
3. **GameplayEffect (GE)** —— 修改 Attribute 的唯一合法手段
4. **GameplayAbility (GA)** —— 可执行的动作
5. **AbilityTask** —— Ability 里的异步步骤（等动画、等网络）
6. **GameplayCue (GC)** —— 视觉/音效反馈

### 三、一次完整的交互：开火如何伤害

画一张端到端流程图：

```
按键 → ASC->TryActivateAbility
      → Ability: PlayMontage (AbilityTask)
      → Ability: 射线检测
      → Target.ASC->ApplyGameplayEffectToTarget(Damage GE)
      → GE 修改 Target.HealthSet.Health
      → ASC 触发 GameplayCue：血花特效
      → 死亡判定（Health <= 0）→ 触发死亡 Ability
```

### 四、网络模型

- Ability 的预测：`Predicted` / `LocalPredicted` / `ServerInitiated`
- Attribute 的 `REPNOTIFY` 模式
- 权限：谁能 `ApplyEffect`，谁能 `ActivateAbility`

### 五、动手练习

- 用纸笔画 GAS 六大概念的关系图
- 找 UE 官方 ActionRPG 样本（比 Lyra 简单）看一个最小 GAS 项目
- 读 [Tranek's GASDocumentation](https://github.com/tranek/GASDocumentation) 的 4.x 章节

---

**⚠️ 本章尚未撰写**。这章其实是 GAS 通用介绍，可以直接推荐外部资源减少撰写量。
