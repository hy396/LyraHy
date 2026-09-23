// Tencent is pleased to support the open source community by making UnLua available.
// 
// Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the MIT License (the "License"); 
// you may not use this file except in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, 
// software distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and limitations under the License.

#include "UnLuaEditorCore.h"
#include "UnLuaInterface.h"
#include "UnLuaPrivate.h"
#include "UnLuaSettings.h"
#include "Engine/Blueprint.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/IPluginManager.h"


FString GetLuaScriptFileName(const UBlueprint* Blueprint)
{
    if (!Blueprint)
    {
        return TEXT("");
    }

    const UClass* Class = Blueprint->GeneratedClass;
    const FString ClassName = Class->GetName();
    FString OuterPath = Class->GetPathName();

    // 获取包路径信息
    FString PackageRoot, PackagePath, PackageName;
    FPackageName::SplitLongPackageName(OuterPath, PackageRoot, PackagePath, PackageName);

    // 确定基础内容目录
    FString ContentDir;
    if (PackageRoot.StartsWith(TEXT("/Game")))
    {
        // 项目Content
        ContentDir = FPaths::ProjectContentDir();
    }
    else if (PackageRoot.StartsWith(TEXT("/")))
    {
        // 插件或模块Content
        FString PluginModuleName = PackageRoot.Mid(1); // 移除开头的'/'
        int32 SlashIndex;
        if (PluginModuleName.FindChar(TEXT('/'), SlashIndex))
        {
            PluginModuleName = PluginModuleName.Left(SlashIndex);
        }

        TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginModuleName);
        if (Plugin.IsValid())
        {
            ContentDir = Plugin->GetContentDir();
        }
        else
        {
            // 尝试查找模块
            FString ModulePath = FPaths::Combine(FPaths::ProjectPluginsDir(), PluginModuleName);
            if (!FPaths::DirectoryExists(ModulePath))
            {
                ModulePath = FPaths::Combine(FPaths::EnginePluginsDir(), PluginModuleName);
            }
            ContentDir = FPaths::Combine(ModulePath, TEXT("Content"));
        }
    }

    // 构建Script目录路径
    FString RelativePath = PackagePath; // 例如 "A/B/C"
    FString ScriptDir = FPaths::Combine(ContentDir, TEXT("Script"), RelativePath);

    const FString FileName = FPaths::ConvertRelativePathToFull(FPaths::Combine(ScriptDir, FString::Printf(TEXT("%s.lua"), *ClassName)));

    return FileName;
}

ELuaBindingStatus GetBindingStatus(const UBlueprint* Blueprint)
{
    if (!Blueprint)
        return ELuaBindingStatus::NotBound;

    if (Blueprint->Status == EBlueprintStatus::BS_Dirty)
        return ELuaBindingStatus::Unknown;

    const auto Target = Blueprint->GeneratedClass;

    if (!IsValid(Target))
        return ELuaBindingStatus::NotBound;

    if (!Target->ImplementsInterface(UUnLuaInterface::StaticClass()))
        return ELuaBindingStatus::NotBound;

    const auto Settings = GetDefault<UUnLuaSettings>();
    if (!Settings || !Settings->ModuleLocatorClass)
        return ELuaBindingStatus::Unknown;

    const auto ModuleLocator = Cast<ULuaModuleLocator>(Settings->ModuleLocatorClass->GetDefaultObject());
    const auto ModuleName = ModuleLocator->Locate(Target);
    if (ModuleName.IsEmpty())
        return ELuaBindingStatus::Unknown;
    
    const FString FileName = GetLuaScriptFileName(Blueprint);

    if (!FPaths::FileExists(FileName))
        return ELuaBindingStatus::BoundButInvalid;

    return ELuaBindingStatus::Bound;
}
