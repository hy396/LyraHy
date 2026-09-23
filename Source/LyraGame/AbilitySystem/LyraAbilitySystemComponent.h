// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Abilities/LyraGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "LyraAbilitySystemComponent.generated.h"

class AActor;
class UGameplayAbility;
class ULyraAbilityTagRelationshipMapping;
class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;

LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked);

/**
 * ULyraAbilitySystemComponent
 *
 *	本项目使用的 GAS（Gameplay Ability System，技能系统）组件基类。
 *
 *	在引擎原生 UAbilitySystemComponent 之上，Lyra 额外补了这几块能力：
 *	- 输入驱动的技能激活：维护「本帧按下 / 本帧松开 / 持续按住」三个输入缓冲队列，
 *	  由 ProcessAbilityInput 统一处理，避免输入在帧间丢失。
 *	- 技能激活组（Activation Group）互斥：同一时刻只允许特定组内的技能共存。
 *	- 标签关系映射：通过 ULyraAbilityTagRelationshipMapping 配置表，
 *	  根据技能标签推导出额外的「必需标签 / 阻塞标签 / 取消标签」。
 *	- 动态授予标签：用一个 GameplayEffect 来动态给角色加/删标签，便于按引用计数移除。
 */
UCLASS()
class LYRAGAME_API ULyraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:

	ULyraAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	/** 批量取消技能时用的判定回调：对某个技能返回 true，表示应当把它取消掉。 */
	typedef TFunctionRef<bool(const ULyraGameplayAbility* LyraAbility, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;

	/** 遍历当前所有已激活的技能，逐个交给 ShouldCancelFunc 判定，把判定为 true 的取消掉。 */
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	/** 取消所有"靠输入触发激活"的技能（例如长按型技能）。 */
	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);

	// ===== 输入缓冲 =====
	// Lyra 不直接在引擎输入回调里激活技能，而是先把输入标签缓存进队列，
	// 再由 ProcessAbilityInput 每帧统一消费。这样即使"输入到达"和"技能被授予"
	// 的先后顺序颠倒（常见于 GameFeature 异步加载场景），输入也不会丢。

	/** 某个输入标签被按下：记入本帧按下队列，同时加入「持续按住」队列。 */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/** 某个输入标签被松开：记入本帧松开队列，并从「持续按住」队列移除。 */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/** 每帧调用：真正消费上面三个输入队列，按顺序尝试激活 / 结束对应的技能。 */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	/** 清空全部输入缓冲（角色失焦、暂停、死亡等场景会用到）。 */
	void ClearAbilityInput();

	// ===== 激活组（Activation Group）互斥 =====

	/** 该激活组当前是否被更高优先级的组阻塞（例如「独占组」正在运行，则其他组不能起）。 */
	bool IsActivationGroupBlocked(ELyraAbilityActivationGroup Group) const;

	/** 技能激活时把自己登记进对应激活组，组内计数 +1。 */
	void AddAbilityToActivationGroup(ELyraAbilityActivationGroup Group, ULyraGameplayAbility* LyraAbility);

	/** 技能结束时从激活组里注销，组内计数 -1。 */
	void RemoveAbilityFromActivationGroup(ELyraAbilityActivationGroup Group, ULyraGameplayAbility* LyraAbility);

	/** 取消某个激活组内的所有技能；IgnoreLyraAbility 用来排除"自己"，避免刚激活就被自己取消。 */
	void CancelActivationGroupAbilities(ELyraAbilityActivationGroup Group, ULyraGameplayAbility* IgnoreLyraAbility, bool bReplicateCancelAbility);

	// ===== 动态授予标签 =====
	// 用一个"无限时长 + 引用计数"的 GameplayEffect 来给角色动态加标签，
	// 好处是同一个标签被多处请求时不会互相覆盖，各方各自移除自己的那份即可。

	/** 借助 GameplayEffect 给角色动态添加指定标签。 */
	void AddDynamicTagGameplayEffect(const FGameplayTag& Tag);

	/** 移除所有用于添加该标签的 GameplayEffect 实例，从而把标签摘掉。 */
	void RemoveDynamicTagGameplayEffect(const FGameplayTag& Tag);

	/** 取回指定技能句柄 + 激活信息所对应的目标数据（TargetData），非实例化的技能常用它取回命中结果。 */
	void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	/** 设置当前的标签关系映射表；传 nullptr 表示清空。 */
	void SetTagRelationshipMapping(ULyraAbilityTagRelationshipMapping* NewMapping);

	/** 根据技能的标签，从映射表里查出额外的「激活必需标签」和「激活阻塞标签」。 */
	void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

protected:

	/** 角色/Pawn 生成完成后，尝试自动激活那些标记为「生成时自动激活」的技能。 */
	void TryActivateAbilitiesOnSpawn();

	// 以下为引擎 UAbilitySystemComponent 的重写点，Lyra 在这里接入自己的输入缓冲与激活组逻辑。

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

	/** 服务器 → 客户端的 RPC：通知客户端"某个技能激活失败了"，客户端据此播放失败反馈。 */
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	/** 技能激活失败后的统一处理入口（本地处理 + 必要时转发给客户端）。 */
	void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);
protected:

	/** 标签关系映射表（由 GameFeature 或 PawnData 配置注入），为空则表示不做额外标签推导。 */
	UPROPERTY()
	TObjectPtr<ULyraAbilityTagRelationshipMapping> TagRelationshipMapping;

	/** 本帧刚被按下的技能句柄（对应 AbilityInputTagPressed）。 */
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	/** 本帧刚被松开的技能句柄（对应 AbilityInputTagReleased）。 */
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	/** 当前处于"按住"状态的技能句柄，用于长按型技能的持续判定。 */
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;

	/** 每个激活组当前正在运行的技能数量，下标即 ELyraAbilityActivationGroup。 */
	int32 ActivationGroupCounts[(uint8)ELyraAbilityActivationGroup::MAX];
};
