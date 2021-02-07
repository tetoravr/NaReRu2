/****************************************************************************
*
* CRI Middleware SDK
*
* Copyright (c) 2018 CRI Middleware Co., Ltd.
*
* Library  : CRIWARE plugin for Unreal Engine 4
* Module   : Platform
* File     : CriWarePlatform.h
*
****************************************************************************/
#pragma once

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#include "ManaComponent.h"
#include "ManaTextureResource.h"
#endif	/* </cri_delete_if_LE> */
#include "CriWareMemory.h"

//---------------------------------------------------------
// Platform specific module interfaces
//---------------------------------------------------------

class ICriWarePlatformAtom {
protected:
	virtual ~ICriWarePlatformAtom() {}
public:
	virtual void InitializeAtomLibrary() {}
	virtual void FinalizeAtomLibrary() {}
};

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
class ICriWarePlatformMana {
protected:
	virtual ~ICriWarePlatformMana() {}
public:
	FORCEINLINE virtual FCriWareAllocator& GetMemoryAllocator(ECriWareMemoryType MemoryType) { return FCriWareAllocator::GetDefault(); }

	virtual void InitializeManaLibrary() {}
	virtual void FinalizeManaLibrary() {}

	virtual FTextureResource* CreateManaTextureResource(UManaTexture* ManaTexture) { return new FManaTextureResource(ManaTexture); }
	
	// old interface
	virtual void InitializeManaComponent(UManaComponent& ManaComponent) {}
	virtual void UninitializeManaComponent(UManaComponent& ManaComponent) {}
	virtual FRHITexture2D* UpdateManaTexture(const UManaTexture& Owner, const CriManaTextureBuffer& ManaTextureBuffer,
		EManaComponentTextureType ComponentType, const FIntPoint& Dimension, FRHITexture2D *Texture) 
	{ return Texture; }
};
#endif	/* </cri_delete_if_LE> */

class FCriWarePlatform
{
public:
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
	FCriWarePlatform() : IAtom(nullptr), IMana(nullptr) {}
#else	/* </cri_delete_if_LE> */
	FCriWarePlatform() : IAtom(nullptr) {}
#endif

	FORCEINLINE void RegisterPlatformAtomInterface(ICriWarePlatformAtom* AtomInterface) {
		IAtom = AtomInterface;
	}
	FORCEINLINE void UnregisterPlatformAtomInterface() { IAtom = nullptr; }
	FORCEINLINE ICriWarePlatformAtom* Atom() { return IAtom; }
private:
	ICriWarePlatformAtom* IAtom;

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
public:
	FORCEINLINE void RegisterPlatformManaInterface(ICriWarePlatformMana* ManaInterface) {
		IMana = ManaInterface;
	}
	FORCEINLINE void UnregisterPlatformManaInterface() { IMana = nullptr; }
	FORCEINLINE ICriWarePlatformMana* Mana() { return IMana; }

private:
	ICriWarePlatformMana* IMana;
#endif	/* </cri_delete_if_LE> */
};