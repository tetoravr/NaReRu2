// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#include "PathFollowPrivatePCH.h"
#if WITH_EDITORONLY_DATA
#include "Components/PFPathFollowerComponent.h"
#include "Classes/PFPathFollowerVisualizer.h"
#include "Classes/PFPathFollowerCustomization.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "LevelEditor.h"
#endif

IMPLEMENT_MODULE(FPathFollowModule, PathFollow)

DEFINE_LOG_CATEGORY(PathFollowModuleLog)

#define LOCTEXT_NAMESPACE "FPathFollowModule"

#if WITH_EDITORONLY_DATA
class FPFPathFollowCommands : public TCommands<FPFPathFollowCommands>
{
public:
  FPFPathFollowCommands()
    : TCommands<FPFPathFollowCommands>(TEXT("PathFollow"), NSLOCTEXT("Contexts", "PathFollow", "PathFollow Plugin"), NAME_None, FEditorStyle::GetStyleSetName())
  {
  }

  // TCommands<> interface
  virtual void RegisterCommands() override
  {
    UI_COMMAND(LockActorSelection, "Lock Selection", "Lock currently selected actor selection i.e. actor is stays always selected. Press again to release.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt, EKeys::E));
		UI_COMMAND(SelectPathFollowerComponent, "Select PathFollower Component if in list.", "Tryies to find the follower component. If it does it selects the component.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Alt, EKeys::S));
  }

public:
  TSharedPtr< FUICommandInfo > LockActorSelection;
	TSharedPtr< FUICommandInfo > SelectPathFollowerComponent;

	mutable TWeakObjectPtr<class AActor> _LockedActor;
};

void FPathFollowModule::RegisterVisualizer()
{
	check(GUnrealEd);

	TSharedPtr<FComponentVisualizer> Vizualizer = MakeShareable(new FPFPathFollowerVisulalizer);
	if (!Vizualizer.IsValid())
	{
		UE_LOG(PathFollowModuleLog, Warning, TEXT("!Vizualizer.IsValid()"));
		return;
	}

	GUnrealEd->RegisterComponentVisualizer(UPFPathFollowerComponent::StaticClass()->GetFName(), Vizualizer);
	Vizualizer->OnRegister();
}

void FPathFollowModule::RegisterDetailsCustomization()
{
	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	PropertyModule.RegisterCustomClassLayout(UPFPathFollowerComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FPFPathFollowerCustomization::MakeInstance));
}


void FPathFollowModule::OnSelectObject(UObject* Selected)
{
	auto& LockedActor = FPFPathFollowCommands::Get()._LockedActor;
	if (LockedActor.IsValid())
	{
		USelection* Selection = Cast<USelection>(Selected);
		if (Selection)
		{
			TArray<AActor*> SelectedActors;
			Selection->GetSelectedObjects(SelectedActors);
			bool ContainsLockedActor = SelectedActors.Contains(LockedActor.Get());
			if (ContainsLockedActor && SelectedActors.Num() == 1)
			{
				return;
			}
			else
			{
				if (GEditor)
				{
					GEditor->GetSelectedActors()->DeselectAll();
					GEditor->SelectActor(LockedActor.Get(), /*InSelected*/true, /*Notify*/true);
				}
			}
		}
	}
}


void FPathFollowModule::SetFollowerActor(class AActor* LockedActor)
{
	FPFPathFollowCommands::Get()._LockedActor = LockedActor;
}

#endif

void FPathFollowModule::StartupModule()
{
	#if WITH_EDITORONLY_DATA

	if (GUnrealEd == nullptr)
	{
		return;
	}

	FPFPathFollowCommands::Register();
  
	auto& Commands = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor").GetGlobalLevelEditorActions();

	Commands->MapAction(
    FPFPathFollowCommands::Get().LockActorSelection, 
    FExecuteAction::CreateLambda([this]
    { 
			auto& LockedActor = FPFPathFollowCommands::Get()._LockedActor;
      if (LockedActor.IsValid())
      {
				LockedActor = nullptr;
      }
      else
      {
				if (GEditor)
				{
					USelection* ActorsSelection = GEditor->GetSelectedActors();
					if (ActorsSelection && ActorsSelection->Num() == 1)
					{
						 auto* SelectedActor = CastChecked<AActor>(ActorsSelection->GetSelectedObject(0));
						 LockedActor = SelectedActor;
					}
				}
      }
    }),
      FCanExecuteAction::CreateLambda([]{ return true; }));


	Commands->MapAction(
		FPFPathFollowCommands::Get().SelectPathFollowerComponent,
		FExecuteAction::CreateLambda([this]
	{
		if (GEditor)
		{
			if (auto* SelectedActor = GEditor->GetSelectedActors()->GetTop<AActor>())
			{
				if (auto* FollowerComp = SelectedActor->GetComponentByClass(UPFPathFollowerComponent::StaticClass()))
				{
					GEditor->SelectComponent(FollowerComp, true, true, true);
				}
			}
		}
	}),
		FCanExecuteAction::CreateLambda([] { return true; }));


  
    
  USelection::SelectionChangedEvent.AddRaw(this, &FPathFollowModule::OnSelectObject);
	
  RegisterVisualizer();
  
  RegisterDetailsCustomization();

#endif
}



void FPathFollowModule::ShutdownModule()
{
  UE_LOG(PathFollowModuleLog, Warning, TEXT("ShutdownModule()"));

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_EDITORONLY_DATA
  if (GUnrealEd != nullptr)
  {
    GUnrealEd->UnregisterComponentVisualizer(UPFPathFollowerComponent::StaticClass()->GetFName());
  }
#endif
}



#undef LOCTEXT_NAMESPACE
	
