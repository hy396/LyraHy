// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"

#include "LyraGameplayAbility.generated.h"

struct FGameplayAbilityActivationInfo;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;

class AActor;
class AController;
class ALyraCharacter;
class ALyraPlayerController;
class APlayerController;
class FText;
class ILyraAbilitySourceInterface;
class UAnimMontage;
class ULyraAbilityCost;
class ULyraAbilitySystemComponent;
class ULyraCameraMode;
class ULyraHeroComponent;
class UObject;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEffectSpec;
struct FGameplayEventData;

/**
 * ELyraAbilityActivationPolicy —— 技能的"激活方式"
 *
 *	决定这个技能是靠什么触发的。
 */
UENUM(BlueprintType)
enum class ELyraAbilityActivationPolicy : uint8
{
	/** 按下瞬间尝试激活一次（点按型，例如跳跃、冲刺）。 */
	OnInputTriggered,

	/** 只要输入还按着就持续尝试激活（长按型，例如瞄准、持续开火）。 */
	WhileInputActive,

	/** 角色（Avatar）一被赋予就自动激活，不需要输入（被动/常驻型技能）。 */
	OnSpawn
};


/**
 * ELyraAbilityActivationGroup —— 技能的"激活组"
 *
 *	决定这个技能和其他技能之间的互斥关系。
 */
UENUM(BlueprintType)
enum class ELyraAbilityActivationGroup : uint8
{
	/** 独立运行：不干扰任何其他技能，也不被其他技能干扰。 */
	Independent,

	/** 可被替换的独占技能：当有新的独占技能要激活时，自己会被取消掉让位。 */
	Exclusive_Replaceable,

	/** 阻塞型独占技能：自己运行期间，阻止其他所有独占技能激活。 */
	Exclusive_Blocking,

	MAX	UMETA(Hidden)
};

/** 技能激活失败时，用于播放"失败反馈蒙太奇"的消息体（走消息路由广播出去）。 */
USTRUCT(BlueprintType)
struct FLyraAbilityMontageFailureMessage
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	/** 本次失败的全部原因标签（可能有多个，例如"冷却中"+"资源不足"）。 */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer FailureTags;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAnimMontage> FailureMontage = nullptr;
};

/**
 * ULyraGameplayAbility
 *
 *	本项目所有 GameplayAbility 的基类。
 *
 *	它在引擎原生 UGameplayAbility 之上封装了 Lyra 常用的几件事：
 *	- 激活方式 / 激活组（见上面两个枚举）
 *	- 额外的技能消耗 ULyraAbilityCost（CheckCost / ApplyCost 里统一处理）
 *	- 失败反馈：失败标签 → 面向玩家的提示文本 / 失败蒙太奇 的映射
 *	- 技能期间临时切换相机模式（SetCameraMode / ClearCameraMode）
 *	- 便捷取用 Lyra 各类对象（ASC、角色、控制器、HeroComponent）
 *
 *	注意本类是 Abstract，不能直接用，必须继承。
 */
UCLASS(Abstract, HideCategories = Input, Meta = (ShortTooltip = "The base gameplay ability class used by this project."))
class LYRAGAME_API ULyraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	friend class ULyraAbilitySystemComponent;

public:

	ULyraGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ===== 便捷取用 Lyra 各类对象（都从 ActorInfo 推导，蓝图可直接调用） =====

	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	ALyraPlayerController* GetLyraPlayerControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	ALyraCharacter* GetLyraCharacterFromActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	ULyraHeroComponent* GetHeroComponentFromActorInfo() const;

	ELyraAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }
	ELyraAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	/** 供 ASC 在 Pawn 生成阶段调用：若本技能的激活方式是 OnSpawn，则在这里尝试激活。 */
	void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	/** 判断"切换到目标激活组"是否是一次合法转换（例如被阻塞的组不能进）。只判断不改状态。 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lyra|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanChangeActivationGroup(ELyraAbilityActivationGroup NewGroup) const;

	/** 真正切换激活组，成功返回 true。会连带处理"取消同组其他技能"等副作用。 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lyra|Ability", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ChangeActivationGroup(ELyraAbilityActivationGroup NewGroup);

	/** 让本技能在激活期间临时接管相机模式（例如瞄准、冲刺时切换视角）。 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	void SetCameraMode(TSubclassOf<ULyraCameraMode> CameraMode);

	/** 清除本技能设置的相机模式。技能结束时若未手动清理，会自动调用。 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|Ability")
	void ClearCameraMode();

	void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		NativeOnAbilityFailedToActivate(FailedReason);
		ScriptOnAbilityFailedToActivate(FailedReason);
	}

protected:

	/** 激活失败时的 C++ 侧处理（由 OnAbilityFailedToActivate 统一触发）。 */
	virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	/** 激活失败时的蓝图侧事件，可在蓝图里播提示、放失败蒙太奇。 */
	UFUNCTION(BlueprintImplementableEvent)
	void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	//~UGameplayAbility interface
	// 以下是引擎技能生命周期的重写点。Lyra 在这些点里接入了自己的逻辑：
	// 激活组切换、额外消耗(Cost)检查、相机模式清理、失败反馈派发等。
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	virtual void OnPawnAvatarSet();

	virtual void GetAbilitySource(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& OutSourceLevel, const ILyraAbilitySourceInterface*& OutAbilitySource, AActor*& OutEffectCauser) const;

	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityAdded")
	void K2_OnAbilityAdded();

	/** Called when this ability is removed from the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnAbilityRemoved")
	void K2_OnAbilityRemoved();

	/** Called when the ability system is initialized with a pawn avatar. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet")
	void K2_OnPawnAvatarSet();

protected:

	/** 本技能的激活方式（点按 / 长按 / 生成时自动）。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	ELyraAbilityActivationPolicy ActivationPolicy;

	/** 本技能的激活组，决定与其他技能的互斥关系。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Ability Activation")
	ELyraAbilityActivationGroup ActivationGroup;

	/** 除 GE 冷却/消耗之外，激活本技能还需额外支付的消耗（例如消耗弹药、消耗物品层数）。 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = Costs)
	TArray<TObjectPtr<ULyraAbilityCost>> AdditionalCosts;

	/** 失败标签 → 面向玩家的提示文案。激活失败时据此显示"冷却中"之类的提示。 */
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, FText> FailureTagToUserFacingMessages;

	/** 失败标签 → 失败时要播放的蒙太奇（例如"没弹药"的摇头动作）。 */
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> FailureTagToAnimMontage;

	/** 调试用：为 true 时，本技能被取消会打印额外日志。官方注释说明这是为排查 bug 临时加的。 */
	UPROPERTY(EditDefaultsOnly, Category = "Advanced")
	bool bLogCancelation;

	/** 本技能当前设置的相机模式；技能结束时用于清理还原。 */
	TSubclassOf<ULyraCameraMode> ActiveCameraMode;
};
