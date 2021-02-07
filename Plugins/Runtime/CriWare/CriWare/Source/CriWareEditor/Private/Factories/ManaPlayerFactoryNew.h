

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Factories/Factory.h"
#include "ManaPlayerFactoryNew.generated.h"

/**
 * Options for UManaPlayerFactoryNew.
 */
struct FManaPlayerFactoryNewOptions
{
	bool CreateManaTexture;
	bool OkClicked;
};


/**
 * Implements a factory for UManaPlayer objects.
 */
UCLASS(hidecategories = Object)
class UManaPlayerFactoryNew
	: public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	//~ UFactory Interface

	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual uint32 GetMenuCategories() const override;
	virtual bool ShouldShowInNewMenu() const override;

private:

	FManaPlayerFactoryNewOptions Options;
};

