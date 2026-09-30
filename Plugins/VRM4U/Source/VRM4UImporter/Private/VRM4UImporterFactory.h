// VRM4U Copyright (c) 2021-2026 Haruyoshi Yamamoto. This software is released under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "Factories/Factory.h"
#include "VRM4UImporterFactory.generated.h"

// 前向声明需放在 UCLASS() 之前，否则会被 UHT 误认为反射类的声明
struct FImportOptionData;

UCLASS()
class UVRM4UImporterFactory : public UFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

public:
	// UE5.8 起 UFactory 移除了 bUseProvidedImportOptions / ProvidedImportOptions，
	// 改用自定义字段承接外部传入的导入选项
	const FImportOptionData* ExternalOptionData = nullptr;
	bool bUseExternalOptionData = false;

	// UFactory interface
	//virtual FText GetToolTip() const override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual UClass* ResolveSupportedClass() override;
	virtual UObject* FactoryCreateText(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn) override;
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	// End of UFactory interface

		// FReimportHandler interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	// End of FReimportHandler interface

protected:

	FString fullFileName;
	class UVrmAssetListObject* ReimportBase = nullptr;
};
