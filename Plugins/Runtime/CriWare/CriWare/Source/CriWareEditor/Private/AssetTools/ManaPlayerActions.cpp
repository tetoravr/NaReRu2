

#include "AssetTools/ManaPlayerActions.h"
#include "ManaPlayer.h"
#include "Misc/PackageName.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "EditorStyleSet.h"
#include "ManaTexture.h"
#include "Toolkits/ManaPlayerEditorToolkit.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions"


/* FManaPlayerActions constructors
 *****************************************************************************/

FManaPlayerActions::FManaPlayerActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{ }


/* FAssetTypeActions_Base interface
 *****************************************************************************/

bool FManaPlayerActions::CanFilter()
{
	return true;
}


uint32 FManaPlayerActions::GetCategories()
{
	return AssetCategory;
}


FText FManaPlayerActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ManaPlayer", "Mana Player");
}


UClass* FManaPlayerActions::GetSupportedClass() const
{
	return UManaPlayer::StaticClass();
}


FColor FManaPlayerActions::GetTypeColor() const
{
	return FColor::Red;
}


void FManaPlayerActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid()
		? EToolkitMode::WorldCentric
		: EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto ManaPlayer = Cast<UManaPlayer>(*ObjIt);

		if (ManaPlayer != nullptr)
		{
			TSharedRef<FManaPlayerEditorToolkit> EditorToolkit = MakeShareable(new FManaPlayerEditorToolkit());
			EditorToolkit->Initialize(ManaPlayer, Mode, EditWithinLevelEditor);
		}
	}
}


#undef LOCTEXT_NAMESPACE
