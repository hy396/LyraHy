// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/World.h"

#include "GameUIPolicy.generated.h"

class UCommonLocalPlayer;
class UGameUIManagerSubsystem;
class ULocalPlayer;
class UPrimaryGameLayout;

/**
 * 
 */
/** 本地多人（分屏）时的视口分配模式。 */
UENUM()
enum class ELocalMultiplayerInteractionMode : uint8
{
	/** 只有主玩家占满整个视口，不管还有没有别的玩家。 */
	PrimaryOnly,

	/** 一个视口，但玩家可以来回切换"当前显示谁、谁进入休眠态"。 */
	SingleToggle,

	/** 两个玩家的视口同时显示（真正的分屏）。 */
	Simultaneous
};

/** 某个本地玩家与其根 UI 布局的绑定关系（含"是否已加入视口"标记）。 */
USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPrimaryGameLayout> RootLayout = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FRootViewportLayoutInfo() {}
	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UPrimaryGameLayout* InRootLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		, RootLayout(InRootLayout)
		, bAddedToViewport(bIsInViewport)
	{}

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};

/**
 * UGameUIPolicy —— UI 策略：决定"每个玩家该有一套什么样的 UI"。
 *
 * 它回答三个问题：
 *   1) 分屏时视口怎么分？（LocalMultiplayerInteractionMode）
 *   2) 每个玩家用哪个根布局控件类？（LayoutClass）
 *   3) 布局何时加入/移出视口、何时释放？
 *
 * 本类用 Within = GameUIManagerSubsystem 限定：它只能存在于 UI 管理器内部。
 */
UCLASS(Abstract, Blueprintable, Within = GameUIManagerSubsystem)
class COMMONGAME_API UGameUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	template <typename GameUIPolicyClass = UGameUIPolicy>
	static GameUIPolicyClass* GetGameUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIPolicyClass>(GetGameUIPolicy(WorldContextObject));
	}

	/** 通过任一 UObject 取到当前世界的 UI 策略（内部先取 UIManager 再取 Policy）。 */
	static UGameUIPolicy* GetGameUIPolicy(const UObject* WorldContextObject);

public:
	virtual UWorld* GetWorld() const override;
	UGameUIManagerSubsystem* GetOwningUIManager() const;
	UPrimaryGameLayout* GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const;

	ELocalMultiplayerInteractionMode GetLocalMultiplayerInteractionMode() const { return LocalMultiplayerInteractionMode; }

	/** 请求把"主控制"交给某个布局（SingleToggle 模式下用于切换当前显示的玩家）。 */
	void RequestPrimaryControl(UPrimaryGameLayout* Layout);

protected:
	void AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	void RemoveLayoutFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);

	virtual void OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	virtual void OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	virtual void OnRootLayoutReleased(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);

	/** 为指定玩家创建根布局控件（但还不加入视口）。 */
	void CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer);
	/** 取该玩家该用哪个根布局类。蓝图可重写以按玩家/平台返回不同类。 */
	TSubclassOf<UPrimaryGameLayout> GetLayoutWidgetClass(UCommonLocalPlayer* LocalPlayer);

private:
	/** 分屏视口模式，默认"只显示主玩家"。 */
	ELocalMultiplayerInteractionMode LocalMultiplayerInteractionMode = ELocalMultiplayerInteractionMode::PrimaryOnly;

	/** 根布局控件类（软引用，按需异步加载，避免启动时就把整套 UI 拉进内存）。 */
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UPrimaryGameLayout> LayoutClass;

	/** 每个本地玩家对应的根布局信息表。 */
	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;

private:
	void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

	friend class UGameUIManagerSubsystem;
};
