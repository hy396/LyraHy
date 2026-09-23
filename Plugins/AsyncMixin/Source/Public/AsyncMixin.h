// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/Ticker.h"
#include "UObject/SoftObjectPtr.h"

class FAsyncCondition;
class FName;
class UPrimaryDataAsset;
struct FPrimaryAssetId;
struct FStreamableHandle;
template <class TClass> class TSubclassOf;

// 回调：一个异步加载句柄已就绪。
DECLARE_DELEGATE_OneParam(FStreamableHandleDelegate, TSharedPtr<FStreamableHandle>)

//TODO（Epic 原版待办，保留未删）考虑引入"资源保留策略"：目前预加载会一直留在内存里直到被取消，
//     但如果只是想用 AsyncLoad 单独预载几项呢？原作者不想为每次调用都加一个策略参数，
//     而是希望整体引入一套保留策略（做成成员还是模板参数尚未决定）。
//     but what if you want to preload individual items just using the AsyncLoad functions?  I don't want to
//     introduce individual policies per call, or introduce a whole set of preload vs asyncloads, so would
//     would rather have a retention policy.  Should it be a member and actually create real memory when
//     you inherit from AsyncMixin, or should it be a template argument?
//enum class EAsyncMixinRetentionPolicy : uint8
//{
//	Default,
//	KeepResidentUntilComplete,
//	KeepResidentUntilCancel
//};

/**
 * FAsyncMixin —— 异步加载"混入类"：把一串异步加载请求串成【有顺序】的流水线。
 *
 * 解决的核心痛点：你要加载 A、B 两个资源，各自完成后做点事，且必须按 A→B 的顺序执行。
 * 手写的话，回调会嵌套成一坨，而且谁先加载完是不确定的。本类让你可以写成平铺的：
 *
 *     CancelAsyncLoading();                       // 先取消上一次还没跑完的（列表项复用时很关键）
 *     AsyncLoad(SoftClassA, [this]() { ... });    // 第一步
 *     AsyncLoad(SoftObjectB, [this](UObject* O) { ... });  // 第二步
 *     StartAsyncLoading();                        // 发车
 *
 * 几个关键保证：
 *   1) 【按请求顺序调用回调】——即使 B 比 A 先加载完，也会先等 A 的回调跑完再跑 B 的。
 *   2) 【宿主销毁即自动解绑】——可以放心在 lambda 里捕获 [this]，宿主没了回调就不会再触发。
 *   3) 【零额外内存】——混入类本身不占空间，加载状态存在一个静态 TMap 里，用时才分配、用完就扔。
 *   4) 全部完成后会调用一次 OnFinishedLoading()。
 *
 * 忘了调 StartAsyncLoading() 也没关系——它会在下一帧自动调用。但最好还是显式调，
 * 因为资源可能已经全部加载好了，显式调用可以省掉"加载指示器闪一帧"的尴尬。
 *
 * 调试时在命令行加 -LogCmds="LogAsyncMixin Verbose" 可以看到每一步在干什么。
 */
