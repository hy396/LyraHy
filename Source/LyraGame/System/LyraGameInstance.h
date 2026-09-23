// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的游戏实例。继承 CommonGame 的 UCommonGameInstance，
 * 主要负责三件事：会话/登录衔接、网络加密密钥握手、客户端 Travel 前改 URL。
 */
#include "CommonGameInstance.h"

#include "LyraGameInstance.generated.h"

class ALyraPlayerController;
class UObject;

// 本项目使用的游戏实例
UCLASS(Config = Game)
class LYRAGAME_API ULyraGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 取主玩家的 PlayerController
	ALyraPlayerController* GetPrimaryPlayerController() const;
	
	// 当前是否允许加入被请求的会话（例如体验还没准备好时返回 false）
	virtual bool CanJoinRequestedSession() const override;
	// 用户登录完成的回调：成功则继续进入会话流程
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

	// 收到服务端下发的网络加密令牌
	virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	// 收到客户端的加密确认
	virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

protected:

	// 初始化：绑定会话子系统的相关事件
	virtual void Init() override;
	// 关闭：解绑事件
	virtual void Shutdown() override;

	// 客户端 Travel 到会话前的钩子，可在这里往 URL 上追加参数
	void OnPreClientTravelToSession(FString& URL);

	// 硬编码的调试用加密密钥；仅用于验证加密流程，**正式项目绝对不能这么做**
	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;
};
