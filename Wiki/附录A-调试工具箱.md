# 附录 A - 调试工具箱

> **一句话概括**：Lyra 学习和开发过程中会反复用到的调试工具清单，按"什么情况下用"组织。

## 这份清单覆盖

1. GameplayDebugger
2. Visual Logger
3. Unreal Insights
4. stat 命令
5. GAS 专用调试
6. Game Feature 专用调试

---

## 待撰写的内容提纲

### 一、GameplayDebugger（GAS 调试神器）

- PIE 中按 `'` 键切换开关
- 数字键切分类：1-Pawn / 2-AI / 3-Ability / 4-Perception
- 看 ASC 的 Tag / Attribute / 当前激活 Ability
- 扩展：自定义 `FGameplayDebuggerCategory`

### 二、Visual Logger

- Window → Developer Tools → Visual Logger
- 录制一段 PIE，离线回放每帧
- 看 Actor 位置、AI 路径、技能调用

### 三、Unreal Insights

- 从命令行启动：`-trace=cpu,frame,bookmark`
- 打开 `.utrace` 文件
- 看 GAS Tick / 网络流量 / GC

### 四、常用 stat 命令

- `stat Unit` / `stat Unitgraph`：帧耗时
- `stat Game` / `stat Slate`
- `stat Net`：网络包
- `stat GPU`：GPU 瓶颈

### 五、GAS 专用调试命令

- `ShowDebug AbilitySystem`
- `AbilitySystem.DebugAbilitySystemComponent`
- `AbilitySystem.DebugNextTarget`

### 六、Game Feature 调试

- Log 类别：`LogGameFeatures` / `LogLyraExperience`
- 命令：`GameFeature.ListRegisteredPlugins` / `GameFeature.LoadGameFeaturePlugin`

### 七、日志类别速查

Lyra 的所有自定义 Log Category：

| Category | 用途 |
|---|---|
| `LogLyra` | 通用 |
| `LogLyraExperience` | Experience 加载 |
| `LogLyraAbilitySystem` | GAS |
| `LogLyraTeams` | 队伍系统 |
| `LogLyraInventory` | 物品栏 |
| `LogLyraCamera` | 相机 |

### 八、编辑器调试辅助

- Asset Audit（资产占用排查）
- Reference Viewer（资产引用图）
- Size Map（包体大小）

---

**⚠️ 本附录尚未撰写**。
