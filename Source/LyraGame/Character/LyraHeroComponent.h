// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Hero 组件：给「玩家控制的 Pawn」（或模拟玩家的 Bot）装输入绑定与摄像机。
 *
 * 它依赖 PawnExtensionComponent，必须等那边放行（PawnData + Controller + PlayerState 都就绪）
 * 才会绑定增强输入、把能力按 InputTag 关联到按键、并决定用哪个摄像机模式。
 */
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "GameFeatures/GameFeatureAction_AddInputContextMapping.h"
#include "GameplayAbilitySpecHandle.h"
#include "LyraHeroComponent.generated.h"

namespace EEndPlayReason { enum Type : int; }
struct FLoadedMappableConfigPair;
struct FMappableConfigPair;

class UGameFrameworkComponentManager;
class UInputComponent;
class ULyraCameraMode;
class ULyraInputConfig;
class UObject;
struct FActorInitStateChangedParams;
struct FFrame;
struct FGameplayTag;
struct FInputActionValue;

// 玩家 Pawn 的输入与摄像机处理组件
/**
 * Component that sets up input and camera handling for player controlled pawns (or bots that simulate players).
 * This depends on a PawnExtensionComponent to coordinate initialization.
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class LYRAGAME_API ULyraHeroComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:

	ULyraHeroComponent(const FObjectInitializer& ObjectInitializer);

	// 便捷查找：取指定 Actor 上的 Hero 组件
	/** Returns the hero component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "Lyra|Hero")
	static ULyraHeroComponent* FindHeroComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<ULyraHeroComponent>() : nullptr); }

	// 由某个激活中的技能临时接管摄像机（例如开镜）
	/** Overrides the camera from an active gameplay ability */
	void SetAbilityCameraMode(TSubclassOf<ULyraCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle);

	// 清除技能设置的摄像机覆盖；只有当初设置它的那个技能才能清掉
	/** Clears the camera override if it is set */
	void ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle);

	// 追加一份额外的输入配置（例如进入载具时加一套载具按键）
	/** Adds mode-specific input config */
	void AddAdditionalInputConfig(const ULyraInputConfig* InputConfig);

	// 移除之前追加的那份额外输入配置
	/** Removes a mode-specific input config if it has been added */
	void RemoveAdditionalInputConfig(const ULyraInputConfig* InputConfig);

	// 是否已经推进到可以绑定额外输入的阶段（非玩家永远不会为 true）
	/** True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added */
	bool IsReadyToBindInputs() const;
	
	// 组件管理器上广播的事件名：表示「现在可以绑输入了」
	/** The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
	static const FName NAME_BindInputsNow;

	// 本特性在组件管理器里的名字
	/** The name of this component-implemented feature */
	static const FName NAME_ActorFeatureName;

	// 初始化状态机接口
	//~ Begin IGameFrameworkInitStateInterface interface
	// 返回特性名
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	// 判断能否迁移状态（关键：要等 PawnExtension 那边先就绪）
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	// 真正执行状态迁移
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	// 其它 Actor 状态变化时收到通知
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	// 尝试按默认路径自动推进初始化
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

protected:

	// 注册时向组件管理器登记，并声明自己依赖 PawnExtension 特性
	virtual void OnRegister() override;
	// 开始：注册状态机
	virtual void BeginPlay() override;
	// 结束：注销并清理输入
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 真正绑定输入：按 InputConfig 把 InputTag 与输入动作关联起来
	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);

	// 输入动作按下：把对应 InputTag 的Ability 触发按下
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	// 输入动作松开
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	// 移动输入
	void Input_Move(const FInputActionValue& InputActionValue);
	// 鼠标视角
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	// 手柄摇杆视角
	void Input_LookStick(const FInputActionValue& InputActionValue);
	// 下蹲
	void Input_Crouch(const FInputActionValue& InputActionValue);
	// 自动奔跑
	void Input_AutoRun(const FInputActionValue& InputActionValue);

	// 决定用哪个摄像机模式：技能覆盖优先，否则用 PawnData 里的默认模式
	TSubclassOf<ULyraCameraMode> DetermineCameraMode() const;
	
	// 输入配置被激活时（增强输入的用户设置变更）
	void OnInputConfigActivated(const FLoadedMappableConfigPair& ConfigPair);
	// 输入配置被反激活时
	void OnInputConfigDeactivated(const FLoadedMappableConfigPair& ConfigPair);

protected:

	// 已废弃：旧的默认输入配置数组；5.3 起改用 DefaultInputMappings
	/**
	 * Input Configs that should be added to this player when initializing the input. These configs
	 * will NOT be registered with the settings because they are added at runtime. If you want the config
	 * pair to be in the settings, then add it via the GameFeatureAction_AddInputConfig
	 * 
	 * NOTE: You should only add to this if you do not have a game feature plugin accessible to you.
	 * If you do, then use the GameFeatureAction_AddInputConfig instead. 
	 */
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	UE_DEPRECATED(5.3, "DefaultInputConfigs have been deprecated, use DefaultInputMappings instead")
	TArray<FMappableConfigPair> DefaultInputConfigs;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	// 初始化时要加进来的输入映射上下文与优先级
	UPROPERTY(EditAnywhere)
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
	
	// 由技能设置的摄像机模式
	/** Camera mode set by an ability. */
	UPROPERTY()
	TSubclassOf<ULyraCameraMode> AbilityCameraMode;

	// 是哪个技能设置的摄像机模式（用于校验清除请求）
	/** Spec handle for the last ability to set a camera mode. */
	FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;

	// 输入绑定是否已应用；非玩家永远为 false
	/** True when player input bindings have been applied, will never be true for non - players */
	bool bReadyToBindInputs;
};
