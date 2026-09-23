// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 调试摄像机控制器：作弊管理器切到调试视角时用的控制器
#include "Engine/DebugCameraController.h"

#include "LyraDebugCameraController.generated.h"

class UObject;


// 调试摄像机控制器
/**
 * ALyraDebugCameraController
 *
 *	Used for controlling the debug camera when it is enabled via the cheat manager.
 */
UCLASS()
class ALyraDebugCameraController : public ADebugCameraController
{
	GENERATED_BODY()

public:

	// 构造
	ALyraDebugCameraController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	// 添加作弊命令
	virtual void AddCheats(bool bForce) override;
};
