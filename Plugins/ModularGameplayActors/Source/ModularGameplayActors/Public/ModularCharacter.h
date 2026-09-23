// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"

#include "ModularCharacter.generated.h"

class UObject;

/**
 * AModularCharacter —— 可被 GameFeature 插件动态扩展的角色（Character）基类。
 *
 * 本类自身几乎不含游戏逻辑，只负责在生命周期的合适时机与 UGameFrameworkComponentManager 对接：
 *   登记为组件接收者 → 派发“Actor 就绪”事件 → 销毁时注销。
 * 这样 GameFeature 插件无需修改本类代码，就能往角色上动态挂组件（例如血量组件、装备组件）。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	//~ Begin AActor Interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
