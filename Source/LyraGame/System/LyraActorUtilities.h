// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Actor 相关的通用工具，以及一个蓝图可用的网络模式枚举
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LyraActorUtilities.generated.h"

class UObject;
struct FFrame;

// 暴露给蓝图的网络模式枚举（引擎的 ENetMode 蓝图里不好用）
UENUM()
enum class EBlueprintExposedNetMode : uint8
{
	// 单机：没有网络，但拥有全部服务端功能，也算「服务端」
	/** Standalone: a game without networking, with one or more local players. Still considered a server because it has all server functionality. */
	Standalone,

	// 专用服务器：没有本地玩家
	/** Dedicated server: server with no local players. */
	DedicatedServer,

	// 监听服务器：服务端本身也有一个本地玩家
	/** Listen server: a server that also has a local player who is hosting the game, available to other players on the network. */
	ListenServer,

	// 网络客户端：连接到远端服务器
	// 注意枚举顺序——所有小于 Client 的值都是某种服务端，所以 NetMode < Client 即「是服务端」
	/**
	 * Network client: client connected to a remote server.
	 * Note that every mode less than this value is a kind of server, so checking NetMode < NM_Client is always some variety of server.
	 */
	Client
};


// Actor 工具函数库
UCLASS()
class ULyraActorUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 以枚举分支的形式把当前网络模式暴露给蓝图
	/**
	 * Get the network mode (dedicated server, client, standalone, etc...) for an actor or component.
	 */
	UFUNCTION(BlueprintCallable, Category="Lyra", meta=(WorldContext="WorldContextObject", ExpandEnumAsExecs=ReturnValue))
	static EBlueprintExposedNetMode SwitchOnNetMode(const UObject* WorldContextObject);
};
