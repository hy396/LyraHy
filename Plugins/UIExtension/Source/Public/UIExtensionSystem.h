// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/WorldSubsystem.h"

#include "UIExtensionSystem.generated.h"

class UUIExtensionSubsystem;
struct FUIExtensionRequest;
template <typename T> class TSubclassOf;

class FSubsystemCollectionBase;
class UUserWidget;
struct FFrame;

/** 扩展点的标签匹配规则：决定本扩展点能收到哪些标签下的扩展。 */
UENUM(BlueprintType)
enum class EUIExtensionPointMatch : uint8
{
	/** 精确匹配：只接收标签完全一致的扩展。
	 *  例如本点注册在 "A.B"，则只有注册在 "A.B" 的扩展会推过来，"A.B.C" 不会。 */
	ExactMatch,

	/** 父子通配匹配：还会接收注册在【子标签】下的扩展。
	 *  例如本点注册在 "A.B"，则 "A.B" 和 "A.B.C" 上的扩展都会推过来。 */
	PartialMatch
};

/** 扩展变更的动作类型：回调据此判断是"新增了扩展"还是"移除了扩展"。
 *  （原版此处沿用了一句 "Match rule for extension points" 的注释，属 Epic 笔误，与语义无关。） */
UENUM(BlueprintType)
enum class EUIExtensionAction : uint8
{
	/** 有新的扩展加入。 */
	Added,

	/** 有扩展被移除。 */
	Removed
};

/**
 * 扩展点回调（C++ 版本，非动态委托）。
 * @param Action       Added 表示有新扩展加入，Removed 表示有扩展被移除。
 * @param Request      本次变更对应的扩展请求，里面有控件类/数据、优先级、上下文对象等。
 */
DECLARE_DELEGATE_TwoParams(FExtendExtensionPointDelegate, EUIExtensionAction Action, const FUIExtensionRequest& Request);

/**
 * FUIExtension —— 一条"扩展"（即被贡献出去的内容）。
 *
 * 谁想往 UI 上加东西，就注册一条 FUIExtension：指明要挂到哪个扩展点标签上，
 * 以及要贡献的数据（通常是一个控件类，也可以是任意 UObject 数据）。
 */
struct FUIExtension : TSharedFromThis<FUIExtension>
{
public:
	/** 本扩展要挂到的扩展点标签。 */
	FGameplayTag ExtensionPointTag;

	/** 优先级。注意本系统自身不排序，只是透传给消费方让它自己决定顺序；惯例是数值越大越优先。 */
	int32 Priority = INDEX_NONE;

	/** 上下文对象（通常是 LocalPlayer 或 PlayerState）。为空表示"不针对特定玩家"。 */
	TWeakObjectPtr<UObject> ContextObject;

	/** 要贡献的数据：可以是控件类（UClass），也可以是任意 UObject。
	 *  由 UUIExtensionSubsystem::AddReferencedObjects 负责保活，防止被 GC 回收。 */
	TObjectPtr<UObject> Data = nullptr;
};

/**
 * FUIExtensionPoint —— 一个"扩展点"（即消费扩展的槽位）。
 *
 * 谁想在 UI 上留一个可插拔的位置，就注册一个 FUIExtensionPoint：
 * 指明自己的标签、匹配规则、能接受什么类型的数据，以及收到扩展时的回调。
 */
struct FUIExtensionPoint : TSharedFromThis<FUIExtensionPoint>
{
public:
	/** 本扩展点监听的标签。 */
	FGameplayTag ExtensionPointTag;

	/** 上下文对象；为空表示接收"不针对特定玩家"的扩展。 */
	TWeakObjectPtr<UObject> ContextObject;

	/** 标签匹配规则（精确 / 父子通配）。 */
	EUIExtensionPointMatch ExtensionPointTagMatchType = EUIExtensionPointMatch::ExactMatch;

