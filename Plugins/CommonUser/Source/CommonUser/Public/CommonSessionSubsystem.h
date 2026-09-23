// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * CommonSessionSubsystem.h
 *
 * 联机会话管理：创建房间（Host）、搜索房间（Find）、快速匹配（QuickPlay）、
 * 加入房间（Join）与会话清理，并在成功后执行 ServerTravel / ClientTravel。
 *
 * 与 CommonUser 配合：先由 UCommonUserSubsystem 完成登录，再由本子系统建/进房间。
 */
#include "CommonUserTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/ObjectPtr.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/PrimaryAssetId.h"
#include "UObject/WeakObjectPtr.h"

class APlayerController;
class ULocalPlayer;
namespace ETravelFailure { enum Type : int; }
struct FOnlineResultInformation;

#if COMMONUSER_OSSV1
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#else
#include "Online/Lobbies.h"
#include "Online/OnlineAsyncOpHandle.h"
#endif // COMMONUSER_OSSV1

#include "CommonSessionSubsystem.generated.h"

class UWorld;
class FCommonSession_OnlineSessionSettings;

#if COMMONUSER_OSSV1
class FCommonOnlineSearchSettingsOSSv1;
using FCommonOnlineSearchSettings = FCommonOnlineSearchSettingsOSSv1;
#else
class FCommonOnlineSearchSettingsOSSv2;
using FCommonOnlineSearchSettings = FCommonOnlineSearchSettingsOSSv2;
#endif // COMMONUSER_OSSV1


//////////////////////////////////////////////////////////////////////
// UCommonSession_HostSessionRequest

/** Specifies the online features and connectivity that should be used for a game session */
UENUM(BlueprintType)
/**
 * 会话的联机模式：离线、局域网、完整联机。
 */
enum class ECommonSessionOnlineMode : uint8
{
	// 离线：不建立任何在线会话，纯单机
	Offline,
	// 局域网：在同一局域网内广播/搜索
	LAN,
	// 联机：走平台/服务的在线会话或大厅
	Online
};

/** A request object that stores the parameters used when hosting a gameplay session */
/**
 * 建房请求对象：保存开房所需的一切参数，创建后可在蓝图里继续修改。
 * MapID 必须是合法的 World 主资源（PrimaryAssetId），否则 ServerTravel 会失败。
 */
UCLASS(BlueprintType)
class COMMONUSER_API UCommonSession_HostSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	// 本次会话是完整联机会话，还是局域网/离线等其他类型
	/** Indicates if the session is a full online session or a different type */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	ECommonSessionOnlineMode OnlineMode;

	// 是否优先使用“玩家自建大厅（Lobby）”，平台支持时生效
	/** True if this request should create a player-hosted lobbies if available */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	// 匹配时对外广播的模式名，用于筛选同类玩法
	/** String used during matchmaking to specify what type of game mode this is */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	FString ModeNameForAdvertisement;

	// 开局要加载的地图，必须是合法的 World 主资源
	/** The map that will be loaded at the start of gameplay, this needs to be a valid Primary Asset top-level map */
	UPROPERTY(BlueprintReadWrite, Category=Session, meta=(AllowedTypes="World"))
	FPrimaryAssetId MapID;

	// 以 URL 参数形式附加传给游戏的额外参数
	/** Extra arguments passed as URL options to the game */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	TMap<FString, FString> ExtraArgs;

	// 单个会话允许的最大玩家数
	/** Maximum players allowed per gameplay session */
	UPROPERTY(BlueprintReadWrite, Category=Session)
	int32 MaxPlayerCount = 16;