class ASYNCMIXIN_API FAsyncMixin : public FNoncopyable
{
protected:
	FAsyncMixin();

public:
	virtual ~FAsyncMixin();

protected:
	/** 加载开始时调用（子类可重写，比如显示一个转圈图标）。 */
	virtual void OnStartedLoading() { }
	/** 所有异步步骤都完成后调用（子类可重写，比如隐藏加载指示）。 */
	virtual void OnFinishedLoading() { }

protected:
	/** 异步加载一个软引用【类】，完成后回调（无参数版本）。 */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载一个软引用【类】，完成后把加载好的类作为回参交给你。 */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, TFunction<void(TSubclassOf<T>)>&& Callback)
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(),
			FSimpleDelegate::CreateLambda([SoftClass, UserCallback = MoveTemp(Callback)]() mutable {
				UserCallback(SoftClass.Get());
			})
		);
	}

	/** Async load a TSoftClassPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftClassPtr<T> SoftClass, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftClass.ToSoftObjectPath(), Callback);
	}

	/** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 异步加载一个软引用【对象】，完成后把加载好的对象指针交给你。 */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, TFunction<void(T*)>&& Callback)
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(),
			FSimpleDelegate::CreateLambda([SoftObject, UserCallback = MoveTemp(Callback)]() mutable {
				UserCallback(SoftObject.Get());
			})
		);
	}

	/** Async load a TSoftObjectPtr<T>, call the Callback when complete. */
	template<typename T = UObject>
	void AsyncLoad(TSoftObjectPtr<T> SoftObject, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		AsyncLoad(SoftObject.ToSoftObjectPath(), Callback);
	}

	/** 异步加载一个 FSoftObjectPath（最底层版本，上面所有重载最终都转到这里）。 */
	void AsyncLoad(FSoftObjectPath SoftObjectPath, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** Async load an array of FSoftObjectPath, call the Callback when complete. */
	void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, TFunction<void()>&& Callback)
	{
		AsyncLoad(SoftObjectPaths, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** 批量异步加载：整批作为一个步骤，全部就绪后才算这一步完成。 */
	void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** 预加载一批主资产（PrimaryAsset）里由属性引用到的 Bundle。
	 *  【注意】Bundle 预加载要求保留住流式句柄，因此用过这个函数的 FAsyncMixin
	 *  在全部完成后【不会】立刻销毁内部状态（见 FLoadingState::bPreloadedBundles）。 */
	template<typename T = UPrimaryDataAsset>
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<T*>& Assets, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate())
	{
		TArray<FPrimaryAssetId> PrimaryAssetIds;
		for (const T* Item : Assets)
		{
			PrimaryAssetIds.Add(Item);
		}

		AsyncPreloadPrimaryAssetsAndBundles(PrimaryAssetIds, LoadBundles, Callback);
	}

	/** Given an array of primary asset ids, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, TFunction<void()>&& Callback)
	{
		AsyncPreloadPrimaryAssetsAndBundles(AssetIds, LoadBundles, FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/** Given an array of primary asset ids, it loads all of the bundles referenced by properties of these assets specified in the LoadBundles array. */
	void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& AssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& Callback = FSimpleDelegate());

	/** 插入一个"自定义条件"步骤：条件满足前流水线会停在这里，每帧重试。
	 *  用于"等某个外部状态就绪"这类无法用资源加载表达的依赖。 */
	void AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback = FSimpleDelegate());

	/**
	 * Rather than load anything, this callback is just inserted into the callback sequence so that when async loading 
	 * completes this event will be called at the same point in the sequence.  Super useful if you don't want a step to be
	 * tied to a particular asset in case some of the assets are optional.
	 */
	void AsyncEvent(TFunction<void()>&& Callback)
	{
		AsyncEvent(FSimpleDelegate::CreateLambda(MoveTemp(Callback)));
	}

	/**
	 * Rather than load anything, this callback is just inserted into the callback sequence so that when async loading
	 * completes this event will be called at the same point in the sequence.  Super useful if you don't want a step to be
	 * tied to a particular asset in case some of the assets are optional.
	 */
	void AsyncEvent(const FSimpleDelegate& Callback);

	/** 正式启动整条流水线。建议在所有 AsyncLoad 添加完后显式调用一次。 */
	void StartAsyncLoading();

	/** 取消所有未完成的异步步骤，并把内部状态标记为待销毁。
	 *  复用型对象（如列表条目控件）在开始新的一批加载前必须先调它。 */
	void CancelAsyncLoading();

	/** 当前是否有异步步骤还在进行中。 */
	bool IsAsyncLoadingInProgress() const;

