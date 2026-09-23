// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 生命值组件：桥接 LyraHealthSet（属性集）与游戏逻辑。
 *
 * 它监听 Health / MaxHealth 属性变化并对外广播，
 * 血量为 0 时启动死亡流程（StartDeath -> OnDeathStarted -> ... -> FinishDeath）。
 */
#include "Components/GameFrameworkComponent.h"

#include "LyraHealthComponent.generated.h"

class ULyraHealthComponent;

class ULyraAbilitySystemComponent;
class ULyraHealthSet;
class UObject;
struct FFrame;
struct FGameplayEffectSpec;

// 死亡事件委托：参数是拥有者 Actor
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLyraHealth_DeathEvent, AActor*, OwningActor);
// 属性变化委托：组件自身 + 旧值 + 新值 + 施加者
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLyraHealth_AttributeChanged, ULyraHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);

// 死亡状态枚举
/**
 * ELyraDeathState
 *
 *	Defines current state of death.
 */
// 死亡流程的三个阶段
UENUM(BlueprintType)
enum class ELyraDeathState : uint8
{
	// 还没死
	NotDead = 0,
	// 死亡已开始（正在播死亡动画等）
	DeathStarted,
	// 死亡已完成
	DeathFinished
};


// 生命值相关的一切都在这里处理
/**
 * ULyraHealthComponent
 *
 *	An actor component used to handle anything related to health.
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:

	ULyraHealthComponent(const FObjectInitializer& ObjectInitializer);

	// 便捷查找：取指定 Actor 上的生命值组件
	// Returns the health component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|Health")
	static ULyraHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<ULyraHealthComponent>() : nullptr); }

	// 用指定 ASC 初始化（会从 ASC 上取 HealthSet 并挂属性变化监听）
	// Initialize the component using an ability system component.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Health")
	void InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC);

	// 反初始化：清掉对 ASC 的引用与监听
	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Health")
	void UninitializeFromAbilitySystem();

	// 取当前血量
	// Returns the current health value.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Health")
	float GetHealth() const;

	// 取当前最大血量
	// Returns the current maximum health value.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Health")
	float GetMaxHealth() const;

	// 取归一化血量 [0,1]
	// Returns the current health in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "Lyra|Health")
	float GetHealthNormalized() const;

	// 取死亡状态
	UFUNCTION(BlueprintCallable, Category = "Lyra|Health")
	ELyraDeathState GetDeathState() const { return DeathState; }

	// 是否已死亡或正在死亡
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lyra|Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > ELyraDeathState::NotDead); }

	// 启动死亡流程
	// Begins the death sequence for the owner.
	virtual void StartDeath();

	// 结束死亡流程
	// Ends the death sequence for the owner.
	virtual void FinishDeath();

	// 对自己施加足以致死的伤害；bFellOutOfWorld 表示是掉出世界导致的
	// Applies enough damage to kill the owner.
	virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

public:

	// 血量变化时广播（客户端也会触发，但施加者可能无效）
	// Delegate fired when the health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FLyraHealth_AttributeChanged OnHealthChanged;

	// 最大血量变化时广播
	// Delegate fired when the max health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FLyraHealth_AttributeChanged OnMaxHealthChanged;

	// 死亡开始时广播
	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable)
	FLyraHealth_DeathEvent OnDeathStarted;

	// 死亡完成时广播
	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable)
	FLyraHealth_DeathEvent OnDeathFinished;

protected:

	// 注销时反初始化
	virtual void OnUnregister() override;

	// 清掉本组件加上的 GameplayTag
	void ClearGameplayTags();

	// 血量属性变化回调
	virtual void HandleHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	// 最大血量属性变化回调
	virtual void HandleMaxHealthChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);
	// 血量归零回调：在这里触发死亡
	virtual void HandleOutOfHealth(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue);

	UFUNCTION()
	// 死亡状态复制到客户端后的处理
	virtual void OnRep_DeathState(ELyraDeathState OldDeathState);

protected:

	// 本组件使用的 ASC
	// Ability system used by this component.
	UPROPERTY()
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;

	// 本组件使用的 HealthSet 属性集
	// Health set used by this component.
	UPROPERTY()
	TObjectPtr<const ULyraHealthSet> HealthSet;

	// 死亡状态（复制到客户端）
	// Replicated state used to handle dying.
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	ELyraDeathState DeathState;
};
