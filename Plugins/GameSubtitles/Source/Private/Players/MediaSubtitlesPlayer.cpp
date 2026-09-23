// Copyright Epic Games, Inc. All Rights Reserved.

#include "Players/MediaSubtitlesPlayer.h"

#include "MediaPlayer.h"
#include "Overlays.h"
#include "Stats/Stats.h"
#include "SubtitleManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MediaSubtitlesPlayer)

UMediaSubtitlesPlayer::UMediaSubtitlesPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MediaPlayer(nullptr)
	, bEnabled(false)
{
}

// 销毁前先把自己注册到字幕管理器上的字幕清掉，避免残留文字一直挂在屏幕上。
void UMediaSubtitlesPlayer::BeginDestroy()
{
	Stop();

	Super::BeginDestroy();
}

void UMediaSubtitlesPlayer::Play()
{
	bEnabled = true;
}

void UMediaSubtitlesPlayer::Stop()
{
	bEnabled = false;

	// Clear the movie subtitle for this object
	FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, TArray<FString>());
}

void UMediaSubtitlesPlayer::SetSubtitles(UOverlays* Subtitles)
{
	SourceSubtitles = Subtitles;
}

void UMediaSubtitlesPlayer::BindToMediaPlayer(UMediaPlayer* InMediaPlayer)
{
	MediaPlayer = InMediaPlayer;
}

// 核心：每帧同步一次字幕。
//   1) 取媒体播放器当前时间；
//   2) 去字幕资源里查这个时间点该显示哪些条目；
//   3) 把文本交给引擎的 FSubtitleManager，由它广播给实际的显示控件。
// 若绑定的媒体播放器已经失效，则自动停止，防止空转。
void UMediaSubtitlesPlayer::Tick(float DeltaSeconds)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_UMediaSubtitlesPlayer_Tick);

	if (bEnabled && SourceSubtitles)
	{
		UMediaPlayer* MediaPlayerPtr = MediaPlayer.Get();
		if (MediaPlayerPtr)
		{
			FTimespan CurrentTime = MediaPlayerPtr->GetTime();
			TArray<FOverlayItem> CurrentSubtitles;
			SourceSubtitles->GetOverlaysForTime(CurrentTime, CurrentSubtitles);

			TArray<FString> SubtitlesText;
			for (const FOverlayItem& Subtitle : CurrentSubtitles)
			{
				SubtitlesText.Add(Subtitle.Text);
			}

			FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, SubtitlesText);
		}
		else
		{
			Stop();
		}
	}
}

