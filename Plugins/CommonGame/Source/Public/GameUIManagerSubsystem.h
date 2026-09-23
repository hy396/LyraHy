// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"

#include "GameUIManagerSubsystem.generated.h"

class FSubsystemCollectionBase;
class UCommonLocalPlayer;
class UGameUIPolicy;
class UObject;

/**
 * UGameUIManagerSubsystem —— UI 总管理器（GameInstance 子系统）。
 *
 * 它本身不做具体工作，只是持有一份 UGameUIPolicy（UI 策略），
 * 并把"玩家加入/移除/销毁"这些事件转发给策略去处理。
 *
 * 本类是【抽象】的——因为"用哪个 Policy"必须由游戏工程决定，
 * 所以游戏应当继承本类，并在配置里指定 DefaultUIPolicyClass。
 */
UCLASS(Abstract, config = Game)
class COMMONGAME_API UGameUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UGameUIManagerSubsystem() { }
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	/** 取当前生效的 UI 策略。 */
	const UGameUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	UGameUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	/** 有新本地玩家加入 —— 转发给当前 Policy。 */
	virtual void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	/** 有本地玩家被移除 —— 转发给当前 Policy。 */
	virtual void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	/** 有本地玩家被销毁 —— 转发给当前 Policy。 */
	virtual void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

protected:
	/** 切换当前 UI 策略：先通知旧的做清理，再换新的并为已存在的玩家补建布局。 */
	void SwitchToPolicy(UGameUIPolicy* InPolicy);

private:
	UPROPERTY(Transient)
	TObjectPtr<UGameUIPolicy> CurrentPolicy = nullptr;

	/** 默认使用的 UI 策略类（在配置里指定，子类通常会把它设成自己的 Policy）。 */
	UPROPERTY(config, EditAnywhere)
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;
};
