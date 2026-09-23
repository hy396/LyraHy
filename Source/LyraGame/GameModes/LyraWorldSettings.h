// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的世界设置：主要用途是给每张地图指定一个「默认玩法体验」，
 * 这样直接在编辑器里点 Play 也能跑到正确的玩法上。
 */
#include "GameFramework/WorldSettings.h"
#include "LyraWorldSettings.generated.h"

class ULyraExperienceDefinition;

// 默认世界设置类，核心作用是指定本地图的默认体验
/**
 * The default world settings object, used primarily to set the default gameplay experience to use when playing on this map
 */
UCLASS()
class LYRAGAME_API ALyraWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	ALyraWorldSettings(const FObjectInitializer& ObjectInitializer);

// 编辑器下的地图错误检查（例如体验没配）
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif

public:
	// 服务端打开本地图时使用的默认体验；会被玩家从前端选的体验覆盖
	// Returns the default experience to use when a server opens this map if it is not overridden by the user-facing experience
	FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	// 本地图的默认玩法体验（软引用）
	// The default experience to use when a server opens this map if it is not overridden by the user-facing experience
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<ULyraExperienceDefinition> DefaultGameplayExperience;

public:

#if WITH_EDITORONLY_DATA
	// 该关卡是否属于前端/独立体验；勾选后在编辑器里点 Play 会强制 Standalone 网络模式
	// Is this level part of a front-end or other standalone experience?
	// When set, the net mode will be forced to Standalone when you hit Play in the editor
	UPROPERTY(EditDefaultsOnly, Category=PIE)
	bool ForceStandaloneNetMode = false;
#endif
};
