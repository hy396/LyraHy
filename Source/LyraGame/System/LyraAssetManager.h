// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Lyra 的资产管理器（在 DefaultEngine.ini 里用 AssetManagerClassName 指定为本项目使用的类）。
 *
 * 它比引擎默认 UAssetManager 多做了三件事：
 *   1) 提供带权重的「启动作业」流水线（FLyraAssetManagerStartupJob），并汇报加载进度；
 *   2) 持有全局游戏数据资产 ULyraGameData 与兜底 Pawn 数据；
 *   3) 初始化 Lyra 自己的 GameplayCueManager。
 */
#include "Engine/AssetManager.h"
#include "LyraAssetManagerStartupJob.h"
#include "Templates/SubclassOf.h"
#include "LyraAssetManager.generated.h"

class UPrimaryDataAsset;

class ULyraGameData;
class ULyraPawnData;

// 资产包（Bundle）名集合；Equipped 表示「体验真正要用到的那一批资产」
struct FLyraBundles
{
	// 已装备：体验加载时会请求这个 Bundle
	static const FName Equipped;
};


// 本项目使用的资产管理器：重写加载逻辑并持有游戏专用类型
/**
 * ULyraAssetManager
 *
 *	Game implementation of the asset manager that overrides functionality and stores game-specific types.
 *	It is expected that most games will want to override AssetManager as it provides a good place for game-specific loading logic.
 *	This class is used by setting 'AssetManagerClassName' in DefaultEngine.ini.
 */
UCLASS(Config = Game)
class ULyraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	ULyraAssetManager();

	// 取资产管理器单例（内部就是 Cast 引擎的那个单例）
	// Returns the AssetManager singleton object.
	static ULyraAssetManager& Get();

	// 取软引用指向的资产；没加载就同步加载。bKeepInMemory 为真会登记进常驻列表防止被 GC
	// Returns the asset referenced by a TSoftObjectPtr.  This will synchronously load the asset if it's not already loaded.
	template<typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// 取软引用指向的类；没加载就同步加载。bKeepInMemory 同上
	// Returns the subclass referenced by a TSoftClassPtr.  This will synchronously load the asset if it's not already loaded.
	template<typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	// 打印当前所有已加载并被资产管理器跟踪的资产（调试用）
	// Logs all assets currently loaded and tracked by the asset manager.
	static void DumpLoadedAssets();

	// 取全局游戏数据资产（含默认伤害/治疗 GE 等配置）
	const ULyraGameData& GetGameData();
	// 取兜底 Pawn 数据：PlayerState 上没指定 PawnData 时用它
	const ULyraPawnData* GetDefaultPawnData() const;

protected:
	// 按类型取（必要时阻塞加载）一份游戏数据资产，加载结果缓存在 GameDataMap 里
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		if (TObjectPtr<UPrimaryDataAsset> const * pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}

		// Does a blocking load if needed
		return *CastChecked<const GameDataClass>(LoadGameDataOfClass(GameDataClass::StaticClass(), DataPath, GameDataClass::StaticClass()->GetFName()));
	}


	// 同步加载一个资产（带日志开关与耗时统计）
	static UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);
	// 是否打印资产加载日志（由控制台变量控制）
	static bool ShouldLogAssetLoads();

	// 线程安全地把一个资产登记进常驻列表
	// Thread safe way of adding a loaded asset to keep in memory.
	void AddLoadedAsset(const UObject* Asset);

	//~UAssetManager interface
	// 引擎启动时的初始加载入口：在这里排入所有启动作业
	virtual void StartInitialLoading() override;
#if WITH_EDITOR
	// PIE 开始前：重置状态并处理 PIE 特有的加载需求
	virtual void PreBeginPIE(bool bStartSimulate) override;
#endif
	//~End of UAssetManager interface

	// 按类加载一份 PrimaryDataAsset 并登记进 GameDataMap
	UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass, const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath, FPrimaryAssetType PrimaryAssetType);

protected:

	// 全局游戏数据资产的软引用路径（Config，可在 DefaultGame.ini 里改）
	// Global game data asset to use.
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraGameData> LyraGameDataPath;

	// 已加载的游戏数据缓存：类 -> 实例
	// Loaded version of the game data
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;

	// PlayerState 上没设 PawnData 时使用的兜底 Pawn 数据
	// Pawn data used when spawning player pawns if there isn't one set on the player state.
	UPROPERTY(Config)
	TSoftObjectPtr<ULyraPawnData> DefaultPawnData;

private:
	// 依次执行启动作业数组里的全部工作
	// Flushes the StartupJobs array. Processes all startup work.
	void DoAllStartupJobs();

	// 初始化 Lyra 自己的 GameplayCueManager
	// Sets up the ability system
	void InitializeGameplayCueManager();

	// 定期回调加载进度，可拿去驱动加载界面
	// Called periodically during loads, could be used to feed the status to a loading screen
	void UpdateInitialGameContentLoadPercent(float GameContentPercent);

	// 启动时要执行的作业列表，带权重用于计算总体进度
	// The list of tasks to execute on startup. Used to track startup progress.
	TArray<FLyraAssetManagerStartupJob> StartupJobs;

private:
	
	// 已加载并被本管理器跟踪持有的资产集合
	// Assets loaded and tracked by the asset manager.
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	// 修改 LoadedAssets 时用的临界区锁（后台线程也会加资产进来）
	// Used for a scope lock when modifying the list of load assets.
	FCriticalSection LoadedAssetsCritical;
};


// GetAsset 模板实现：先 TryGet，取不到就同步加载，可选择登记常驻
template<typename AssetType>
AssetType* ULyraAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

// GetSubclass 模板实现：同上，只是目标是 UClass
template<typename AssetType>
TSubclassOf<AssetType> ULyraAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			// Added to loaded asset list.
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}
