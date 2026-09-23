// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularCharacter.h"
#include "Components/GameFrameworkComponentManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularCharacter)

// 把自己登记为“组件接收者”。
// 只有登记过的 Actor，GameFeature 插件才能通过 UGameFrameworkComponentManager 往它身上动态挂组件。
// 必须早于“Actor 就绪”事件派发，否则组件会挂不上去。
void AModularCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// 派发 NAME_GameActorReady 事件，通知所有监听者“本 Actor 已就绪，可以挂组件了”。
// 注意特意放在 Super::BeginPlay() 之前：确保组件在基类 BeginPlay 逻辑执行前就已就位。
void AModularCharacter::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

// 注销“组件接收者”身份，清掉 GameFrameworkComponentManager 中对本 Actor 的引用，避免悬空。
void AModularCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

