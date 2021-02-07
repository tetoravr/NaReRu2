
#pragma once

#include "AssetTypeActions_Base.h"
#include "Styling/ISlateStyle.h"
#include "Templates/SharedPointer.h"

class FMenuBuilder;
class UManaPlayer;

/**
 * Implements an action for UManaPlayer assets.
 */
class FManaPlayerActions
	: public FAssetTypeActions_Base
{
public:

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InStyle The style set to use for asset editor toolkits.
	 */
	FManaPlayerActions(EAssetTypeCategories::Type InAssetCategory);

public:

	//~ FAssetTypeActions_Base interface

	virtual bool CanFilter() override;
	virtual uint32 GetCategories() override;
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:

	/** Asset category to use for this asset. */
	EAssetTypeCategories::Type AssetCategory;
};