public:
	// 实际使用的最大玩家数，子类可重写
	/** Returns the maximum players that should actually be used, could be overridden in child classes */
	virtual int32 GetMaxPlayers() const;

	// 返回开局要使用的完整地图名
	/** Returns the full map name that will be used during gameplay */
	virtual FString GetMapName() const;

	// 拼出传给 ServerTravel 的完整 URL（含 ExtraArgs）
	/** Constructs the full URL that will be passed to ServerTravel */
	virtual FString ConstructTravelURL() const;

	// 校验请求是否合法，不合法时写入 OutError 并打日志，返回 false
	/** Returns true if this request is valid, returns false and logs errors if it is not */
	virtual bool ValidateAndLogErrors(FText& OutError) const;
};


//////////////////////////////////////////////////////////////////////
// UCommonSession_SearchResult

/** A result object returned from the online system that describes a joinable game session */
/**
 * 搜索结果对象：包装平台相关的底层会话/Lobby 数据，对外提供统一读取接口。
 */
UCLASS(BlueprintType)
class COMMONUSER_API UCommonSession_SearchResult : public UObject
{
	GENERATED_BODY()

public:
	// 返回内部描述字符串，面向调试而非玩家展示
	/** Returns an internal description of the session, not meant to be human readable */
	UFUNCTION(BlueprintCallable, Category=Session)
	FString GetDescription() const;

	// 读取任意字符串设置，不存在时 bFoundValue 为 false
	/** Gets an arbitrary string setting, bFoundValue will be false if the setting does not exist */
	UFUNCTION(BlueprintPure, Category=Sessions)
	void GetStringSetting(FName Key, FString& Value, bool& bFoundValue) const;

	// 读取任意整数设置，不存在时 bFoundValue 为 false
	/** Gets an arbitrary integer setting, bFoundValue will be false if the setting does not exist */
	UFUNCTION(BlueprintPure, Category = Sessions)
	void GetIntSetting(FName Key, int32& Value, bool& bFoundValue) const;

	// 可用的私有连接数
	/** The number of private connections that are available */
	UFUNCTION(BlueprintPure, Category=Sessions)
	int32 GetNumOpenPrivateConnections() const;

	// 可用的公开连接数
	/** The number of publicly available connections that are available */
	UFUNCTION(BlueprintPure, Category=Sessions)
	int32 GetNumOpenPublicConnections() const;

	// 公开连接总数上限（含已占用的）
	/** The maximum number of publicly available connections that could be available, including already filled connections */
	UFUNCTION(BlueprintPure, Category = Sessions)
	int32 GetMaxPublicConnections() const;

	// 到该结果的延迟；MAX_QUERY_PING 表示不可达
	/** Ping to the search result, MAX_QUERY_PING is unreachable */
	UFUNCTION(BlueprintPure, Category=Sessions)
	int32 GetPingInMs() const;

public:
	/** Pointer to the platform-specific implementation */
// 平台相关的底层结果对象：OSSv1 是 FOnlineSessionSearchResult，OSSv2 是 Lobby 指针
#if COMMONUSER_OSSV1
	FOnlineSessionSearchResult Result;
#else
	TSharedPtr<const UE::Online::FLobby> Lobby;
#endif // COMMONUSER_OSSV1

};


//////////////////////////////////////////////////////////////////////
// UCommonSession_SearchSessionRequest

/** Delegates called when a session search completes */
// 原生多播委托：一次会话搜索结束时触发
DECLARE_MULTICAST_DELEGATE_TwoParams(FCommonSession_FindSessionsFinished, bool bSucceeded, const FText& ErrorMessage);
// 蓝图版搜索结束委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCommonSession_FindSessionsFinishedDynamic, bool, bSucceeded, FText, ErrorMessage);

/** Request object describing a session search, this object will be updated once the search has completed */
/**
 * 搜索请求对象：搜索完成后本对象会被填充（Results）并触发 OnSearchFinished。
 */
UCLASS(BlueprintType)
class COMMONUSER_API UCommonSession_SearchSessionRequest : public UObject
{
	GENERATED_BODY()

public:
	// 要搜索的联机模式（联机 / 局域网 / 离线）
	/** Indicates if the this is looking for full online games or a different type like LAN */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	ECommonSessionOnlineMode OnlineMode;

