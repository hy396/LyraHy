// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "Fonts/SlateFontInfo.h"

#include "CommonPlayerInputKey.generated.h"

enum class ECommonInputType : uint8;

class APlayerController;
class FPaintArgs;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
class UCommonLocalPlayer;
class UMaterialInstanceDynamic;
class UObject;
struct FFrame;
struct FGeometry;

/** 键位提示控件是否"强制显示为长按"。用于覆盖按键本身的默认表现。 */
UENUM(BlueprintType)
enum class ECommonKeybindForcedHoldStatus : uint8
{
	NoForcedHold,
	ForcedHold,
	NeverShowHold
};

/**
 * FMeasuredText —— 带尺寸缓存的文本。
 *
 * 测量文本尺寸（Slate 的 ComputeDesiredSize）并不便宜，而键位提示控件每帧都要用，
 * 所以这里把"文字 + 量出来的尺寸"缓存起来，只在文字变脏时重测。
 */
USTRUCT()
struct FMeasuredText
{
	GENERATED_BODY()

public:
	FText GetText() const { return CachedText; }
	void SetText(const FText& InText);

	FVector2D GetTextSize() const { return CachedTextSize; }
	FVector2D UpdateTextSize(const FSlateFontInfo &InFontInfo, float FontScale = 1.0f) const;

private:

	FText CachedText;
	mutable FVector2D CachedTextSize;
	mutable bool bTextDirty = true;
};

/**
 * UCommonPlayerInputKey —— 键位提示控件（显示"按 [E] 开门"里的那个 [E] 图标）。
 *
 * 它能自动跟随玩家当前的输入设备（键鼠/手柄/触屏）显示对应的按键图标，
 * 并支持"长按"键位显示进度圈。
 *
 * 两种用法：
 *   - SetBoundAction(FName)  —— 绑定输入动作名，图标随玩家实际键位设置自动变化（推荐）
 *   - SetBoundKey(FKey)      —— 直接指定某个键，忽略玩家的自定义键位
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class COMMONGAME_API UCommonPlayerInputKey : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UCommonPlayerInputKey(const FObjectInitializer& ObjectInitializer);

	/** 按当前绑定的动作重新查一次实际按键，并刷新显示。 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void UpdateKeybindWidget();

	/** Set the bound key for our keybind */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetBoundKey(FKey NewBoundAction);

	/** Set the bound action for our keybind */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetBoundAction(FName NewBoundAction);

	/** 【已废弃】强制显示为长按。请改用 SetForcedHoldKeybindStatus。 */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget", meta=(DeprecatedFunction, DeprecationMessage = "Use SetForcedHoldKeybindStatus instead"))
	void SetForcedHoldKeybind(bool InForcedHoldKeybind);

	/** Force this keybind to be a hold keybind */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetForcedHoldKeybindStatus(ECommonKeybindForcedHoldStatus InForcedHoldKeybindStatus);

	/** Force this keybind to be a hold keybind */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetShowProgressCountDown(bool bShow);

	/** Set the axis scale value for this keybind */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetAxisScale(const float NewValue) { AxisScale = NewValue; }

	/** Set the preset name override value for this keybind. */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	void SetPresetNameOverride(const FName NewValue) { PresetNameOverride = NewValue; }

	/** 当前绑定的输入动作名（用它才能跟随玩家的自定义键位）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FName BoundAction;

	/** 轴映射的比例值：负值表示这是个"反向"的轴（如 A 键 = -1）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	float AxisScale;

	/** 在蓝图里直接指定的按键。用于"就想显示某个固定键、不跟随玩家键位"的场景。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKeyFallback;

	/** Allows us to set the input type explicitly for the keybind widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	ECommonInputType InputTypeOverride;

	/** Allows us to set the preset name explicitly for the keybind widget. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind Widget")
	FName PresetNameOverride;

	/** 强制显示为长按 / 强制不显示为长按（即使它实际就是长按）。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keybind Widget")
	ECommonKeybindForcedHoldStatus ForcedHoldKeybindStatus;

	/** 由委托回调：开始长按进度显示。 */
	UFUNCTION()
	void StartHoldProgress(FName HoldActionName, float HoldDuration);

	/** 由委托回调：结束长按进度显示（参数表示是否按满了）。 */
	UFUNCTION()
	void StopHoldProgress(FName HoldActionName, bool bCompletedSuccessfully);

	/** Get whether this keybind is a hold action. */
	UFUNCTION(BlueprintCallable, Category = "Keybind Widget")
	bool IsHoldKeybind() const { return bIsHoldKeybind; }

	UFUNCTION()
	bool IsBoundKeyValid() const { return BoundKey.IsValid(); }

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	void RecalculateDesiredSize();

	/** Overridden to destroy our MID */
	virtual void NativeDestruct() override;

	/** 本控件当前是否被当作"长按"键位来显示。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Keybind Widget", meta=(ScriptName = "IsHoldKeybindValue"))
	bool bIsHoldKeybind;

	/**  */
	UPROPERTY(Transient)
	bool bShowKeybindBorder;

	UPROPERTY(Transient)
	FVector2D FrameSize;

	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	bool bShowTimeCountDown;

	/** Derived Key this widget is bound to */
	UPROPERTY(BlueprintReadOnly, Category = "Keybind Widget")
	FKey BoundKey;

	/** 长按进度圈的画刷。 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush HoldProgressBrush;

	/** The key bind text border. */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FSlateBrush KeyBindTextBorder;

	/** 若该动作当前没有绑定任何键，是否显示"未绑定"提示。 */
	UPROPERTY(EditAnywhere, Category = "Keybind Widget")
	bool bShowUnboundStatus = false;

	/** The font to apply at each size */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo KeyBindTextFont;

	/** The font to apply at each size */
	UPROPERTY(EditDefaultsOnly, Category = "Font")
	FSlateFontInfo CountdownTextFont;

	UPROPERTY(Transient)
	FMeasuredText CountdownText;

	UPROPERTY(Transient)
	FMeasuredText KeybindText;

	UPROPERTY(Transient)
	FMargin KeybindTextPadding;

	UPROPERTY(Transient)
	FVector2D KeybindFrameMinimumSize;

	/** 进度材质里"百分比"参数的名字。 */
	UPROPERTY(EditDefaultsOnly, Category = "Keybind Widget")
	FName PercentageMaterialParameterName;	

	/** 进度百分比用的动态材质实例（运行时动态改参数）。 */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressPercentageMID;

	virtual void NativeOnInitialized() override;

private:
	/**
	 * Synchronizes the hold progress to whatever is currently set in the
	 * owning player controller.
	 */
	void SyncHoldProgress();

	/** Called for updating the HoldKeybindImage during a hold keybind */
	void UpdateHoldProgress();

	/** Called when we want to set up this keybind widget as a hold keybind */
	void SetupHoldKeybind();

	void ShowHoldBackPlate();

	void HandlePlayerControllerSet(UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController);

	/** Time when we started using a hold keybind */
	float HoldKeybindStartTime = 0;

	/** How long, in seconds, we will be doing a hold keybind */
	float HoldKeybindDuration = 0;

	bool bDrawProgress = false;
	bool bDrawBrushForKey = false;
	bool bDrawCountdownText = false;
	bool bWaitingForPlayerController = false;

	UPROPERTY(Transient)
	FSlateBrush CachedKeyBrush;
};
