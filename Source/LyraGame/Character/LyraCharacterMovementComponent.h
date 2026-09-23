// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Lyra 的角色移动组件：主要加了「地面信息缓存」与复制加速用的 FastShared 支持
#include "GameFramework/CharacterMovementComponent.h"
#include "NativeGameplayTags.h"

#include "LyraCharacterMovementComponent.generated.h"

class UObject;
struct FFrame;

// 全局标签：移动已停止
LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

// 角色脚下的地面信息；只在需要时才更新（按帧号缓存）
/**
 * FLyraCharacterGroundInfo
 *
 *	Information about the ground under the character.  It only gets updated as needed.
 */
// 地面信息结构
USTRUCT(BlueprintType)
struct FLyraCharacterGroundInfo
{
	GENERATED_BODY()

	FLyraCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	// 上次更新是在第几帧，用于判断缓存是否过期
	uint64 LastUpdateFrame;

	// 地面命中结果
	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	// 到地面的距离
	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};


// 本项目使用的角色移动组件
/**
 * ULyraCharacterMovementComponent
 *
 *	The base character movement component class used by this project.
 */
// 移动组件类
UCLASS(Config = Game)
class LYRAGAME_API ULyraCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	ULyraCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	// 模拟移动：额外处理复制加速量的模拟
	virtual void SimulateMovement(float DeltaTime) override;

	// 能否尝试跳跃
	virtual bool CanAttemptJump() const override;

	// 取地面信息；缓存过期时会顺带更新一次（所以别直接读成员）
	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "Lyra|CharacterMovement")
	const FLyraCharacterGroundInfo& GetGroundInfo();

	// 设置要复制的加速度（供 FastShared 路径使用）
	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	// 取本帧的旋转增量
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	// 取最大速度
	virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

protected:

	// 组件初始化
	virtual void InitializeComponent() override;

protected:

	// 缓存的地面信息；**不要直接访问**，必须通过 GetGroundInfo 取
	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FLyraCharacterGroundInfo CachedGroundInfo;

	// 是否有需要复制的加速度
	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};