	// true 搜索玩家自建大厅；false 只搜索已注册的专用服务器会话
	/** True if this request should look for player-hosted lobbies if they are available, false will only search for registered server sessions */
	UPROPERTY(BlueprintReadWrite, Category = Session)
	bool bUseLobbies;

	// 搜索到的全部会话，在 OnSearchFinished 触发时有效
	/** List of all found sessions, will be valid when OnSearchFinished is called */
	UPROPERTY(BlueprintReadOnly, Category=Session)
	TArray<TObjectPtr<UCommonSession_SearchResult>> Results;

	// 原生委托：搜索完成时触发
	/** Native Delegate called when a session search completes */
	FCommonSession_FindSessionsFinished OnSearchFinished;

	// 由子系统调用以统一触发原生与蓝图两个版本的完成委托
	/** Called by subsystem to execute finished delegates */
	void NotifySearchFinished(bool bSucceeded, const FText& ErrorMessage);

private:
	// 蓝图可绑定的搜索完成事件（私有访问，通过 NotifySearchFinished 触发）
	/** Delegate called when a session search completes */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Search Finished", AllowPrivateAccess = true))
	FCommonSession_FindSessionsFinishedDynamic K2_OnSearchFinished;
};


//////////////////////////////////////////////////////////////////////
// CommonSessionSubsystem Events

/**
 * Event triggered when the local user has requested to join a session from an external source, for example from a platform overlay.
 * Generally, the game should transition the player into the session.
 * @param LocalPlatformUserId the local user id that accepted the invitation. This is a platform user id because the user might not be signed in yet.
 * @param RequestedSession the requested session. Can be null if there was an error processing the request.
 * @param RequestedSessionResult result of the requested session processing
 */
// 玩家从外部（例如平台好友邀请浮层）请求加入某个会话时触发；游戏应把玩家送进该会话
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnUserRequestedSession, const FPlatformUserId& /*LocalPlatformUserId*/, UCommonSession_SearchResult* /*RequestedSession*/, const FOnlineResultInformation& /*RequestedSessionResult*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnUserRequestedSession_Dynamic, const FPlatformUserId&, LocalPlatformUserId, UCommonSession_SearchResult*, RequestedSession, const FOnlineResultInformation&, RequestedSessionResult);

/**
 * Event triggered when a session join has completed, after joining the underlying session and before traveling to the server if it was successful.
 * The event parameters indicate if this was successful, or if there was an error that will stop it from traveling.
 * @param Result result of the session join
 */
// 加入会话流程完成、即将（或因失败不会）Travel 到服务器时触发
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonSessionOnJoinSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonSessionOnJoinSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * Event triggered when a session creation for hosting has completed, right before it travels to the map.
 * The event parameters indicate if this was successful, or if there was an error that will stop it from traveling.
 * @param Result result of the session join
 */
// 建房（CreateSession）完成、即将 Travel 到地图时触发
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonSessionOnCreateSessionComplete, const FOnlineResultInformation& /*Result*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCommonSessionOnCreateSessionComplete_Dynamic, const FOnlineResultInformation&, Result);

/**
 * Event triggered when a session join has completed, after resolving the connect string and prior to the client traveling.
 * @param URL resolved connection string for the session with any additional arguments
 */
// 客户端解析出连接串之后、真正 ClientTravel 之前触发，可就地修改 URL
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonSessionOnPreClientTravel, FString& /*URL*/);

/**
 * Event triggered at different points in the session ecosystem that represent a user-presentable state of the session.
 * This should not be used for online functionality (use OnCreateSessionComplete or OnJoinSessionComplete for those) but for features such as rich presence
 */
/**
 * 会话的“可展示状态”，用于好友状态/富文本展示（Rich Presence）。
 * 不要用它判断在线流程，流程请用 OnCreateSessionComplete / OnJoinSessionComplete。
 */
