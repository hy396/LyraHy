// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Lyra 的 GameSession：关掉引擎默认的自动登录，并把比赛开始/结束接给会话系统
#include "GameFramework/GameSession.h"

#include "LyraGameSession.generated.h"

class UObject;


// 本项目使用的 GameSession
UCLASS(Config = Game)
class ALyraGameSession : public AGameSession
{
	GENERATED_BODY()

public:

	ALyraGameSession(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 重写以禁用引擎默认的自动登录（Lyra 走 CommonUser 的登录流程）
	/** Override to disable the default behavior */
	virtual bool ProcessAutoLogin() override;

	// 比赛开始
	virtual void HandleMatchHasStarted() override;
	// 比赛结束
	virtual void HandleMatchHasEnded() override;
};