	/** 数据契约：只有当扩展的数据属于（或实现了）这里的某个类时，才会被本点接受。 */
	TArray<TObjectPtr<UClass>> AllowedDataClasses;

	/** 收到扩展变更时的回调。 */
	FExtendExtensionPointDelegate Callback;

	// Tests if the extension and the extension point match up, if they do then this extension point should learn
	// about this extension.
	bool DoesExtensionPassContract(const FUIExtension* Extension) const;
};

/**
 * FUIExtensionPointHandle —— 扩展点的句柄（蓝图可用的不透明结构体）。
 * 注册扩展点后拿到它，用于之后 Unregister；注册被拒绝时它是无效的（IsValid() 为 false）。
 */
USTRUCT(BlueprintType)
struct UIEXTENSION_API FUIExtensionPointHandle
{
	GENERATED_BODY()

public:
	FUIExtensionPointHandle() {}

	void Unregister();

	bool IsValid() const { return DataPtr.IsValid(); }

	bool operator==(const FUIExtensionPointHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FUIExtensionPointHandle& Other) const { return !operator==(Other); }

	friend uint32 GetTypeHash(const FUIExtensionPointHandle& Handle)
	{
		return PointerHash(Handle.DataPtr.Get());
	}

private:
	TWeakObjectPtr<UUIExtensionSubsystem> ExtensionSource;

	TSharedPtr<FUIExtensionPoint> DataPtr;

	friend UUIExtensionSubsystem;

	FUIExtensionPointHandle(UUIExtensionSubsystem* InExtensionSource, const TSharedPtr<FUIExtensionPoint>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr) {}
};

template<>
struct TStructOpsTypeTraits<FUIExtensionPointHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionPointHandle>
{
	enum
	{
		WithCopy = true,  // This ensures the opaque type is copied correctly in BPs
		WithIdenticalViaEquality = true,
	};
};

/**
 * FUIExtensionHandle —— 扩展的句柄（蓝图可用的不透明结构体）。
 * 注册扩展后拿到它，用于之后 Unregister；注册被拒绝时它是无效的（IsValid() 为 false）。
 */
USTRUCT(BlueprintType)
struct UIEXTENSION_API FUIExtensionHandle
{
	GENERATED_BODY()

public:
	FUIExtensionHandle() {}

	void Unregister();

	bool IsValid() const { return DataPtr.IsValid(); }

	bool operator==(const FUIExtensionHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FUIExtensionHandle& Other) const { return !operator==(Other); }

	friend FORCEINLINE uint32 GetTypeHash(FUIExtensionHandle Handle)
	{
		return PointerHash(Handle.DataPtr.Get());
	}

private:
	TWeakObjectPtr<UUIExtensionSubsystem> ExtensionSource;

	TSharedPtr<FUIExtension> DataPtr;

	friend UUIExtensionSubsystem;

	FUIExtensionHandle(UUIExtensionSubsystem* InExtensionSource, const TSharedPtr<FUIExtension>& InDataPtr) : ExtensionSource(InExtensionSource), DataPtr(InDataPtr) {}
};

template<>
struct TStructOpsTypeTraits<FUIExtensionHandle> : public TStructOpsTypeTraitsBase2<FUIExtensionHandle>
{
	enum
	{
		WithCopy = true,  // This ensures the opaque type is copied correctly in BPs
		WithIdenticalViaEquality = true,
	};
};

/**
 * FUIExtensionRequest —— 回调里收到的"一次扩展变更请求"。
 * 消费方（扩展点）根据它来创建/销毁实际的控件或处理数据。
 */
USTRUCT(BlueprintType)
struct FUIExtensionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FUIExtensionHandle ExtensionHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag ExtensionPointTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Priority = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UObject> Data = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UObject> ContextObject = nullptr;
};