UENUM(BlueprintType)
enum class ECommonSessionInformationState : uint8
{
	// 未在游戏中
	OutOfGame,
	// 正在匹配中
	Matchmaking,
	// 已在游戏对局中
	InGame
};
// 会话可展示状态发生变化时触发（状态 + 模式名 + 地图名）
DECLARE_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnSessionInformationChanged, ECommonSessionInformationState /*SessionStatus*/, const FString& /*GameMode*/, const FString& /*MapName*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCommonSessionOnSessionInformationChanged_Dynamic, ECommonSessionInformationState, SessionStatus, const FString&, GameMode, const FString&, MapName);

//////////////////////////////////////////////////////////////////////
// UCommonSessionSubsystem

/** 
 * Game subsystem that handles requests for hosting and joining online games.
 * One subsystem is created for each game instance and can be accessed from blueprints or C++ code.
 * If a game-specific subclass exists, this base subsystem will not be created.
 */
/**
 * 会话子系统：一个 GameInstance 一份，蓝图与 C++ 均可访问。
 * 如果存在游戏自定义子类，则不会创建这个基类实例（靠 ShouldCreateSubsystem 控制）。
 */
UCLASS(BlueprintType, Config=Engine)
class COMMONUSER_API UCommonSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCommonSessionSubsystem() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// 创建一个带默认联机参数的建房请求，创建后仍可修改
	/** Creates a host session request with default options for online games, this can be modified after creation */
	UFUNCTION(BlueprintCallable, Category = Session)
	virtual UCommonSession_HostSessionRequest* CreateOnlineHostSessionRequest();

	// 创建一个带默认参数的搜索请求，创建后仍可修改
	/** Creates a session search object with default options to look for default online games, this can be modified after creation */
	UFUNCTION(BlueprintCallable, Category = Session)
	virtual UCommonSession_SearchSessionRequest* CreateOnlineSearchSessionRequest();

	// 按请求开一个新房间；成功后会执行一次硬切地图（ServerTravel）
	/** Creates a new online game using the session request information, if successful this will start a hard map transfer */
	UFUNCTION(BlueprintCallable, Category=Session)
	virtual void HostSession(APlayerController* HostingPlayer, UCommonSession_HostSessionRequest* Request);

	// 快速匹配：先找现成房间，找不到就用这份请求自己开一个
	/** Starts a process to look for existing sessions or create a new one if no viable sessions are found */
	UFUNCTION(BlueprintCallable, Category=Session)
	virtual void QuickPlaySession(APlayerController* JoiningOrHostingPlayer, UCommonSession_HostSessionRequest* Request);

	// 加入一个已存在的会话；成功后会连接到指定服务器
	/** Starts process to join an existing session, if successful this will connect to the specified server */
	UFUNCTION(BlueprintCallable, Category=Session)
	virtual void JoinSession(APlayerController* JoiningPlayer, UCommonSession_SearchResult* Request);

	// 按搜索请求向在线系统查询可加入的会话列表
	/** Queries online system for the list of joinable sessions matching the search request */
	UFUNCTION(BlueprintCallable, Category=Session)
	virtual void FindSessions(APlayerController* SearchingPlayer, UCommonSession_SearchSessionRequest* Request);

	// 清理所有活动会话，例如返回主菜单时调用
	/** Clean up any active sessions, called from cases like returning to the main menu */
	UFUNCTION(BlueprintCallable, Category=Session)
	virtual void CleanUpSessions();

	//////////////////////////////////////////////////////////////////////
	// Events

	// 原生委托：本地用户接受了邀请
	/** Native Delegate when a local user has accepted an invite */
	FCommonSessionOnUserRequestedSession OnUserRequestedSessionEvent;
	/** Event broadcast when a local user has accepted an invite */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On User Requested Session"))
	FCommonSessionOnUserRequestedSession_Dynamic K2_OnUserRequestedSessionEvent;

	// 原生委托：JoinSession 流程结束
	/** Native Delegate when a JoinSession call has completed */
	FCommonSessionOnJoinSessionComplete OnJoinSessionCompleteEvent;
	/** Event broadcast when a JoinSession call has completed */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Join Session Complete"))
	FCommonSessionOnJoinSessionComplete_Dynamic K2_OnJoinSessionCompleteEvent;

	// 原生委托：CreateSession 流程结束
	/** Native Delegate when a CreateSession call has completed */
	FCommonSessionOnCreateSessionComplete OnCreateSessionCompleteEvent;
	/** Event broadcast when a CreateSession call has completed */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Create Session Complete"))
	FCommonSessionOnCreateSessionComplete_Dynamic K2_OnCreateSessionCompleteEvent;

	// 原生委托：会话可展示信息发生变化
	/** Native Delegate when the presentable session information has changed */
	FCommonSessionOnSessionInformationChanged OnSessionInformationChangedEvent;
	/** Event broadcast when the presentable session information has changed */
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "On Session Information Changed"))
	FCommonSessionOnSessionInformationChanged_Dynamic K2_OnSessionInformationChangedEvent;

	// 原生委托：客户端 Travel 之前，可就地修改连接 URL
	/** Native Delegate for modifying the connect URL prior to a client travel */
	FCommonSessionOnPreClientTravel OnPreClientTravelEvent;

	// Config settings, these can overridden in child classes or config files

	// 建房/搜索请求中 bUseLobbies 的默认值，可在子类或配置文件里覆盖
	/** Sets the default value of bUseLobbies for session search and host requests */
	UPROPERTY(Config)
	bool bUseLobbiesDefault = true;

