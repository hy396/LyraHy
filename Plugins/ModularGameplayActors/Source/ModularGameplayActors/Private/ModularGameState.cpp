// Copyright Epic Games, Inc. All Rights Reserved.

#include "ModularGameState.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Components/GameStateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularGameState)

// 把自己登记为“组件接收者”。
// 只有登记过的 Actor，GameFeature 插件才能通过 UGameFrameworkComponentManager 往它身上动态挂组件。
// 必须早于“Actor 就绪”事件派发，否则组件会挂不上去。
void AModularGameStateBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// 派发 NAME_GameActorReady 事件，通知所有监听者“本 Actor 已就绪，可以挂组件了”。
// 注意特意放在 Super::BeginPlay() 之前：确保组件在基类 BeginPlay 逻辑执行前就已就位。
void AModularGameStateBase::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

// 注销“组件接收者”身份，清掉 GameFrameworkComponentManager 中对本 Actor 的引用，避免悬空。
void AModularGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}


// 把自己登记为“组件接收者”。
// 只有登记过的 Actor，GameFeature 插件才能通过 UGameFrameworkComponentManager 往它身上动态挂组件。
// 必须早于“Actor 就绪”事件派发，否则组件会挂不上去。
void AModularGameState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

// 派发 NAME_GameActorReady 事件，通知所有监听者“本 Actor 已就绪，可以挂组件了”。
// 注意特意放在 Super::BeginPlay() 之前：确保组件在基类 BeginPlay 逻辑执行前就已就位。
void AModularGameState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

// 注销“组件接收者”身份，清掉 GameFrameworkComponentManager 中对本 Actor 的引用，避免悬空。
void AModularGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

// 比赛开始时，把这个事件转发给身上所有 UGameStateComponent，
// 让那些由 GameFeature 动态挂上来的状态组件也能响应比赛开始。
void AModularGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	TArray<UGameStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UGameStateComponent* Component : ModularComponents)
	{
		Component->HandleMatchHasStarted();
	}
}

