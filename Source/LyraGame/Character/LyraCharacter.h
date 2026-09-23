// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的角色基类（人形）。
 *
 * 它自己并不持有 ASC——ASC 通常来自 PlayerState，角色只作为 Avatar。
 * 主要职责：初始化时序衔接、队伍、移动复制优化（FastSharedReplication）、
 * 以及把死亡流程接到 HealthComponent 上。
 */
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "GameplayTagAssetInterface.h"
#include "ModularCharacter.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraCharacter.generated.h"

class AActor;
class AController;
class ALyraPlayerController;
class ALyraPlayerState;
class FLifetimeProperty;
class IRepChangedPropertyTracker;
class UAbilitySystemComponent;
class UInputComponent;
class ULyraAbilitySystemComponent;
class ULyraCameraComponent;
class ULyraHealthComponent;
class ULyraPawnExtensionComponent;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FGameplayTagContainer;


/**
 * FLyraReplicatedAcceleration: Compressed representation of acceleration
 */
// 用于复制的加速度结构
USTRUCT()
struct FLyraReplicatedAcceleration
{
	GENERATED_BODY()

	// 加速度（复制用）
	UPROPERTY()
	uint8 AccelXYRadians = 0;	// Direction of XY accel component, quantized to represent [0, 2*pi]

	// 是否压缩到整数以省带宽
	UPROPERTY()
	uint8 AccelXYMagnitude = 0;	//Accel rate of XY component, quantized to represent [0, MaxAcceleration]

	// 朝向
	UPROPERTY()
	int8 AccelZ = 0;	// Raw Z accel rate component, quantized to represent [-MaxAcceleration, MaxAcceleration]
};

/** The type we use to send FastShared movement updates. */
// 合并后的移动复制包：一次 RPC 把一份完整移动状态发给所有人
USTRUCT()
struct FSharedRepMovement
{
	GENERATED_BODY()

	// 构造函数
	FSharedRepMovement();

	// 用角色当前状态填充本结构
	bool FillForCharacter(ACharacter* Character);
	// 与另一份比较，判断是否需要重发
	bool Equals(const FSharedRepMovement& Other, ACharacter* Character) const;

	// 自定义网络序列化
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	// 位置
	UPROPERTY(Transient)
	FRepMovement RepMovement;

	// 旋转
	UPROPERTY(Transient)
	float RepTimeStamp = 0.0f;

	// 速度
	UPROPERTY(Transient)
	uint8 RepMovementMode = 0;

	// 移动模式
	UPROPERTY(Transient)
	bool bProxyIsJumpForceApplied = false;

	// 是否压缩过
	UPROPERTY(Transient)
	bool bIsCrouched = false;
};

template<>
// 声明 FSharedRepMovement 带自定义序列化器
struct TStructOpsTypeTraits<FSharedRepMovement> : public TStructOpsTypeTraitsBase2<FSharedRepMovement>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

/**
 * ALyraCharacter
 *
 *	The base character pawn class used by this project.
 *	Responsible for sending events to pawn components.
 *	New behavior should be added via pawn components when possible.
 */