protected:
	// Functions called during the process of creating or joining a session, these can be overidden for game-specific behavior

	// 由快速匹配的建房参数填充出一份搜索条件，子类可重写以加入自定义筛选
	/** Called to fill in a session request from quick play host settings, can be overridden for game-specific behavior */
	virtual TSharedRef<FCommonOnlineSearchSettings> CreateQuickPlaySearchSettings(UCommonSession_HostSessionRequest* Request, UCommonSession_SearchSessionRequest* QuickPlayRequest);

	// 快速匹配搜索结束时调用：有结果就加入，没结果就自己建房
	/** Called when a quick play search finishes, can be overridden for game-specific behavior */
	virtual void HandleQuickPlaySearchFinished(bool bSucceeded, const FText& ErrorMessage, TWeakObjectPtr<APlayerController> JoiningOrHostingPlayer, TStrongObjectPtr<UCommonSession_HostSessionRequest> HostRequest);

	// 切换到会话地图失败时调用
	/** Called when traveling to a session fails */
	virtual void TravelLocalSessionFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString);

	// 会话创建成功或失败时调用
	/** Called when a new session is either created or fails to be created */
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	// 收尾会话创建流程（成功后开始 Travel）
	/** Called to finalize session creation */
	virtual void FinishSessionCreation(bool bWasSuccessful);

	// 已切换到新建会话的地图之后调用
	/** Called after traveling to the new hosted session map */
	virtual void HandlePostLoadMap(UWorld* World);

protected:
	// Internal functions for initializing and handling results from the online systems

	// 绑定底层在线系统的各类回调
	void BindOnlineDelegates();
	// 与版本无关的开房入口，内部再分派到 OSSv1 / OSSv2 实现
	void CreateOnlineSessionInternal(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request);
	// 与版本无关的搜索入口
	void FindSessionsInternal(APlayerController* SearchingPlayer, const TSharedRef<FCommonOnlineSearchSettings>& InSearchSettings);
	// 与版本无关的加入入口
	void JoinSessionInternal(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request);
	// 内部统一的 Travel 到会话逻辑
	void InternalTravelToSession(const FName SessionName);
	// 统一触发“用户请求加入会话”事件（原生 + 蓝图）
	void NotifyUserRequestedSession(const FPlatformUserId& PlatformUserId, UCommonSession_SearchResult* RequestedSession, const FOnlineResultInformation& RequestedSessionResult);
	// 统一触发“加入会话完成”事件
	void NotifyJoinSessionComplete(const FOnlineResultInformation& Result);
	// 统一触发“创建会话完成”事件
	void NotifyCreateSessionComplete(const FOnlineResultInformation& Result);
	// 统一触发“会话可展示信息变化”事件
	void NotifySessionInformationUpdated(ECommonSessionInformationState SessionStatusStr, const FString& GameMode = FString(), const FString& MapName = FString());
	// 记录建房错误信息，供稍后展示
	void SetCreateSessionError(const FText& ErrorText);

