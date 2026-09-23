// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 作弊管理器：只在非 Shipping 构建里存在，提供大量开发调试命令。
 * 所有命令都通过控制台输入（~ 键），前缀对应 exec 函数名。
 */
#include "GameFramework/CheatManager.h"
#include "LyraCheatManager.generated.h"

class ULyraAbilitySystemComponent;


// 是否启用作弊管理器：非 Shipping 构建默认启用
#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif // #ifndef USING_CHEAT_MANAGER

DECLARE_LOG_CATEGORY_EXTERN(LogLyraCheat, Log, All);


// 作弊管理器基类
/**
 * ULyraCheatManager
 *
 *	Base cheat manager class used by this project.
 */
UCLASS(config = Game, Within = PlayerController, MinimalAPI)
class ULyraCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:

	// 构造
	ULyraCheatManager();

	// 初始化：注册作弊命令
	virtual void InitCheatManager() override;

	// 把文本同时输出到控制台和日志
	// Helper function to write text to the console and to the log.
	static void CheatOutputText(const FString& TextToOutput);

	// 在服务端对 owning player 执行作弊命令
	// Runs a cheat on the server for the owning player.
	UFUNCTION(exec)
	void Cheat(const FString& Msg);

	// 在服务端对所有玩家执行作弊命令
	// Runs a cheat on the server for the all players.
	UFUNCTION(exec)
	void CheatAll(const FString& Msg);

	// 开始下一场比赛
	// Starts the next match
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void PlayNextGame();

	// 切换固定摄像机
	UFUNCTION(Exec)
	virtual void ToggleFixedCamera();

	// 循环切换调试摄像机
	UFUNCTION(Exec)
	virtual void CycleDebugCameras();

	// 循环切换 ASC 调试显示
	UFUNCTION(Exec)
	virtual void CycleAbilitySystemDebug();

	// 强制取消所有已激活的技能
	// Forces input activated abilities to be canceled.  Useful for tracking down ability interruption bugs. 
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void CancelActivatedAbilities();

	// 给自己加一个动态标签
	// Adds the dynamic tag to the owning player's ability system component.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void AddTagToSelf(FString TagName);

	// 从自己身上移除一个动态标签
	// Removes the dynamic tag from the owning player's ability system component.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void RemoveTagFromSelf(FString TagName);

	// 对自己造成伤害
	// Applies the specified damage amount to the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelf(float DamageAmount);

	// 对视线目标造成伤害
	// Applies the specified damage amount to the actor that the player is looking at.
	virtual void DamageTarget(float DamageAmount) override;

	// 对自己进行治疗
	// Applies the specified amount of healing to the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void HealSelf(float HealAmount);

	// 对视线目标进行治疗
	// Applies the specified amount of healing to the actor that the player is looking at.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void HealTarget(float HealAmount);

	// 自毁（致死伤害）
	// Applies enough damage to kill the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelfDestruct();

	// 无敌（God Mode）
	// Prevents the owning player from taking any damage.
	virtual void God() override;

	// 无限血量（血量不会降到 1 以下）
	// Prevents the owning player from dropping below 1 health.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void UnlimitedHealth(int32 Enabled = -1);

protected:

	// 启用调试摄像机
	virtual void EnableDebugCamera() override;
	// 禁用调试摄像机
	virtual void DisableDebugCamera() override;
	// 是否正在使用调试摄像机
	bool InDebugCamera() const;

	// 启用固定摄像机
	virtual void EnableFixedCamera();
	// 禁用固定摄像机
	virtual void DisableFixedCamera();
	// 是否正在使用固定摄像机
	bool InFixedCamera() const;

	// 通过 SetByCaller 对自己施加伤害
	void ApplySetByCallerDamage(ULyraAbilitySystemComponent* LyraASC, float DamageAmount);
	// 通过 SetByCaller 对自己施加治疗
	void ApplySetByCallerHeal(ULyraAbilitySystemComponent* LyraASC, float HealAmount);

	// 取 owning player 的 ASC
	ULyraAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;
};
