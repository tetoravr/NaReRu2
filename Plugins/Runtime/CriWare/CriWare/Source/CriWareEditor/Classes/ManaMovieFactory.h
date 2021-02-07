/****************************************************************************
*
* CRI Middleware SDK
*
* Copyright (c) 2017 CRI Middleware Co., Ltd.
*
* Library  : CRIWARE plugin for Unreal Engine 4
* Module   : Asset Factory of ManaMovie
* File     : ManaMovieFactory.h
*
****************************************************************************/

#pragma once

#include "Factories/Factory.h"
#include "EditorReimportHandler.h"

#include "ManaMovieFactory.generated.h"


UCLASS(HideCategories = Object)
class UManaMovieFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	UManaMovieFactory(const FObjectInitializer& ObjectInitializer);

	//~ Begin UFactory Interface
	virtual UObject* FactoryCreateNew(
		UClass* Class,
		UObject* InParent,
		FName Name,
		EObjectFlags Flags,
		UObject* Context,
		FFeedbackContext* Warn) override;
	//~ End UFactory Interface

	//~ Begin FReimportHandler interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual const UObject* GetFactoryObject() const { return this; }
	//~ End FReimportHandler interface
};
