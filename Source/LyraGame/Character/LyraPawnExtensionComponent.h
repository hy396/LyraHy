// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Pawn 扩展组件：所有 Lyra Pawn（角色/载具等）都必须挂它。
 *
 * 它是「初始化总闸门」：通过 UGameFrameworkComponentManager 的状态机，
 * 把 PawnData 就绪、Controller 就绪、PlayerState 就绪、ASC 就绪这些条件串起来，
 * 只有全部满足后才允许 LyraHeroComponent 等组件继续往下初始化。
 *
 * 一句话：谁想初始化，都得先问它「现在能不能动」。
 */
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"

#include "LyraPawnExtensionComponent.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UGameFrameworkComponentManager;
class ULyraAbilitySystemComponent;
class ULyraPawnData;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;

// 挂在所有 Pawn 上的初始化协调组件
/**
 * Component that adds functionality to all Pawn classes so it can be used for characters/vehicles/etc.
 * This coordinates the initialization of other components.
 */
UCLASS()
class LYRAGAME_API ULyraPawnExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:

	ULyraPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	// 本特性在组件管理器里的名字；其他组件声明自己依赖这个名字即可排队等待
	/** The name of this overall feature, this one depends on the other named component features */
	static const FName NAME_ActorFeatureName;

	// 初始化状态机接口：见下面几个回调
	//~ Begin IGameFrameworkInitStateInterface interface
	// 返回特性名
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	// 判断能否从 CurrentState 迁移到 DesiredState（这里检查 PawnData/Controller/PlayerState）
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	// 真正执行状态迁移
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	// 其它 Actor 的初始化状态变化时收到通知，用于触发本组件继续推进
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	// 尝试按默认路径自动推进初始化
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	// 便捷查找：取指定 Actor 上的 PawnExtension 组件
	/** Returns the pawn extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "Lyra|Pawn")
	static ULyraPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<ULyraPawnExtensionComponent>() : nullptr); }

	// 取 PawnData（模板，调用方指定具体类型）
	/** Gets the pawn data, which is used to specify pawn properties in data */
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	// 设置 PawnData；设置完会重新检查能否继续初始化
	/** Sets the current pawn data */
	void SetPawnData(const ULyraPawnData* InPawnData);

	// 取当前缓存的 ASC；注意它可能属于别的 Actor（通常是 PlayerState）
	/** Gets the current ability system component, which may be owned by a different actor */
	UFUNCTION(BlueprintPure, Category = "Lyra|Pawn")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const { return AbilitySystemComponent; }

	// 由所属 Pawn 调用：把自己注册为 ASC 的 Avatar
	/** Should be called by the owning pawn to become the avatar of the ability system. */
	void InitializeAbilitySystem(ULyraAbilitySystemComponent* InASC, AActor* InOwnerActor);

	// 由所属 Pawn 调用：把自己从 ASC 的 Avatar 上摘掉
	/** Should be called by the owning pawn to remove itself as the avatar of the ability system. */
	void UninitializeAbilitySystem();

	// 由所属 Pawn 在 Controller 变化时调用
	/** Should be called by the owning pawn when the pawn's controller changes. */
	void HandleControllerChanged();

	// 由所属 Pawn 在 PlayerState 复制下来后调用
	/** Should be called by the owning pawn when the player state has been replicated. */
	void HandlePlayerStateReplicated();

	// 由所属 Pawn 在输入组件建立好时调用
	/** Should be called by the owning pawn when the input component is setup. */
	void SetupPlayerInputComponent();

	// 注册「ASC 已初始化」回调；如果已经初始化过，立刻同步调用一次
	/** Register with the OnAbilitySystemInitialized delegate and broadcast if our pawn has been registered with the ability system component */
	void OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate);

	// 注册「ASC 被反初始化」回调
	/** Register with the OnAbilitySystemUninitialized delegate fired when our pawn is removed as the ability system's avatar actor */
	void OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate);

protected:

	// 注册时向组件管理器登记本特性
	virtual void OnRegister() override;
	// 开始：把自己注册进状态机
	virtual void BeginPlay() override;
	// 结束：注销
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 客户端收到复制下来的 PawnData
	UFUNCTION()
	void OnRep_PawnData();

	// Pawn 成为 ASC 的 Avatar 时广播
	/** Delegate fired when our pawn becomes the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	// Pawn 被摘掉 ASC Avatar 身份时广播
	/** Delegate fired when our pawn is removed as the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	// 创建 Pawn 用的 PawnData（生成时指定，或摆放在关卡里的实例上直接指定）
	/** Pawn data used to create the pawn. Specified from a spawn function or on a placed instance. */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "Lyra|Pawn")
	TObjectPtr<const ULyraPawnData> PawnData;

	// 缓存的 ASC 指针，方便各组件直接取用
	/** Pointer to the ability system component that is cached for convenience. */
	UPROPERTY(Transient)
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
};
