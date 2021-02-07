/****************************************************************************
*
* CRI Middleware SDK
*
* Copyright (c) 2017 CRI Middleware Co., Ltd.
*
* Library  : CRIWARE plugin for Unreal Engine 4
* Module   : Asset Type Action of Mana Movie Source
* File     : AssetTypeActions_ManaMovie.h
*
****************************************************************************/

#pragma once

#include "AssetTypeActions_Base.h"
#include "ManaMovie.h"

class UManaPlayer;

class FAssetTypeActions_ManaMovie : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_ManaMovie(EAssetTypeCategories::Type InAssetCategory);
	//~ Begin IAssetTypeActions interface
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ManaMovie", "Sofdec2 Movie Source"); }
	virtual UClass* GetSupportedClass() const override { return UManaMovie::StaticClass(); }
	virtual FColor GetTypeColor() const override { return FColor::White; }
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	virtual bool CanFilter() override { return true; }
	virtual uint32 GetCategories() override { return MyAssetCategory; }
	virtual bool IsImportedAsset() const override { return true; }
	virtual void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
	virtual UThumbnailInfo* GetThumbnailInfo(UObject* Asset) const override;
	virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 24
	virtual bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override;
#else
	bool AssetsActivatedOverride(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType);
	virtual void AssetsActivated(const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType) override {
		if (!AssetsActivatedOverride(InObjects, ActivationType)) { FAssetTypeActions_Base::AssetsActivated(InObjects, ActivationType); }
	}
#endif
	//~ End IAssetTypeActions interface

private:
	EAssetTypeCategories::Type MyAssetCategory;

private:
	void ExecuteCreateTexture(TArray<TWeakObjectPtr<UManaMovie>> ManaMovies);
	void ExecuteCreatePlaylist(TArray<TWeakObjectPtr<UManaMovie>> ManaMovies);

protected:
	/** Plays the specified movie */
	void PlayMovie(UManaMovie* Movie) const;

	/** Stops any currently playing movies */
	void StopMovie() const;

	/** Return true if the specified movie is playing */
	bool IsMoviePlaying(UManaMovie* Movie) const;

private:
	/** Handler for when PlayMovie is selected */
	void ExecutePlayMovie(TArray<TWeakObjectPtr<UManaMovie>> Objects) const;

	/** Handler for when StopMovie is selected */
	void ExecuteStopMovie(TArray<TWeakObjectPtr<UManaMovie>> Objects) const;

	/** Returns true if only one movie is selected to play */
	bool CanExecutePlayCommand(TArray<TWeakObjectPtr<UManaMovie>> Objects) const;

	TSharedPtr<UManaPlayer> ManaPlayer;
};
