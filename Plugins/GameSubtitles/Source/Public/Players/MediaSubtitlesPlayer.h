// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Tickable.h"

#include "UObject/ObjectPtr.h"
#include "UObject/WeakObjectPtr.h"
#include "MediaSubtitlesPlayer.generated.h"

class UMediaPlayer;
class UOverlays;
struct FFrame;

/**
 * UMediaSubtitlesPlayer —— 媒体字幕播放器。
 *
 * 它需要与一个 UMediaPlayer 成对存在，并且它的 Play() / Stop() 要和媒体播放器的
 * Play() / Stop() 在同一时刻被调用，这样字幕才能和画面同步。
 *
 * 工作方式：每帧 Tick 时读取媒体播放器的当前播放时间，去 UOverlays 里查"此刻该显示哪些字幕"，
 * 再把结果交给引擎的 FSubtitleManager 显示出来。
 *
 * 同时实现 FTickableGameObject，因此它是靠 Tick 驱动的，而不是靠定时器。
 */
UCLASS(BlueprintType)
class GAMESUBTITLES_API UMediaSubtitlesPlayer
	: public UObject
	, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

public:

	/** 本播放器要用的字幕资源（UOverlays，即一条条带时间轴的文本叠加层）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitles Source")
	TObjectPtr<UOverlays> SourceSubtitles;

public:

	virtual void BeginDestroy() override;

	/** 开始播放字幕（只是把 bEnabled 置 true，实际推进靠 Tick）。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void Play();

	/** 停止播放字幕，并清空本对象在 FSubtitleManager 上注册的字幕。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void Stop();

	/** 更换字幕资源。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void SetSubtitles(UOverlays* Subtitles);

	/** 绑定到媒体播放器：字幕时间轴将以它的当前播放时间为准。 */
	UFUNCTION(BlueprintCallable, Category="Game Subtitles|Subtitles Player")
	void BindToMediaPlayer(UMediaPlayer* InMediaPlayer);

public:

	//~ FTickableGameObject interface
	virtual void Tick(float DeltaSeconds) override;
	virtual ETickableTickType GetTickableTickType() const override { return (HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Always); }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMediaSubtitlesPlayer, STATGROUP_Tickables); }

private:

	/** 绑定的媒体播放器。用弱指针是避免本对象把媒体播放器强引用住导致无法释放。 */
	TWeakObjectPtr<class UMediaPlayer> MediaPlayer;

	/** 当前是否处于播放状态。 */
	bool bEnabled;
};
