# 附录 B - 常见陷阱与 FAQ

> **一句话概括**：读 Lyra / 写 GameFeature 时最容易踩的坑，以及新手最常问的问题。

## 这份清单会收录

- 编译、启动、加载阶段的典型报错
- Experience / GameFeature 配置错误
- GAS 常见陷阱
- 网络同步问题
- 性能问题

---

## 待撰写的内容提纲

### 一、"点 PIE 一直黑屏，没有角色"

- 排查点：
  1. Experience 是否被正确选中？看 Log 的 "Identified experience"
  2. Experience 的 `DefaultPawnData` 是否为空？
  3. `PawnData.PawnClass` 是否为空？

### 二、"我的 Ability 激活不了"

- 排查点：
  1. `AbilitySet` 授予了吗？GameplayDebugger 看 ASC 的 Ability 列表
  2. Cost / Cooldown 挡住了？
  3. `ActivationRequiredTags` / `ActivationBlockedTags` 冲突？
  4. InputConfig 有没有绑按键？
  5. `LyraHeroComponent.BindAbilityActions` 有没有调用？

### 三、"GameFeature 激活日志没打印"

- 排查点：
  1. uplugin 的 `Type` 是不是 `"GameFeature"`？
  2. Experience 的 `GameFeaturesToEnable` 里写的是不是插件名？
  3. 插件是不是被编辑器 Enabled？

### 四、"Attribute 变化客户端看不到"

- 排查点：
  1. Attribute 有没有 `ReplicatedUsing`？
  2. `GetLifetimeReplicatedProps` 有没有声明？
  3. 是不是直接 `Attribute.SetValue`，应该用 `GameplayEffect`

### 五、"InitState 卡住不前进"

- 最常见原因：某个组件 `CanChangeInitState` 返回了 false
- 调试：`GameFrameworkComponentManager.DebugShowComponentManager`

### 六、"Editor 编译巨慢"

- 单独编译 `LyraGame` 模块：在 VS 的 Solution Explorer 右键单独 Build
- 用 Live Coding（Ctrl+Alt+F11）做小修改
- Clean + Rebuild 只在结构变化时做

### 七、常见问题 FAQ

**Q：Lyra 能直接商用吗？**
A：可以。Epic 明确说 Lyra 是 Sample，你可以 fork 了改，但要注意 UE EULA 的通用条款。

**Q：Lyra 最小化要裁掉什么？**
A：自己用不到的 GameFeatures（ShooterCore / TopDownArena）、示例关卡、示例资源。但 `CommonUI` / `ModularGameplay` / `GameFeatures` 这些基础插件别动。

**Q：Lyra 适合做非 FPS 游戏吗？**
A：适合。TopDownArena 就证明了俯视角也能用。你要做 RPG / 卡牌 / 赛车都可以套它的架构——只要你理解 Experience + GameFeature 这套思路。

**Q：Lyra 的蓝图多得读不完怎么办？**
A：C++ 是骨架，蓝图大多数是数据配置和表现层（如 `ABP_Mannequin_UE5` 动画蓝图）。先看 C++，蓝图按需查。

**Q：为什么 Lyra 要 UE 5.x？**
A：它用了 UE 5 特性：Enhanced Input、Nanite、Lumen、World Partition。降到 UE 4 要重写很多。

---

**⚠️ 本附录尚未撰写**。会随着你实际遇到的问题不断增补。
