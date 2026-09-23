// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/DynamicEntryBoxBase.h"
#include "UIExtensionSystem.h"

#include "UIExtensionPointWidget.generated.h"

class IWidgetCompilerLog;

class UCommonLocalPlayer;
class APlayerState;

/**
 * UUIExtensionPointWidget —— UI 上的"扩展点控件"（消费端）。
 *
 * 在 UI 布局里放一个它，就相当于声明"这里以后可以被别人塞东西进来"。
 * 它派生自 UDynamicEntryBoxBase，因此具备动态创建/移除子控件的能力。
 *
 * 一个本控件在运行时最多会注册【3 个】扩展点：
 *   1. 无上下文        —— 接收不针对特定玩家的扩展
 *   2. LocalPlayer 上下文 —— 接收针对本玩家的扩展
 *   3. PlayerState 上下文 —— 玩家状态就绪后再补注册一个
 * 所以 ExtensionPointHandles 是数组而不是单个句柄。
 */
UCLASS()
class UIEXTENSION_API UUIExtensionPointWidget : public UDynamicEntryBoxBase
{
	GENERATED_BODY()

public:

	/** 当扩展贡献的是【数据】而非控件类时，用它把数据转换成"该用哪个控件类来显示"。
	 *  返回空表示这份数据与本控件无关，会被忽略。 */
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(TSubclassOf<UUserWidget>, FOnGetWidgetClassForData, UObject*, DataItem);

	/** 控件创建出来之后，用它对新控件做初始化（例如把数据绑上去）。 */
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnConfigureWidgetForData, UUserWidget*, Widget, UObject*, DataItem);

	UUIExtensionPointWidget(const FObjectInitializer& ObjectInitializer);

	//~UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif
	//~End of UWidget interface

private:
	/** 反注册全部扩展点并清空已有的控件映射（重建前先清干净）。 */
	void ResetExtensionPoint();

	/** 注册"无上下文"和"LocalPlayer 上下文"两个扩展点。 */
	void RegisterExtensionPoint();

	/** 玩家状态就绪后再补注册一个"PlayerState 上下文"的扩展点。 */
	void RegisterExtensionPointForPlayerState(UCommonLocalPlayer* LocalPlayer, APlayerState* PlayerState);

	/** 扩展点回调：Added 时创建控件，Removed 时移除控件。 */
	void OnAddOrRemoveExtension(EUIExtensionAction Action, const FUIExtensionRequest& Request);

protected:
	/** 本扩展点的标签。贡献方必须把扩展注册到同一个（或子）标签上才会被这里收到。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	FGameplayTag ExtensionPointTag;

	/** 标签匹配规则：精确匹配，还是连子标签一起收。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	EUIExtensionPointMatch ExtensionPointTagMatch = EUIExtensionPointMatch::ExactMatch;

	/** 额外允许的数据类型。注意 UUserWidget::StaticClass() 会被自动加入，无需手动填。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<TObjectPtr<UClass>> DataClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnGetWidgetClassForData GetWidgetClassForData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI Extension", meta=( IsBindableEvent="True" ))
	FOnConfigureWidgetForData ConfigureWidgetForData;

	/** 本控件注册出来的所有扩展点句柄（通常有 2~3 个）。 */
	TArray<FUIExtensionPointHandle> ExtensionPointHandles;

	/** 已创建的控件：扩展句柄 → 对应的控件实例，移除时靠它找到要删的那个。 */
	UPROPERTY(Transient)
	TMap<FUIExtensionHandle, TObjectPtr<UUserWidget>> ExtensionMapping;
};
