// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的 Pawn 基类（非人形也用它，例如载具）。
 * 除了继承 ModularPawn，它主要实现了队伍接口 ILyraTeamAgentInterface。
 */
#include "ModularPawn.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraPawn.generated.h"

class AController;
class UObject;
struct FFrame;

// Lyra 的 Pawn 基类
/**
 * ALyraPawn
 */
UCLASS()
class LYRAGAME_API ALyraPawn : public AModularPawn, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:

	ALyraPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// AActor 接口
	//~AActor interface
	// 组件初始化前
	virtual void PreInitializeComponents() override;
	// 结束
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface

	// APawn 接口
	//~APawn interface
	// 被 Controller 占据
	virtual void PossessedBy(AController* NewController) override;
	// 被 Controller 释放
	virtual void UnPossessed() override;
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

protected:
	// 决定 Controller 离开后队伍 ID 怎么处理；默认回到无队伍，子类可改
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

// 队伍 ID（复制到客户端）
private:
	UPROPERTY(ReplicatedUsing = OnRep_MyTeamID)
	FGenericTeamId MyTeamID;

	// 队伍变化委托
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

// 队伍 ID 的 OnRep 回调
private:
	UFUNCTION()
	void OnRep_MyTeamID(FGenericTeamId OldTeamID);
};