private:
	/**
	 * FLoadingState —— 真正干活的那份状态。
	 *
	 * 它【不】作为 FAsyncMixin 的成员存在，而是放在一个全局静态 TMap 里，
	 * 用时才分配、用完就销毁——这正是"混入类不增加宿主对象体积"的实现方式。
	 */
	class FLoadingState : public TSharedFromThis<FLoadingState>
	{
	public:
		FLoadingState(FAsyncMixin& InOwner);
		virtual ~FLoadingState();

		/** 启动整条流水线：按顺序把各步骤发出去。 */
		void Start();

		/** 取消整条流水线，并安排销毁这份状态。 */
		void CancelAndDestroy();

		void AsyncLoad(FSoftObjectPath SoftObject, const FSimpleDelegate& DelegateToCall);
		void AsyncLoad(const TArray<FSoftObjectPath>& SoftObjectPaths, const FSimpleDelegate& DelegateToCall);
		void AsyncPreloadPrimaryAssetsAndBundles(const TArray<FPrimaryAssetId>& PrimaryAssetIds, const TArray<FName>& LoadBundles, const FSimpleDelegate& DelegateToCall);
		void AsyncCondition(TSharedRef<FAsyncCondition> Condition, const FSimpleDelegate& Callback);
		void AsyncEvent(const FSimpleDelegate& Callback);

		bool IsLoadingComplete() const { return !IsLoadingInProgress(); }
		bool IsLoadingInProgress() const;
		bool IsLoadingInProgressOrPending() const;
		bool IsPendingDestroy() const;

	private:
		void CancelOnly(bool bDestroying);
		void CancelStartTimer();
		void TryScheduleStart();
		void TryCompleteAsyncLoading();
		void CompleteAsyncLoading();

	private:
		void RequestDestroyThisMemory();
		void CancelDestroyThisMemory(bool bDestroying);

		/** 宿主混入对象的引用（需要回调它的 OnStartedLoading / OnFinishedLoading）。 */
		FAsyncMixin& OwnerRef;

		/** 是否用过 Bundle 预加载。用过的话流式句柄必须一直留着（否则资源会被卸载），
		 *  因此全部完成后也不能立刻销毁这份状态。 */
		bool bPreloadedBundles = false;

		class FAsyncStep
		{
		public:
			FAsyncStep(const FSimpleDelegate& InUserCallback);
			FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FStreamableHandle>& InStreamingHandle);
			FAsyncStep(const FSimpleDelegate& InUserCallback, const TSharedPtr<FAsyncCondition>& InCondition);

			~FAsyncStep();

			void ExecuteUserCallback();

			bool IsLoadingInProgress() const
			{
				return !IsComplete();
			}

			bool IsComplete() const;
			void Cancel();

			bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);
			bool IsCompleteDelegateBound() const;

		private:
			FSimpleDelegate UserCallback;
			bool bIsCompletionDelegateBound = false;

			// Possible Async 'thing'
			TSharedPtr<FStreamableHandle> StreamingHandle;
			TSharedPtr<FAsyncCondition> Condition;
		};

		/** 流水线是否已启动（未启动时只是把步骤攒起来）。 */
		bool bHasStarted = false;

		/** 当前执行到第几步。 */
		int32 CurrentAsyncStep = 0;
		/** 流水线上的所有步骤，按顺序执行。 */
		TArray<TUniquePtr<FAsyncStep>> AsyncSteps;
		/** 已取消、等待安全时机销毁的步骤（防止在回调执行过程中把自己删掉）。 */
		TArray<TUniquePtr<FAsyncStep>> AsyncStepsPendingDestruction;

		/** "下一帧自动 Start"的定时器句柄（用户忘记调 StartAsyncLoading 时兜底）。 */
		FTSTicker::FDelegateHandle StartTimerDelegate;
		/** "下一帧销毁这份状态"的定时器句柄。 */
		FTSTicker::FDelegateHandle DestroyMemoryDelegate;
	};

	const FLoadingState& GetLoadingStateConst() const;
	
	FLoadingState& GetLoadingState();

	bool HasLoadingState() const;

	bool IsLoadingInProgressOrPending() const;

private:
	/** 全局状态表：混入对象 → 它的加载状态。用静态表是为了让混入类本身零体积。 */
	static TMap<FAsyncMixin*, TSharedRef<FLoadingState>> Loading;
};

/**
 * FAsyncScope —— 独立可用的"异步作用域"。
 *
 * 当你的类需要同时管理【多组】互不相干的异步加载链时，继承 FAsyncMixin 就不合适了
 * （一个类只能有一条流水线）。这时就给每组任务各建一个 FAsyncScope 成员——
 * 它把 FAsyncMixin 的受保护接口全部公开出来，可以当成一个独立的小句柄用。
 */
class ASYNCMIXIN_API FAsyncScope : public FAsyncMixin
{
public:
	using FAsyncMixin::AsyncLoad;

	using FAsyncMixin::AsyncPreloadPrimaryAssetsAndBundles;

	using FAsyncMixin::AsyncCondition;

	using FAsyncMixin::AsyncEvent;

	using FAsyncMixin::CancelAsyncLoading;

	using FAsyncMixin::StartAsyncLoading;

	using FAsyncMixin::IsAsyncLoadingInProgress;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

/** 自定义条件每帧被询问时的返回值。 */
enum class EAsyncConditionResult : uint8
{
	/** 条件还不满足，下一帧再来问一次。 */
	TryAgain,

	/** 条件满足了，流水线可以继续往下走。 */
	Complete
};

DECLARE_DELEGATE_RetVal(EAsyncConditionResult, FAsyncConditionDelegate);

/**
 * FAsyncCondition —— 自定义异步条件：让流水线停下来等"某个条件成立"。
 *
 * 它每帧被询问一次，返回 TryAgain 表示继续等，返回 Complete 表示可以往下走。
 * 用于表达"等玩家登录完成""等某个子系统初始化好"这类无法用资源加载描述的依赖。
 */
class FAsyncCondition : public TSharedFromThis<FAsyncCondition>
{
public:
	FAsyncCondition(const FAsyncConditionDelegate& Condition);
	FAsyncCondition(TFunction<EAsyncConditionResult()>&& Condition);
	virtual ~FAsyncCondition();

protected:
	bool IsComplete() const;
	bool BindCompleteDelegate(const FSimpleDelegate& NewDelegate);

private:
	bool TryToContinue(float DeltaTime);

	/** 每帧询问条件的定时器句柄。 */
	FTSTicker::FDelegateHandle RepeatHandle;
	/** 用户提供的条件判断函数。 */
	FAsyncConditionDelegate UserCondition;
	/** 条件满足时要通知的对象（由 FAsyncMixin 绑定）。 */
	FSimpleDelegate CompletionDelegate;

	friend FAsyncMixin;
};
