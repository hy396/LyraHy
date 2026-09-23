// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "LyraGamePhaseSubsystem.generated.h"

template <typename T> class TSubclassOf;

class ULyraGamePhaseAbility;
class UObject;
struct FFrame;
struct FGameplayAbilitySpecHandle;

DECLARE_DYNAMIC_DELEGATE_OneParam(FLyraGamePhaseDynamicDelegate, const ULyraGamePhaseAbility*, Phase);
DECLARE_DELEGATE_OneParam(FLyraGamePhaseDelegate, const ULyraGamePhaseAbility* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FLyraGamePhaseTagDynamicDelegate, const FGameplayTag&, PhaseTag);
DECLARE_DELEGATE_OneParam(FLyraGamePhaseTagDelegate, const FGameplayTag& PhaseTag);

/** 阶段标签的匹配规则（决定观察者能监听到哪些阶段）。 */
UENUM(BlueprintType)
enum class EPhaseTagMatchType : uint8
{
	/** 精确匹配：只监听标签完全一致的阶段。
	 *  例如监听 "A.B"，只有 "A.B" 阶段会触发，"A.B.C" 不会。 */
	ExactMatch,

	/** 父子通配匹配：监听该标签及其所有子标签。
	 *  例如监听 "A.B"，"A.B" 和 "A.B.C" 都会触发。 */
	PartialMatch
};


/**
 * ULyraGamePhaseSubsystem —— 游戏阶段（Game Phase）管理器
 *
 *	用【可嵌套的 GameplayTag】来管理比赛阶段，例如：
 *	Game.Playing、Game.Playing.WarmUp、Game.Playing.PostGame、Game.ShowingScore。
 *
 *	核心规则（这套嵌套语义是本系统最容易误解的地方）：
 *	- 父子阶段可以同时存在，兄弟阶段互斥。
 *	  例：Game.Playing 和 Game.Playing.WarmUp 可以共存；
 *	      但 Game.Playing 和 Game.ShowingScore 不能共存。
 *	- 开启一个新阶段时，所有"不是它的祖先"的已激活阶段都会被结束。
 *	  例：当前 Game.Playing 和 Game.Playing.CaptureTheFlag 都激活，
 *	      此时开启 Game.Playing.PostGame，则 Game.Playing 保留（它是祖先），
 *	      而 Game.Playing.CaptureTheFlag 会被结束（它是兄弟/后代，非祖先）。
 *
 *	阶段本身是用 GameplayAbility（ULyraGamePhaseAbility）实现的，
 *	所以"阶段激活"本质上就是激活一个技能，能天然享受 GAS 的复制与标签体系。
 */
UCLASS()
class ULyraGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULyraGamePhaseSubsystem();

	virtual void PostInitialize() override;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/**
	 * 开启一个阶段：激活对应的 ULyraGamePhaseAbility 技能。
	 * @param PhaseAbility        要开启的阶段技能类。
	 * @param PhaseEndedCallback  该阶段结束时的回调（C++ 用）。
	 */
	void StartPhase(TSubclassOf<ULyraGamePhaseAbility> PhaseAbility, FLyraGamePhaseDelegate PhaseEndedCallback = FLyraGamePhaseDelegate());

	// 注意（Epic 原始 TODO，值得留意）：这两个观察者注册接口目前【没有返回句柄】，
	// 也就是说注册进去的观察者无法单独注销，会一直累积到世界重置为止。
	// 所以不要在每帧/频繁调用的地方反复注册，否则观察者数组会不停膨胀。

	/**
	 * 注册"阶段开始（或已经处于激活状态）"的观察者。
	 * 如果目标阶段当前已经激活，回调会【立即】触发一次，而不只是等将来。
	 */
	void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FLyraGamePhaseTagDelegate& WhenPhaseActive);

	/** 注册"阶段结束"的观察者。 */
	void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FLyraGamePhaseTagDelegate& WhenPhaseEnd);

	/** 指定阶段当前是否处于激活状态（BlueprintAuthorityOnly：只在服务端有意义）。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName="Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<ULyraGamePhaseAbility> Phase, const FLyraGamePhaseDynamicDelegate& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FLyraGamePhaseTagDynamicDelegate WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FLyraGamePhaseTagDynamicDelegate WhenPhaseEnd);

	void OnBeginPhase(const ULyraGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	void OnEndPhase(const ULyraGamePhaseAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);

private:
	/** 一个处于激活状态的阶段记录：阶段标签 + 它结束时的回调。 */
	struct FLyraGamePhaseEntry
	{
	public:
		FGameplayTag PhaseTag;
		FLyraGamePhaseDelegate PhaseEndedCallback;
	};

	/** 当前所有激活中的阶段，键是阶段技能的 SpecHandle（因为阶段本身是个 GameplayAbility）。 */
	TMap<FGameplayAbilitySpecHandle, FLyraGamePhaseEntry> ActivePhaseMap;

	/** 一个阶段观察者：记录要监听哪个标签、用什么匹配规则、触发时调哪个回调。 */
	struct FPhaseObserver
	{
	public:
		/** 按 MatchType 判定传入的阶段标签是否命中本观察者。 */
		bool IsMatch(const FGameplayTag& ComparePhaseTag) const;
	
		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FLyraGamePhaseTagDelegate PhaseCallback;
	};

	/** 监听"阶段开始/已激活"的观察者列表（见上方 TODO：只增不减）。 */
	TArray<FPhaseObserver> PhaseStartObservers;

	/** 监听"阶段结束"的观察者列表。 */
	TArray<FPhaseObserver> PhaseEndObservers;

	friend class ULyraGamePhaseAbility;
};
