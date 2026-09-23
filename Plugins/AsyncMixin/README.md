# AsyncMixin 插件说明

> 本文为中文说明文档。插件源码中的注释也已补充为中文。

## 一、插件用途与适用场景

`AsyncMixin` 是一个**极小但很实用**的工具插件（仅 969 行），它解决的是异步加载里的一个经典痛点：

> 我要加载 A、B、C 三个资源，每个加载完各做一点事，而且**必须按 A→B→C 的顺序**执行。
> 手写的话回调会层层嵌套成"回调地狱"，而且谁先加载完是不确定的，顺序根本保证不了。

`FAsyncMixin` 让你把这件事写成平铺的几行，并且保证回调按你书写的顺序执行：

```cpp
CancelAsyncLoading();                                   // 先取消上一批没跑完的
AsyncLoad(SoftClassA,  [this]() { /* 第一步 */ });
AsyncLoad(SoftObjectB, [this](UObject* O) { /* 第二步 */ });
AsyncLoad(SoftClassC,  [this]() { /* 第三步 */ });
StartAsyncLoading();                                    // 发车
```

适用场景：

- 界面需要先加载图标、再加载数据、最后才显示
- 列表条目控件会被复用，切换数据时必须取消上一批未完成的加载
- 需要在多个异步加载之间插入"等某个条件成立"的步骤
- 想避免"加载指示器只闪一帧"的尴尬

**它不依赖任何其它插件**，可以单独拿到任何工程里用。

## 二、目录结构

```
Plugins/AsyncMixin/
├── AsyncMixin.uplugin
├── README.md
└── Source/
    ├── AsyncMixin.Build.cs
    ├── Public/
    │   └── AsyncMixin.h       全部对外接口（FAsyncMixin / FAsyncScope / FAsyncCondition）
    └── Private/
        ├── AsyncMixin.cpp     流水线状态机实现（FLoadingState / FAsyncStep）
        └── AsyncMixinModule.cpp
```

## 三、核心类与职责

### 3.1 `FAsyncMixin`（主类）

混入类。任何类（**包括 UObject**）都可以额外继承它，它不增加宿主对象的内存占用。

| 接口 | 作用 |
|---|---|
| `AsyncLoad(...)` | 添加一个加载步骤。有 6 个重载，支持 `TSoftClassPtr` / `TSoftObjectPtr` / `FSoftObjectPath` / 数组，回调可以是无参、带加载好的类、带加载好的对象 |
| `AsyncPreloadPrimaryAssetsAndBundles(...)` | 预加载主资产（PrimaryAsset）里由属性引用到的 Bundle |
| `AsyncCondition(Condition, Callback)` | 插入一个"自定义条件"步骤，条件成立前流水线停在这里 |
| `AsyncEvent(Callback)` | 插入一个**不加载任何东西**的纯回调步骤，只用来占位置、保证顺序 |
| `StartAsyncLoading()` | 启动流水线（建议显式调用） |
| `CancelAsyncLoading()` | 取消所有未完成的步骤 |
| `IsAsyncLoadingInProgress()` | 是否还在加载中 |
| `OnStartedLoading()` / `OnFinishedLoading()` | 子类可重写的生命周期钩子 |

### 3.2 `FAsyncScope`

`FAsyncMixin` 的公开版。当你的类需要同时管理**多组互不相干**的异步链时（继承只能有一条流水线），给每组任务各建一个 `FAsyncScope` 成员即可。

### 3.3 `FAsyncCondition`

自定义等待条件。每帧被询问一次：

- 返回 `TryAgain` —— 条件不成立，下一帧再问
- 返回 `Complete` —— 条件成立，流水线继续往下走

用于表达"等玩家登录完成""等某子系统初始化好"这类**无法用资源加载描述**的依赖。

### 3.4 内部实现（了解即可）

| 内部类 | 作用 |
|---|---|
| `FLoadingState` | 真正维护流水线的状态。**不作为成员存在**，而是放在一个静态 `TMap` 里，用时分配、用完销毁 |
| `FAsyncStep` | 流水线上的一个步骤，可能挂着一个流式句柄或一个自定义条件 |

## 四、依赖关系

**插件依赖**：无（`Plugins` 列表为空）。

**模块依赖**（`AsyncMixin.Build.cs`）：

| 模块 | 用途 |
|---|---|
| Core / CoreUObject | UE 基础 |
| Engine | `UAssetManager` / `FStreamableManager`——真正执行异步加载的地方 |

**被谁依赖**：本工程中被大量 UI 与前端类使用（Lyra 的界面基类、Experience 加载流程等）。

## 五、接入与使用要点

### 5.1 基本用法

```cpp
// 1) 继承
class UMyWidget : public UUserWidget, public FAsyncMixin
{
    void SetData(UMyData* Data)
    {
        // 2) 先取消上一批（控件被复用时非常关键）
        CancelAsyncLoading();

        // 3) 平铺地添加各步
        AsyncLoad(Data->IconSoftPtr, [this](UTexture2D* Tex) {
            Image_Icon->SetBrushFromTexture(Tex);
        });

        AsyncLoad(Data->DetailClass, [this](TSubclassOf<UUserWidget> Cls) {
            DetailWidgetClass = Cls;
        });

        // 4) 发车
        StartAsyncLoading();
    }

    // 5) 可选：重写生命周期钩子
    virtual void OnStartedLoading() override  { ShowSpinner(); }
    virtual void OnFinishedLoading() override { HideSpinner(); }
};
```

### 5.2 插入自定义等待条件

```cpp
AsyncCondition(
    MakeShared<FAsyncCondition>([]() -> EAsyncConditionResult
    {
        return bLoggedIn ? EAsyncConditionResult::Complete
                         : EAsyncConditionResult::TryAgain;
    }),
    FSimpleDelegate::CreateLambda([this]() { /* 条件满足后要做的事 */ }));
```

### 5.3 几个容易踩的坑

1. **复用型对象必须先 `CancelAsyncLoading()`** —— 列表条目控件在切数据时，上一批加载可能还没跑完，不取消的话旧回调会覆盖新数据。

2. **务必显式调 `StartAsyncLoading()`** —— 不调的话它会在下一帧自动启动（不会出错），但可能让加载指示器白闪一帧。

3. **`AsyncPreloadPrimaryAssetsAndBundles` 会让内部状态一直存活** —— Bundle 预加载要求保留住流式句柄，否则资源会被立刻卸载。所以用过这个函数的对象，全部加载完成后内部状态**不会**被销毁（这个行为由 `FLoadingState::bPreloadedBundles` 控制）。

4. **可以放心在 lambda 里捕获 `[this]`** —— 宿主析构时会从全局状态表摘掉自己并解绑所有回调，不存在悬空回调。这是本插件最实用的特性之一。

5. **`AsyncLoad` 的重载很多，注意选对** ——
   - `TSoftClassPtr` → 加载的是**类**（回参是 `TSubclassOf<T>`）
   - `TSoftObjectPtr` → 加载的是**对象**（回参是 `T*`）
   选错了会得到空指针，而且不会报错，排查时容易绕圈。

6. **只在游戏线程使用** —— 内部多处 `check(IsInGameThread())`，在工作线程上调用会直接断言失败。

### 5.4 调试

命令行加 `-LogCmds="LogAsyncMixin Verbose"`，可以看到每一步的发出、完成、取消过程。