// 本项目使用的角色基类
UCLASS(Config = Game, Meta = (ShortTooltip = "The base character pawn class used by this project."))
class LYRAGAME_API ALyraCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:

	ALyraCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取 Lyra 的 PlayerController
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	ALyraPlayerController* GetLyraPlayerController() const;

	// 取 Lyra 的 PlayerState
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	ALyraPlayerState* GetLyraPlayerState() const;

	// 取 Lyra 的 ASC
	UFUNCTION(BlueprintCallable, Category = "Lyra|Character")
	ULyraAbilitySystemComponent* GetLyraAbilitySystemComponent() const;
	// IAbilitySystemInterface：ASC 通常来自 PlayerState
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// GameplayTag 资产接口：从 ASC 上取标签
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	// 是否含有指定标签
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	// 是否含有全部指定标签
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	// 是否含有任一指定标签
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;

	// 切换下蹲
	void ToggleCrouch();

	// AActor 接口
	//~AActor interface
	// 组件初始化前
	virtual void PreInitializeComponents() override;
	// 开始
	virtual void BeginPlay() override;
	// 结束
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// 重置（对象池复用时）
	virtual void Reset() override;
	// 复制属性登记
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 复制前：决定这一帧是否走 FastShared 路径
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~End of AActor interface

	// APawn 接口
	//~APawn interface
	// Controller 变化时通知 PawnExtension 组件
	virtual void NotifyControllerChanged() override;
	//~End of APawn interface

	// 队伍接口
	//~ILyraTeamAgentInterface interface
	// 设置队伍 ID
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 取队伍 ID
	virtual FGenericTeamId GetGenericTeamId() const override;
	// 取队伍变化委托
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

	// 在属性复制被跳过的帧上也会发的 RPC，把一次移动更新广播给所有人
	/** RPCs that is called on frames when default property replication is skipped. This replicates a single movement update to everyone. */
	// FastSharedReplication 的实现体
	UFUNCTION(NetMulticast, unreliable)
	void FastSharedReplication(const FSharedRepMovement& SharedRepMovement);

	// 上一次发出去的移动包，用于避免重复发送
	// Last FSharedRepMovement we sent, to avoid sending repeatedly.
	FSharedRepMovement LastSharedReplication;

	// 判断并填充本帧要发的共享移动包
	virtual bool UpdateSharedReplication();

protected:

	// ASC 初始化完成：初始化游戏标签、绑定属性监听
	virtual void OnAbilitySystemInitialized();
	// ASC 被反初始化
	virtual void OnAbilitySystemUninitialized();

	// 被 Controller 占据
	virtual void PossessedBy(AController* NewController) override;
	// 被 Controller 释放
	virtual void UnPossessed() override;
// 客户端 Controller 复制下来

	// 客户端 PlayerState 复制下来
	virtual void OnRep_Controller() override;
	// 建立输入组件
	virtual void OnRep_PlayerState() override;

	// 把「移动中/静止」等状态写成 GameplayTag
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// 初始化与移动状态相关的标签
	void InitializeGameplayTags();

	// 掉出世界
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	// 死亡开始：关闭碰撞与移动
	// Begins the death sequence for the character (disables collision, disables movement, etc...)
	UFUNCTION()
	// 死亡开始的回调
	virtual void OnDeathStarted(AActor* OwningActor);

	// 死亡结束：脱离 Controller 并销毁 Pawn
	// Ends the death sequence for the character (detaches controller, destroys pawn, etc...)
	UFUNCTION()
	// 死亡结束的回调
	virtual void OnDeathFinished(AActor* OwningActor);

	// 关闭移动与碰撞
	void DisableMovementAndCollision();
	// 因死亡而销毁
	void DestroyDueToDeath();
	// 反初始化后销毁
	void UninitAndDestroy();

	// 死亡流程走完的蓝图事件
	// Called when the death sequence for the character has completed
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="OnDeathFinished"))
	// 蓝图可实现的死亡完成事件
	void K2_OnDeathFinished();

	// 移动模式变化：同步更新移动状态标签
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	// 增删某个移动模式对应的标签
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

	// 开始下蹲
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	// 结束下蹲
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	// 能否跳跃（受死亡状态影响）
	virtual bool CanJumpInternal_Implementation() const;

private:

	// PawnExtension 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraPawnExtensionComponent> PawnExtComponent;

	// 摄像机组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraHealthComponent> HealthComponent;

	// 生命值组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraCameraComponent> CameraComponent;

	// 复制的加速度
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedAcceleration)
	FLyraReplicatedAcceleration ReplicatedAcceleration;

	// 队伍 ID
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	// 队伍变化委托
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

protected:
	// 决定 Controller 离开后队伍 ID 怎么处理；默认回到无队伍
	// Called to determine what happens to the team ID when possession ends
	virtual FGenericTeamId DetermineNewTeamAfterPossessionEnds(FGenericTeamId OldTeamID) const
	{
		// This could be changed to return, e.g., OldTeamID if you want to keep it assigned afterwards, or return an ID for some neutral faction, or etc...
		return FGenericTeamId::NoTeam;
	}

private:
	// 所属 Controller 的队伍发生变化
	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

	// 复制加速度的 OnRep
	UFUNCTION()
	void OnRep_ReplicatedAcceleration();

	// 队伍 ID 的 OnRep
	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};
