

#include "ManaPlayerFactoryNew.h"

#include "AssetToolsModule.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "IAssetTools.h"
#include "Input/Reply.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "Layout/Visibility.h"
#include "ManaPlayer.h"
#include "ManaTexture.h"
#include "Styling/SlateTypes.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"

#include "ManaTextureFactory.h"


#define LOCTEXT_NAMESPACE "UManaPlayerFactoryNew"


/* Local helpers
 *****************************************************************************/

class SManaPlayerFactoryDialog
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SManaPlayerFactoryDialog) { }
	SLATE_END_ARGS()

		/** Construct this widget. */
		void Construct(const FArguments& InArgs, FManaPlayerFactoryNewOptions& InOptions, TSharedRef<SWindow> InWindow)
	{
		Options = &InOptions;
		Window = InWindow;

		ChildSlot
			[
				SNew(SBorder)
				.Visibility(EVisibility::Visible)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
			.FillHeight(1)
			.VAlign(VAlign_Top)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			.Content()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CreateAdditionalAssetsLabel", "Additional assets to create and link to the Mana Player:"))
			]

		+ SVerticalBox::Slot()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(SCheckBox)
				.IsChecked(Options->CreateManaTexture ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState CheckBoxState) {
			Options->CreateManaTexture = (CheckBoxState == ECheckBoxState::Checked);
		})
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CreateManaTextureLabel", "Video output ManaTexture asset"))
			]
			]
			]
			]

		+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(8)
			[
				SNew(SUniformGridPanel)
				.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))

			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
			.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
			.OnClicked_Lambda([this]() -> FReply { CloseDialog(true); return FReply::Handled(); })
			.Text(LOCTEXT("OkButtonLabel", "OK"))
			]

		+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
			.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
			.OnClicked_Lambda([this]() -> FReply { CloseDialog(false); return FReply::Handled(); })
			.Text(LOCTEXT("CancelButtonLabel", "Cancel"))
			]
			]
			]
			];
	}

protected:

	void CloseDialog(bool InOkClicked)
	{
		Options->OkClicked = InOkClicked;

		if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
		}
	}

private:

	FManaPlayerFactoryNewOptions* Options;
	TWeakPtr<SWindow> Window;
};


/* UManaPlayerFactoryNew structors
 *****************************************************************************/

UManaPlayerFactoryNew::UManaPlayerFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UManaPlayer::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


/* UFactory interface
 *****************************************************************************/

bool UManaPlayerFactoryNew::ConfigureProperties()
{
	Options.CreateManaTexture = false;
	Options.OkClicked = false;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("CreateManaPlayerDialogTitle", "Create Mana Player"))
		.ClientSize(FVector2D(400, 160))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	Window->SetContent(SNew(SManaPlayerFactoryDialog, Options, Window));
	GEditor->EditorAddModalWindow(Window);

	return Options.OkClicked;
}


UObject* UManaPlayerFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewManaPlayer = NewObject<UManaPlayer>(InParent, InClass, InName, Flags);

	if ((NewManaPlayer != nullptr) && Options.CreateManaTexture)
	{
		IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		const FString ParentName = InParent->GetOutermost()->GetName();

		FString OutAssetName;
		FString OutPackageName;

		AssetTools.CreateUniqueAssetName(ParentName, TEXT("_Texture"), OutPackageName, OutAssetName);
		const FString PackagePath = FPackageName::GetLongPackagePath(OutPackageName);
		auto Factory = NewObject<UManaTextureFactory>();
		auto NewManaTexture = Cast<UManaTexture>(AssetTools.CreateAsset(OutAssetName, PackagePath, UManaTexture::StaticClass(), Factory));

		if (NewManaTexture != nullptr)
		{
			NewManaPlayer->SetTexture(NewManaTexture);
		}
	}

	return NewManaPlayer;
}


uint32 UManaPlayerFactoryNew::GetMenuCategories() const
{
	return EAssetTypeCategories::Media;
}


bool UManaPlayerFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
