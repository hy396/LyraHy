// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFeatures/GameFeatureAction_AddInputConfig.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/GameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "Character/LyraHeroComponent.h"	// for NAME_BindInputsNow
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "GameFeatures/GameFeatureAction_WorldActionBase.h"
#include "PlayerMappableInputConfig.h"
#include "GameFramework/Pawn.h"
#include "Input/LyraMappableConfigPair.h"
#include "InputMappingContext.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameFeatureAction_AddInputConfig)

#define LOCTEXT_NAMESPACE "GameFeatures_AddInputConfig"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

namespace
{
	// UPlayerMappableInputConfig is no longer consumed by the Enhanced Input subsystems directly in
	// UE 5.6 (AddPlayerMappableConfig/RemovePlayerMappableConfig were removed). Applying the mapping
	// contexts it holds, using the priorities stored on the config, reproduces the old behaviour.
	void ApplyMappableConfigContexts(UEnhancedInputLocalPlayerSubsystem* Subsystem, const UPlayerMappableInputConfig* Config, const bool bAdd, const FModifyContextOptions& Options)
	{
		if (!Subsystem || !Config)
		{
			return;
		}

		for (const TPair<TObjectPtr<UInputMappingContext>, int32>& ContextPair : Config->GetMappingContexts())
		{
			if (!ContextPair.Key)
			{
				continue;
			}

			if (bAdd)
			{
				// Registering with the user settings is what makes the player mappable keys inside the
				// context show up (and stay rebindable) on the keyboard settings screen. This is what
				// AddPlayerMappableConfig used to do internally, and what
				// UGameFeatureAction_AddInputContextMapping does today.
				if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
				{
					Settings->RegisterInputMappingContext(ContextPair.Key);
				}

				Subsystem->AddMappingContext(ContextPair.Key, ContextPair.Value, Options);
			}
			else
			{
				// Note: intentionally not unregistering from the user settings here - the saved key
				// profile has to survive the game feature being deactivated.
				Subsystem->RemoveMappingContext(ContextPair.Key, Options);
			}
		}

		Subsystem->RequestRebuildControlMappings(Options);
	}
}

void UGameFeatureAction_AddInputConfig::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	// Register the input configs with the local settings, this way the data inside them is available all the time
	// and not just when this game feature is active. This is necessary for displaying key binding options
	// on the main menu, or other times when the game feature may not be active.
	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		FMappableConfigPair::RegisterPair(Pair);
	}
}

void UGameFeatureAction_AddInputConfig::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();

	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		FMappableConfigPair::UnregisterPair(Pair);
	}
}

void UGameFeatureAction_AddInputConfig::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveData.PawnsAddedTo.IsEmpty()))
	{
		Reset(ActiveData);
	}

	// Call super after the above logic so that we have our context before being added to the world
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputConfig::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FPerContextData* ActiveData = ContextData.Find(Context);

	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputConfig::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FMappableConfigPair& Pair : InputConfigs)
	{
		if (Pair.Config.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullConfigPointer", "Null Config pointer at index {0} in Pair list"), FText::AsNumber(EntryIndex)));
		}

		++EntryIndex;
	}
	
	return Result;
}
#endif	// WITH_EDITOR

void UGameFeatureAction_AddInputConfig::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);
	
	if (GameInstance && World && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddConfigDelegate =
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandlePawnExtension, ChangeContext);
			
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(APawn::StaticClass(), AddConfigDelegate);
			ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddInputConfig::Reset(FPerContextData& ActiveData)
{
	ActiveData.ExtensionRequestHandles.Empty();

	while (!ActiveData.PawnsAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APawn> PawnPtr = ActiveData.PawnsAddedTo.Top();
		if (PawnPtr.IsValid())
		{
			RemoveInputConfig(PawnPtr.Get(), ActiveData);
		}
		else
		{
			ActiveData.PawnsAddedTo.Pop();
		}
	}
}

void UGameFeatureAction_AddInputConfig::HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APawn* AsPawn = CastChecked<APawn>(Actor);
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == ULyraHeroComponent::NAME_BindInputsNow)
	{
		AddInputConfig(AsPawn, ActiveData);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveInputConfig(AsPawn, ActiveData);
	}
}

void UGameFeatureAction_AddInputConfig::AddInputConfig(APawn* Pawn, FPerContextData& ActiveData)
{
	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());

	if (ULocalPlayer* LP = PlayerController ? PlayerController->GetLocalPlayer() : nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			// We don't want to ignore keys that were "Down" when we add the mapping context
			// This allows you to die holding a movement key, keep holding while waiting for respawn,
			// and have it be applied after you respawn immediately. Leaving bIgnoreAllPressedKeysUntilRelease
			// to it's default "true" state would require the player to release the movement key,
			// and press it again when they respawn
			FModifyContextOptions Options = {};
			Options.bIgnoreAllPressedKeysUntilRelease = false;
			
			// Add the input mappings
			for (const FMappableConfigPair& Pair : InputConfigs)
			{
				if (Pair.bShouldActivateAutomatically && Pair.CanBeActivated())
				{
					ApplyMappableConfigContexts(Subsystem, Pair.Config.LoadSynchronous(), /*bAdd*/ true, Options);
				}
			}
			ActiveData.PawnsAddedTo.AddUnique(Pawn);
		}		
	}
}

void UGameFeatureAction_AddInputConfig::RemoveInputConfig(APawn* Pawn, FPerContextData& ActiveData)
{
	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());

	if (ULocalPlayer* LP = PlayerController ? PlayerController->GetLocalPlayer() : nullptr)
	{
		// If this is called during the shutdown of the game then there isn't a strict guarantee that the input subsystem is valid
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			// Remove the input mappings
			for (const FMappableConfigPair& Pair : InputConfigs)
			{
				ApplyMappableConfigContexts(Subsystem, Pair.Config.LoadSynchronous(), /*bAdd*/ false, FModifyContextOptions());
			}	
		}
	}
	ActiveData.PawnsAddedTo.Remove(Pawn);
}

#undef LOCTEXT_NAMESPACE

PRAGMA_ENABLE_DEPRECATION_WARNINGS