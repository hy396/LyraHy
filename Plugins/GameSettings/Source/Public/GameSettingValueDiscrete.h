// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameSettingValue.h"

#include "GameSettingValueDiscrete.generated.h"

class UObject;
struct FFrame;

/**
 * UGameSettingValueDiscrete —— 离散值设置项（从若干个固定选项里选一个）。
 *
 * 例如"画质：低/中/高""语言：中文/英文"。UI 上通常表现为下拉框或左右切换器。
 * 子类只需实现"按序号取值/设值"和"提供选项文字列表"。
 */
UCLASS(Abstract)
class GAMESETTINGS_API UGameSettingValueDiscrete : public UGameSettingValue
{
	GENERATED_BODY()

public:
	UGameSettingValueDiscrete();

	/** 按序号设置当前选中项（子类必须实现）。 */
	virtual void SetDiscreteOptionByIndex(int32 Index) PURE_VIRTUAL(,);
	
	UFUNCTION(BlueprintCallable)
	virtual int32 GetDiscreteOptionIndex() const PURE_VIRTUAL(,return INDEX_NONE;);

	/** 默认项的序号（可选实现）。不实现则返回 INDEX_NONE，表示"无默认值、不可重置"。 */
	UFUNCTION(BlueprintCallable)
	virtual int32 GetDiscreteOptionDefaultIndex() const { return INDEX_NONE; }

	/** 所有可选项的显示文字（子类必须实现），下标与上面的序号一一对应。 */
	UFUNCTION(BlueprintCallable)
	virtual TArray<FText> GetDiscreteOptions() const PURE_VIRTUAL(,return TArray<FText>(););

	virtual FString GetAnalyticsValue() const;
};
