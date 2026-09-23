// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Lyra 的显著性管理器；目前只是留出继承点，方便以后按距离/重要性做 LOD 与更新频率控制
#include "SignificanceManager.h"

#include "LyraSignificanceManager.generated.h"

class UObject;

// 显著性管理器子类，为将来的自定义显著性策略预留
UCLASS()
class ULyraSignificanceManager : public USignificanceManager
{
	GENERATED_BODY()

};
