// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "LyraAttributeSet.h"
// #include "NativeGameplayTags.h"
#include "LyraRPGStatSet.generated.h"
#define UE_API LYRAGAME_API

class UObject;
struct FFrame;

// // ---------------------------------------------------------------------------
// // 本属性集用到的 GameplayTag 声明（对应的定义在 .cpp 中）
// // 用于标记伤害的类型/来源，供 PostGameplayEffectExecute 判断处理逻辑
// // ---------------------------------------------------------------------------
// LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);				// 普通伤害
// LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);		// 伤害免疫
// LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);	// 自毁伤害（可无视无敌/无限血）
// LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);		// 掉出世界造成的伤害
// LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Lyra_Damage_Message);			// 伤害消息广播使用的 Verb

struct FGameplayEffectModCallbackData;

/**
 * ULyraRPGStatSet
 *
 * RPG 角色属性集（AttributeSet），继承自 ULyraAttributeSet。
 * 在 Lyra 原生生命值的基础上，扩展了「体力（Stamina）」相关属性。
 *
 * 包含四个属性：
 *   - Health  / MaxHealth  ：当前生命值 / 最大生命值
 *   - Stamina / MaxStamina ：当前体力值 / 最大体力值
 *
 * 职责：
 *   1. 存储属性数值（由 GameplayEffect 修改）；
 *   2. 在 GE 执行前后做数值钳制（Clamp）与死亡判定；
 *   3. 通过委托把「属性变化」广播给 ULyraHealthComponent，最终驱动 UI。
 */
UCLASS(MinimalAPI, BlueprintType)
class ULyraRPGStatSet : public ULyraAttributeSet
{
	GENERATED_BODY()

public:
	UE_API ULyraRPGStatSet();

	// 为每个属性生成 Get / Set / GetAttribute 等访问器（GAS 标准宏）
	ATTRIBUTE_ACCESSORS(ULyraRPGStatSet, Health);
	ATTRIBUTE_ACCESSORS(ULyraRPGStatSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(ULyraRPGStatSet, Stamina);
	ATTRIBUTE_ACCESSORS(ULyraRPGStatSet, MaxStamina);



	// 当前生命值（会被 MaxHealth 限制）
	UPROPERTY(BlueprintReadOnly, Category = "Lyra|RPGStats")
	FGameplayAttributeData Health;


	// 最大生命值（本身也是属性，可被 GameplayEffect 修改，例如升级提升血上限）
	UPROPERTY(BlueprintReadOnly, Category = "Lyra|RPGStats")
	FGameplayAttributeData MaxHealth;


	// 当前体力值（会被 MaxStamina 限制）
	UPROPERTY(BlueprintReadOnly, Category = "Lyra|RPGStats")
	FGameplayAttributeData Stamina;


	// 最大体力值
	UPROPERTY(BlueprintReadOnly, Category = "Lyra|RPGStats")
	FGameplayAttributeData MaxStamina;

	// 生命值变化时广播（客户端上 Instigator 可能无效）
	mutable FLyraAttributeEvent OnHealthChanged;
	// 体力值变化时广播
	mutable FLyraAttributeEvent OnStaminaChanged;
	// 最大生命值变化时广播
	mutable FLyraAttributeEvent OnMaxHealthChanged;
	// 最大体力值变化时广播
	mutable FLyraAttributeEvent OnMaxStaminaChanged;
	// 生命值归零时广播（用于触发死亡流程）
	mutable FLyraAttributeEvent OnOutOfHealth;

	// 标记当前是否已处于「生命归零」状态，避免死亡事件被重复广播
	bool bOutOfHealth;

	// 记录 GE 执行前的旧值，供 PostGameplayEffectExecute 计算「变化前后」的差值
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;

	// 同上，体力值的旧值备份
	float MaxStaminaBeforeAttributeChange;
	float StaminaBeforeAttributeChange;

protected:
	// GE 执行「之前」调用：保存属性旧值（返回 false 会中断本次 GE）
	UE_API virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;

	// GE 执行「之后」调用：数值钳制、死亡判定、广播事件
	UE_API virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

};

#undef UE_API
