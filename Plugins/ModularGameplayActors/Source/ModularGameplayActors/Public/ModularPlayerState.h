// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerState.h"

#include "ModularPlayerState.generated.h"

namespace EEndPlayReason { enum Type : int; }

class UObject;

/**
 * AModularPlayerState —— 可被 GameFeature 插件动态扩展的玩家状态基类。
 *
 * 除了标准的“登记 / 派发就绪 / 注销”三件事之外，本类还额外做了转发：
 *   - Reset()          转发给身上所有 UPlayerStateComponent
 *   - CopyProperties() 把属性逐个拷贝给目标 PlayerState 上的同名同类型组件
 *     （用于玩家重连 / 换座位时，把旧 PlayerState 的数据搬到新的上面）
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;
	//~ End AActor interface

protected:
	//~ Begin APlayerState interface
	virtual void CopyProperties(APlayerState* PlayerState);
	//~ End APlayerState interface
};