/** 扩展点回调的蓝图版本（动态委托），供 BlueprintCallable 的 K2_ 系列函数使用。 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FExtendExtensionPointDynamicDelegate, EUIExtensionAction, Action, const FUIExtensionRequest&, ExtensionRequest);

/**
 * UUIExtensionSubsystem —— UI 扩展系统的核心（World 子系统）。
 *
 * 它维护两张表：
 *   - ExtensionPointMap：标签 → 该标签下所有【扩展点】（消费方）
 *   - ExtensionMap     ：标签 → 该标签下所有【扩展】（贡献方）
 *
 * 任何一方注册时，都会立刻与另一方做匹配并通知，因此"谁先注册"都不影响结果。
 * 匹配要同时满足两个条件：上下文对象一致（都为空也算一致），且数据类通过 AllowedDataClasses 契约。
 */
UCLASS()
class UIEXTENSION_API UUIExtensionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 注册一个扩展点（不绑定任何上下文，只接收"无上下文"的扩展）。 */
	FUIExtensionPointHandle RegisterExtensionPoint(const FGameplayTag& ExtensionPointTag, EUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FExtendExtensionPointDelegate ExtensionCallback);

	/** 注册一个绑定了上下文对象（如 LocalPlayer / PlayerState）的扩展点。
	 *  它只会收到 ContextObject 相同的扩展，用于实现"分玩家的 UI 扩展"。 */
	FUIExtensionPointHandle RegisterExtensionPointForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, EUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FExtendExtensionPointDelegate ExtensionCallback);

	/** 贡献一个控件类（无上下文版本）。注意注册的是【类】而不是实例。 */
	FUIExtensionHandle RegisterExtensionAsWidget(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);

	/** 贡献一个控件类，并绑定到指定上下文对象。 */
	FUIExtensionHandle RegisterExtensionAsWidgetForContext(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);

	/** 贡献任意 UObject 数据（最通用的版本），上面两个 Widget 版本最终都会转调到这里。 */
	FUIExtensionHandle RegisterExtensionAsData(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	void UnregisterExtension(const FUIExtensionHandle& ExtensionHandle);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	void UnregisterExtensionPoint(const FUIExtensionPointHandle& ExtensionPointHandle);

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void NotifyExtensionPointOfExtensions(TSharedPtr<FUIExtensionPoint>& ExtensionPoint);
	void NotifyExtensionPointsOfExtension(EUIExtensionAction Action, TSharedPtr<FUIExtension>& Extension);

	/**
	 * 注册一个"扩展点" —— 即一个按标签命名的"槽位",用来消费(接收)挂在同一标签下的扩展。
	 *
	 * 每当有匹配的扩展加入,回调会收到 EUIExtensionAction::Added;扩展被移除时收到 ::Removed。
	 * 特别注意:本次调用返回之前,系统会为"在本扩展点注册之前就已存在"的扩展同步补发一批 Added,
	 * 所以回调必须能正确处理这初始的一批,不能假定它只会在将来异步触发。
	 *
	 * @param ExtensionPointTag          本扩展点监听的标签。必须是有效标签,否则注册失败。
	 * @param ExtensionPointTagMatchType ExactMatch 只接收标签完全一致的扩展;PartialMatch 还会接收
	 *                                   注册在子标签下的扩展(例如本点注册在 "A.B",那么 "A.B.C" 上的
	 *                                   扩展也会被推送过来)。
	 * @param AllowedDataClasses         扩展必须满足的"契约"过滤条件:扩展的数据必须继承自(或实现了接口)
	 *                                   其中的某个类,才会被本扩展点接受。至少要填 1 个,否则注册失败。
	 * @param ExtensionCallback          以 (Action, ExtensionRequest) 的形式被调用。它对宿主对象是弱绑定,
	 *                                   宿主销毁后自动不再触发。必须处于已绑定状态,否则注册失败。
	 * @return 用于后续反注册的句柄;注册被拒绝时返回无效句柄。请保存它,并在本扩展点不再需要时
	 *         (例如控件销毁时)调用 UnregisterExtensionPoint,否则回调会继续打给一个已经死掉的对象。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension Point"))
	FUIExtensionPointHandle K2_RegisterExtensionPoint(FGameplayTag ExtensionPointTag, EUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FExtendExtensionPointDynamicDelegate ExtensionCallback);
	
	/**
	 * 向某个扩展点"贡献"一个控件类,让拥有该标签的扩展点能够把这个控件创建出来。
	 *
	 * 这里注册的是控件【类】而不是实例:接收方的扩展点通过 FUIExtensionRequest::Data 拿到这个类,
	 * 由它自己负责创建(以及销毁)真正的控件实例。
	 * 如果这个控件属于某个特定的玩家 / 上下文对象,请改用 "Register Extension (Widget For Context)"。
	 *
	 * @param ExtensionPointTag 要贡献到的扩展点标签。必须是有效标签,否则注册失败。
	 * @param WidgetClass       要贡献的控件类。不能为空;并且目标扩展点的 AllowedDataClasses 必须接受它,
	 *                          否则这条扩展会被静默过滤掉 —— 扩展点永远收不到,而且不报任何错,很难排查。
	 * @param Priority          原样透传给 FUIExtensionRequest::Priority,由消费方(扩展点)自己决定怎么排序。
	 *                          本系统自身并不对扩展排序;惯例是数值越大越优先。默认 -1。
	 * @return 用于后续反注册的句柄;注册被拒绝时返回无效句柄。贡献方对象销毁时要记得反注册,
	 *         否则扩展点会一直拿着一个已经失效的控件类。
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget)"))
	FUIExtensionHandle K2_RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority = -1);

	/**
	 * Registers the widget (as data) for a specific player.  This means the extension points will receive a UIExtensionForPlayer data object
	 * that they can look at to determine if it's for whatever they consider their player.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension", meta = (DisplayName = "Register Extension (Widget For Context)"))
	FUIExtensionHandle K2_RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority = -1);

	/**
	 * Registers the extension as data for any extension point that can make use of it.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data)"))
	FUIExtensionHandle K2_RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority = -1);

	/**
	 * Registers the extension as data for any extension point that can make use of it.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="UI Extension", meta = (DisplayName = "Register Extension (Data For Context)"))
	FUIExtensionHandle K2_RegisterExtensionAsDataForContext(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority = -1);

	/** 把内部的 FUIExtension 打包成对外回调用的 FUIExtensionRequest。 */
	FUIExtensionRequest CreateExtensionRequest(const TSharedPtr<FUIExtension>& Extension);

private:
	/** 某个标签下的所有扩展点。 */
	typedef TArray<TSharedPtr<FUIExtensionPoint>> FExtensionPointList;
	/** 标签 → 扩展点列表（消费方总表）。 */
	TMap<FGameplayTag, FExtensionPointList> ExtensionPointMap;

	/** 某个标签下的所有扩展。 */
	typedef TArray<TSharedPtr<FUIExtension>> FExtensionList;
	/** 标签 → 扩展列表（贡献方总表）。 */
	TMap<FGameplayTag, FExtensionList> ExtensionMap;
};


/** 蓝图函数库：让蓝图也能对 FUIExtensionHandle 做反注册和有效性判断（C++ 里可直接调 Handle 的成员函数）。 */
UCLASS()
class UIEXTENSION_API UUIExtensionHandleFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UUIExtensionHandleFunctions() { }

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static void Unregister(UPARAM(ref) FUIExtensionHandle& Handle);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static bool IsValid(UPARAM(ref) FUIExtensionHandle& Handle);
};

UCLASS()
class UIEXTENSION_API UUIExtensionPointHandleFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UUIExtensionPointHandleFunctions() { }

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static void Unregister(UPARAM(ref) FUIExtensionPointHandle& Handle);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "UI Extension")
	static bool IsValid(UPARAM(ref) FUIExtensionPointHandle& Handle);
};
