// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

/**
 * CommonUserBasicPresence.h
 *
 * 极简的“富状态（Rich Presence）”推送：监听会话子系统的信息变化，
 * 把“在主菜单 / 匹配中 / 游戏中”以及模式名、地图名推送给平台好友系统。
 */
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonUserBasicPresence.generated.h"

class UCommonSessionSubsystem;
enum class ECommonSessionInformationState : uint8;

//////////////////////////////////////////////////////////////////////
// UCommonUserBasicPresence

/**
 * This subsystem plugs into the session subsystem and pushes its information to the presence interface.
 * It is not intended to be a full featured rich presence implementation, but can be used as a proof-of-concept
 * for pushing information from the session subsystem to the presence system
 */
/**
 * 基础在线状态推送子系统。
 * 它不主动做轮询，只是订阅 UCommonSessionSubsystem::OnSessionInformationChangedEvent，
 * 会话状态一变就推送一次平台 Presence。
 */
UCLASS(BlueprintType, Config = Engine)
class COMMONUSER_API UCommonUserBasicPresence : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonUserBasicPresence();


	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

	// 总开关，false 时整个类不再推送任何 Presence
	/** False is a general purpose killswitch to stop this class from pushing presence*/
	UPROPERTY(Config)
	bool bEnableSessionsBasedPresence = false;

	// “游戏中”状态映射到后端用的键名
	/** Maps the presence status "In-game" to a backend key*/
	UPROPERTY(Config)
	FString PresenceStatusInGame;

	// “主菜单”状态映射到后端用的键名
	/** Maps the presence status "Main Menu" to a backend key*/
	UPROPERTY(Config)
	FString PresenceStatusMainMenu;

	// “匹配中”状态映射到后端用的键名
	/** Maps the presence status "Matchmaking" to a backend key*/
	UPROPERTY(Config)
	FString PresenceStatusMatchmaking;

	// “游戏模式”这条富状态对应的键名
	/** Maps the "Game Mode" rich presence entry to a backend key*/
	UPROPERTY(Config)
	FString PresenceKeyGameMode;

	// “地图名”这条富状态对应的键名
	/** Maps the "Map Name" rich presence entry to a backend key*/
	UPROPERTY(Config)
	FString PresenceKeyMapName;

	// 会话信息变化回调：在这里组织并推送 Presence
	void OnNotifySessionInformationChanged(ECommonSessionInformationState SessionStatus, const FString& GameMode, const FString& MapName);
	// 把 ECommonSessionInformationState 转成后端约定的状态字符串
	FString SessionStateToBackendKey(ECommonSessionInformationState SessionStatus);
};