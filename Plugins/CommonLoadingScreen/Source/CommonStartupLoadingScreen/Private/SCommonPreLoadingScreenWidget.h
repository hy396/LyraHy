// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/GCObject.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"
#include "Widgets/SCompoundWidget.h"

class FReferenceCollector;

/**
 * SCommonPreLoadingScreenWidget —— 启动画面的 Slate 控件。
 *
 * 目前就是一个纯黑底。它的意义在于"占位"：项目可以换成自己的 Logo 动画，
 * 而不必改动整套 PreLoadScreen 机制。
 *
 * 它额外继承 FGCObject 是为了将来能持有 UObject 资源（如贴图）而不被 GC 回收。
 */
class SCommonPreLoadingScreenWidget : public SCompoundWidget, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SCommonPreLoadingScreenWidget) {}
    SLATE_END_ARGS()

	/** 构建控件：目前只放一个黑色填充框。 */
	void Construct(const FArguments& InArgs);

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End FGCObject interface

private:

};
