// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameSession.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameSession)


// 构造：默认设置
ALyraGameSession::ALyraGameSession(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 恒返回 true 以禁用引擎默认自动登录（Lyra 用 CommonUser 自己管登录）
bool ALyraGameSession::ProcessAutoLogin()
{
	// This is actually handled in LyraGameMode::TryDedicatedServerLogin
	return true;
}

// 比赛开始
void ALyraGameSession::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

// 比赛结束
void ALyraGameSession::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}

