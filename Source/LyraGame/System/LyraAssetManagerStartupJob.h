// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 启动作业：引擎启动阶段要做的「一件加载工作」。
 *
 * 每个作业有名字、权重和一个执行函数；资产管理器按权重算总进度，
 * 作业内部还能通过 SubstepProgressDelegate 汇报子进度，用于驱动启动加载界面。
 */
#include "Engine/StreamableManager.h"

// 子进度汇报委托，参数是新的进度值（0~1）
DECLARE_DELEGATE_OneParam(FLyraAssetManagerStartupJobSubstepProgress, float /*NewProgress*/);

// 一个启动作业：把 FStreamableHandle 的进度转发成统一的进度回调
/** Handles reporting progress from streamable handles */
struct FLyraAssetManagerStartupJob
{
	// 子进度汇报委托
	FLyraAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;
	// 真正干活的函数；返回的 Handle 由资产管理器持有
	TFunction<void(const FLyraAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)> JobFunc;
	// 作业名（打日志用）
	FString JobName;
	// 作业权重，用于把多个作业换算成总进度
	float JobWeight;
	// 上次汇报进度的时间，用于节流
	mutable double LastUpdate = 0;

	// 构造一个全同步执行的作业
	/** Simple job that is all synchronous */
	FLyraAssetManagerStartupJob(const FString& InJobName, const TFunction<void(const FLyraAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)>& InJobFunc, float InJobWeight)
		: JobFunc(InJobFunc)
		, JobName(InJobName)
		, JobWeight(InJobWeight)
	{}

	// 执行作业；如果创建了异步 Handle 就返回它
	/** Perform actual loading, will return a handle if it created one */
	TSharedPtr<FStreamableHandle> DoJob() const;

	// 直接汇报一个子进度值
	void UpdateSubstepProgress(float NewProgress) const
	{
		SubstepProgressDelegate.ExecuteIfBound(NewProgress);
	}

	// 从 FStreamableHandle 取进度并汇报；注意 GetProgress 遍历开销大，这里做了 60Hz 节流
	void UpdateSubstepProgressFromStreamable(TSharedRef<FStreamableHandle> StreamableHandle) const
	{
		if (SubstepProgressDelegate.IsBound())
		{
			// StreamableHandle::GetProgress traverses() a large graph and is quite expensive
			double Now = FPlatformTime::Seconds();
			if (LastUpdate - Now > 1.0 / 60)
			{
				SubstepProgressDelegate.Execute(StreamableHandle->GetProgress());
				LastUpdate = Now;
			}
		}
	}
};
