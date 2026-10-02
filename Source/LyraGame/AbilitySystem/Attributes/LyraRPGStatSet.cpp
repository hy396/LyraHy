// Copyright Epic Games, Inc. All Rights Reserved.


#include "LyraRPGStatSet.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Messages/LyraVerbMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraGameplayTags.h"
#include "LyraLogChannels.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraRPGStatSet)

// // ---------------------------------------------------------------------------
// // GameplayTag 定义（与头文件中的 DECLARE 一一对应）
// // ---------------------------------------------------------------------------
// UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_Damage, "Gameplay.Damage");								// 普通伤害
// UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageImmunity, "Gameplay.DamageImmunity");				// 伤害免疫
// UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_DamageSelfDestruct, "Gameplay.Damage.SelfDestruct");	// 自毁伤害（可无视无敌/无限血）
// UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_FellOutOfWorld, "Gameplay.Damage.FellOutOfWorld");		// 掉出世界造成的伤害
// UE_DEFINE_GAMEPLAY_TAG(TAG_Lyra_Damage_Message, "Lyra.Damage.Message");						// 伤害消息广播使用的 Verb

/**
 * 构造函数
 *
 * 设置四个属性的初始默认值（均为 100），并初始化所有状态标记。
 *
 * 注意：这里只是「默认值」。实际运行时，ULyraHealthComponent 会在初始化阶段
 *       把当前值（Health / Stamina）拉满到各自的最大值。
 */
ULyraRPGStatSet::ULyraRPGStatSet()
    :Health(100.f)
    , MaxHealth(100.f)
    , Stamina(100.f)
    , MaxStamina(100.f)
{
    bOutOfHealth = false;                    // 初始未处于死亡状态
    MaxHealthBeforeAttributeChange = 0.0f;   // 旧值备份清零
    HealthBeforeAttributeChange = 0.0f;
    MaxStaminaBeforeAttributeChange = 0.0f;
    StaminaBeforeAttributeChange = 0.0f;
}


/**
 * GameplayEffect 执行「之前」调用
 *
 * 目的：把属性的当前值保存下来。
 * 因为 GE 一旦执行完毕，属性就已经被改写，届时无法再得知「原来是多少」；
 * 而 UI 需要知道「从多少变到多少」，才能播放血条动画、伤害飘字等。
 *
 * 返回 false 会中断本次 GE 的执行。
 */
bool ULyraRPGStatSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
    if (!Super::PreGameplayEffectExecute(Data))
    {
        return false;
    }
    // 备份生命值的旧值
    HealthBeforeAttributeChange = GetHealth();
    MaxHealthBeforeAttributeChange = GetMaxHealth();

    // 备份体力值的旧值
    StaminaBeforeAttributeChange = GetStamina();
    MaxStaminaBeforeAttributeChange = GetMaxStamina();
    return true;
}

/**
 * GameplayEffect 执行「之后」调用
 *
 * 负责三件事：
 *   1. 数值钳制（Clamp）——把生命/体力限制在合法范围内；
 *   2. 死亡判定与事件广播——生命归零时广播 OnOutOfHealth；
 *   3. 伤害消息广播——让其他系统（UI、统计、击杀播报等）能观察到伤害事件。
 */
void ULyraRPGStatSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // ---- ① 生命值钳制：限制在 [0, 100] ----
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // 取出当前生命值（此时已由前置流程修改过）
        float CurrentHealth = GetHealth();

        // 再次钳制（防御性编程，避免出现负数或超出上限）
        float ClampedHealth = FMath::Clamp(CurrentHealth, 0.0f, GetMaxHealth());
        // 仅在与钳制结果不同时才写回（减少无意义的赋值）
        if (CurrentHealth != ClampedHealth)
        {
            SetHealth(ClampedHealth);
        }
    }
    // ---- ② 体力值钳制：限制在 [0, 100] ----
    if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        // 取出当前体力值
        float CurrentStamina = GetStamina();

        // 再次钳制（防御性编程）
        float ClampedStamina = FMath::Clamp(CurrentStamina, 0.0f, GetMaxStamina());

        // 仅在与钳制结果不同时才写回
        if (CurrentStamina != ClampedStamina)
        {
            SetStamina(ClampedStamina);
        }
    }

    // 判断本次伤害是否来自「自毁」（自毁伤害可以无视无敌/无限血）
    const bool bIsDamageFromSelfDestruct = Data.EffectSpec.GetDynamicAssetTags().HasTagExact(TAG_Gameplay_DamageSelfDestruct);
    float MinimumHealth = 0.0f;

#if !UE_BUILD_SHIPPING
    // 调试用：开启无敌（GodMode）或无限血时阻止死亡（自毁伤害除外）
    if (!bIsDamageFromSelfDestruct &&
        (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode) || Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_UnlimitedHealth)))
    {
        MinimumHealth = 1.0f;
    }
#endif // #if !UE_BUILD_SHIPPING

    // 取出伤害的来源（Instigator）与施加者（Causer），用于后续事件广播
    const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
    AActor* Instigator = EffectContext.GetOriginalInstigator();
    AActor* Causer = EffectContext.GetEffectCauser();

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // Magnitude > 0 视为「造成伤害」，广播一条伤害消息给全局消息系统
        if (Data.EvaluatedData.Magnitude > 0.0f)
        {
            FLyraVerbMessage Message;
            Message.Verb = TAG_Lyra_Damage_Message;
            Message.Instigator = Data.EffectSpec.GetEffectContext().GetEffectCauser();
            Message.InstigatorTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
            Message.Target = GetOwningActor();
            Message.TargetTags = *Data.EffectSpec.CapturedTargetTags.GetAggregatedTags();
            //@TODO: 补充上下文标签，以及非能力系统来源的 source/instigator 标签
            //@TODO: 判断是否为敌方击杀、自杀、队友击杀等情况
            Message.Magnitude = Data.EvaluatedData.Magnitude;

            UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
            MessageSystem.BroadcastMessage(Message.Verb, Message);
        }
        // 生命值确实发生变化时，广播「生命值变化」事件
        if (GetHealth() != HealthBeforeAttributeChange)
        {
            OnHealthChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
        }

        // 生命值归零且此前未处于死亡状态 → 广播「生命耗尽」事件（只广播一次）
        if ((GetHealth() <= 0.0f) && !bOutOfHealth)
        {
            OnOutOfHealth.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth());
        }

        // 再次检查生命值，防止在上面的广播过程中被其他逻辑改动
        bOutOfHealth = (GetHealth() <= 0.0f);
    }
    if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        // 体力值发生变化时，广播「体力值变化」事件
        if (GetStamina() != StaminaBeforeAttributeChange)
        {
            OnStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
        }
    }
}
