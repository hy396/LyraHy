# 16 - 实战：第一个自定义 GameFeature

> **一句话概括**：从零建一个最小 GameFeature 插件，目标是跑通激活/卸载流程，这是你"真正会用" Lyra 的起点。

## 前置知识

- [04 - GameFeatures 与 ModularGameplay](04-GameFeatures与ModularGameplay.md)

## 这一章读完你能干

- 独立创建一个新的 GameFeature 插件
- 写一个自定义 `GameFeatureAction`
- 在一个已有 Experience 里引用它
- 验证激活/卸载时日志符合预期

---

## 待撰写的内容提纲

### 步骤 1：建插件目录

- `Plugins/GameFeatures/MyFirstFeature/`
- 必需文件：`MyFirstFeature.uplugin`
- 可选：`Source/MyFirstFeature/` 如果要写 C++

### 步骤 2：uplugin 文件

```json
{
    "FileVersion": 3,
    "Version": 1,
    "Type": "GameFeature",
    "Description": "我的第一个 Feature",
    "Category": "Lyra",
    "Plugins": [
        { "Name": "GameFeatures", "Enabled": true },
        { "Name": "ModularGameplay", "Enabled": true }
    ]
}
```

### 步骤 3：写 GameFeatureData 资产

- 编辑器里 Create Advanced → Game Feature Data
- 添加 Actions 列表

### 步骤 4：C++ 自定义 Action

```cpp
UCLASS()
class UMyPrintLogAction : public UGameFeatureAction
{
    GENERATED_BODY()
public:
    virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
    virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
    UPROPERTY(EditDefaultsOnly) FString Message = TEXT("Hello from MyFeature");
};
```

### 步骤 5：在 Experience 里引用

- 新建或修改一个 Experience
- `GameFeaturesToEnable` 添加 `"MyFirstFeature"`

### 步骤 6：PIE 测试

- 观察 Output Log：激活日志
- 切到其他 Experience：卸载日志
- 用 `GameFeatureDataInspector` 调试

### 步骤 7：加个有用的 Action

- `GameFeatureAction_AddWidget` 加一个浮动文字
- `GameFeatureAction_AddAbilities` 加一个新能力

### 进阶：Feature Actions 按 World Context

- 只在某个关卡激活：用 `GameFeatureAction_WorldActionBase`
- 观察多 PIE 窗口的行为

---

**⚠️ 本章尚未撰写**。做完这章你就有"能动的成果物"，学习动机会大幅提升。
