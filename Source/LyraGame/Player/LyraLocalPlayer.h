// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的本地玩家：继承 CommonLocalPlayer，额外持有队伍、设置与音频设备状态。
 * 它负责加载本地设置（Config）与共享设置（SaveGame），并在切换控制器时同步队伍。
 */
#include "CommonLocalPlayer.h"
#include "Teams/LyraTeamAgentInterface.h"

#include "LyraLocalPlayer.generated.h"

struct FGenericTeamId;

class APlayerController;
class UInputMappingContext;
class ULyraSettingsLocal;
class ULyraSettingsShared;
class UObject;
class UWorld;
struct FFrame;
struct FSwapAudioOutputResult;

// 本项目使用的本地玩家类
/**
 * ULyraLocalPlayer
 */
UCLASS()
class LYRAGAME_API ULyraLocalPlayer : public UCommonLocalPlayer, public ILyraTeamAgentInterface
{
	GENERATED_BODY()

public:

	// 构造
	ULyraLocalPlayer();

	// UObject 接口
	//~UObject interface
	// 属性初始化后
	virtual void PostInitProperties() override;
	//~End of UObject interface

	// UPlayer 接口
	//~UPlayer interface
	// 切换控制器
	virtual void SwitchController(class APlayerController* PC) override;
	//~End of UPlayer interface

	// ULocalPlayer 接口
	//~ULocalPlayer interface
	// 生成玩家 Actor
	virtual bool SpawnPlayActor(const FString& URL, FString& OutError, UWorld* InWorld) override;
	// 初始化在线会话
	virtual void InitOnlineSession() override;
	//~End of ULocalPlayer interface

	// 队伍接口
	//~ILyraTeamAgentInterface interface
	// 设置队伍 ID
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	// 取队伍 ID
	virtual FGenericTeamId GetGenericTeamId() const override;
	// 取队伍变化委托
	virtual FOnLyraTeamIndexChangedDelegate* GetOnTeamIndexChangedDelegate() override;
	//~End of ILyraTeamAgentInterface interface

	// 取本地设置（从配置文件读取，进程启动时就可用）
	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	ULyraSettingsLocal* GetLocalSettings() const;

	// 取共享设置（从存档系统读取，登录后才保证正确）
	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	UFUNCTION()
	ULyraSettingsShared* GetSharedSettings() const;

	// 异步加载共享设置；加载或创建完成后回调 OnSharedSettingsLoaded
	/** Starts an async request to load the shared settings, this will call OnSharedSettingsLoaded after loading or creating new ones */
	void LoadSharedSettingsFromDisk(bool bForceLoad = false);

protected:
	// 共享设置加载完成
	void OnSharedSettingsLoaded(ULyraSettingsShared* LoadedOrCreatedSettings);

	// 音频输出设备变化
	void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	// 音频设备切换完成
	UFUNCTION()
	void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);

	// 所属 Controller 变化
	void OnPlayerControllerChanged(APlayerController* NewController);

	// 所属 Controller 换队伍
	UFUNCTION()
	void OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam);

private:
	// 共享设置缓存
	UPROPERTY(Transient)
	mutable TObjectPtr<ULyraSettingsShared> SharedSettings;

	// 用于加载共享设置时匹配存档的 NetId
	FUniqueNetIdRepl NetIdForSharedSettings;

	// 缓存的输入映射上下文
	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;

	// 队伍变化委托
	UPROPERTY()
	FOnLyraTeamIndexChangedDelegate OnTeamChangedDelegate;

	// 上次绑定的控制器
	UPROPERTY()
	TWeakObjectPtr<APlayerController> LastBoundPC;
};