// 以下为 OnlineSubsystem（OSS v1）实现分支
#if COMMONUSER_OSSV1
	void BindOnlineDelegatesOSSv1();
	void CreateOnlineSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request);
	void FindSessionsInternalOSSv1(ULocalPlayer* LocalPlayer);
	void JoinSessionInternalOSSv1(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request);
	TSharedRef<FCommonOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv1(UCommonSession_HostSessionRequest* Request, UCommonSession_SearchSessionRequest* QuickPlayRequest);
	void CleanUpSessionsOSSv1();

	void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);
	void HandleSessionUserInviteAccepted(const bool bWasSuccessful, const int32 LocalUserIndex, FUniqueNetIdPtr AcceptingUserId, const FOnlineSessionSearchResult& SearchResult);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnRegisterLocalPlayerComplete_CreateSession(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);
	void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

// 以下为 OnlineServices（OSS v2）实现分支
#else
	void BindOnlineDelegatesOSSv2();
	void CreateOnlineSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UCommonSession_HostSessionRequest* Request);
	void FindSessionsInternalOSSv2(ULocalPlayer* LocalPlayer);
	void JoinSessionInternalOSSv2(ULocalPlayer* LocalPlayer, UCommonSession_SearchResult* Request);
	TSharedRef<FCommonOnlineSearchSettings> CreateQuickPlaySearchSettingsOSSv2(UCommonSession_HostSessionRequest* HostRequest, UCommonSession_SearchSessionRequest* SearchRequest);
	void CleanUpSessionsOSSv2();

	// 处理来自在线服务（例如大厅 UI）的加入请求
	/** Process a join request originating from the online service */
	void OnSessionJoinRequested(const UE::Online::FUILobbyJoinRequested& EventParams);

	// 取指定控制器对应的账号 ID
	/** Get the local user id for a given controller */
	UE::Online::FAccountId GetAccountId(APlayerController* PlayerController) const;
	// 取指定会话名对应的大厅 ID
	/** Get the lobby id for a given session name */
	UE::Online::FLobbyId GetLobbyId(const FName SessionName) const;
	// “UI 请求加入大厅”事件的句柄，析构/反初始化时用于解绑
	/** Event handle for UI lobby join requested */
	UE::Online::FOnlineEventDelegateHandle LobbyJoinRequestedHandle;
#endif // COMMONUSER_OSSV1

protected:
	// 会话相关操作完成后要跳转到的 URL
	/** The travel URL that will be used after session operations are complete */
	FString PendingTravelURL;

	// 最近一次建房的结果（含错误码），保存下来以便稍后展示
	/** Most recent result information for a session creation attempt, stored here to allow storing error codes for later */
	FOnlineResultInformation CreateSessionResult;

	// 建好之后是否要立刻销毁该会话（用于出错回滚）
	/** True if we want to cancel the session after it is created */
	bool bWantToDestroyPendingSession = false;

	// 是否专用服务器；专用服务器没有 LocalPlayer 也能建房
	/** True if this is a dedicated server, which doesn't require a LocalPlayer to create a session */
	bool bIsDedicatedServer = false;

	// 当前进行中的搜索设置
	/** Settings for the current search */
	TSharedPtr<FCommonOnlineSearchSettings> SearchSettings;

	// 当前进行中的建房设置
	/** Settings for the current host request */
	TSharedPtr<FCommonSession_OnlineSessionSettings> HostSettings;
};
