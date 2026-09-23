// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"

#include "CommonGameInstance.generated.h"

enum class ECommonUserAvailability : uint8;
enum class ECommonUserPrivilege : uint8;

class FText;
class UCommonUserInfo;
class UCommonSession_SearchResult;
struct FOnlineResultInformation;
class ULocalPlayer;
class USocialManager;
class UObject;
struct FFrame;
struct FGameplayTag;

/**
 * UCommonGameInstance —— 增强版 GameInstance：把"在线身份"与"会话"两件事接进游戏。
 *
 * 它在标准 UGameInstance 基础上补了三块：
 *   1) 接收 CommonUser 子系统的通知（系统消息、权限变化、用户初始化结果）
 *   2) 处理"被邀请加入会话"的完整流程（平台好友邀请、平台覆盖层点击等）
 *   3) 管理主玩家 PrimaryPlayer
 * 本类是抽象的，游戏工程应当继承它。
 */
UCLASS(Abstract, Config = Game)
class COMMONGAME_API UCommonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UCommonGameInstance(const FObjectInitializer& ObjectInitializer);
	
	/** 处理来自 CommonUser 的错误/警告消息。游戏可重写以弹出自己的提示 UI。 */
	UFUNCTION()
	virtual void HandleSystemMessage(FGameplayTag MessageType, FText Title, FText Message);

	UFUNCTION()
	virtual void HandlePrivilegeChanged(const UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability);

	UFUNCTION()
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);

	/** 重置用户与会话状态。通常在玩家掉线时调用。 */
	virtual void ResetUserAndSessionState();

	/**
	 * Requested Session Flow
	 *   Something requests the user to join a specific session (for example, a platform overlay via OnUserRequestedSession).
	 *   This request is handled in SetRequestedSession.
	 *   Check if we can join the requested session immediately (CanJoinRequestedSession).  If we can, join the requested session (JoinRequestedSession)
	 *   If not, cache the requested session and instruct the game to get into a state where the session can be joined (ResetGameAndJoinRequestedSession)
	 */
	/** 玩家从外部接受了会话邀请时调用（例如平台好友邀请、平台覆盖层）。
	 *  默认实现只是把它转给 SetRequestedSession；游戏通常要重写以先回到主菜单。 */
	virtual void OnUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* InRequestedSession, const FOnlineResultInformation& RequestedSessionResult);

	/** 取当前"被请求加入"的会话（没有则为 nullptr）。 */
	UCommonSession_SearchResult* GetRequestedSession() const { return RequestedSession; }
	/** 设置（或清空）被请求的会话。一旦设置，"请求加入会话流程"就开始了。 */
	virtual void SetRequestedSession(UCommonSession_SearchResult* InRequestedSession);
	/** 当前是否具备立刻加入该会话的条件（例如已登录、已在主菜单）。游戏可重写。 */
	virtual bool CanJoinRequestedSession() const;
	/** 真正加入被请求的会话。 */
	virtual void JoinRequestedSession();
	/** 把游戏调整到"可以加入会话"的状态（通常是先回到主菜单），然后再加入。 */
	virtual void ResetGameAndJoinRequestedSession();
	
	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	virtual void Init() override;
	virtual void ReturnToMainMenu() override;

private:
	/** 主玩家（分屏时第一个玩家，许多全局性操作都以它为准）。 */
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
	/** 玩家被请求加入的那个会话。 */
	UPROPERTY()
	TObjectPtr<UCommonSession_SearchResult> RequestedSession;
};
