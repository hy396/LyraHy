// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Lyra 的游戏引擎类，主要用途是在 Init 时挂上自定义的资产管理器相关设置
#include "Engine/GameEngine.h"

#include "LyraGameEngine.generated.h"

class IEngineLoop;
class UObject;


// 本项目使用的 UGameEngine 子类
UCLASS()
class ULyraGameEngine : public UGameEngine
{
	GENERATED_BODY()

public:

	ULyraGameEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 引擎初始化入口
	virtual void Init(IEngineLoop* InEngineLoop) override;
};
