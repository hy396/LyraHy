// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameEngine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameEngine)

class IEngineLoop;


// 构造：默认设置
ULyraGameEngine::ULyraGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 引擎初始化入口
void ULyraGameEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}

