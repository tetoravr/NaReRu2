/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2019-2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE Android plugin for Unreal Engine 4
 * Module   : Sofdec2 Mana for Android
 * File     : CriWareManaAndroid.cpp
 *
 ****************************************************************************/

#include "CriWareManaAndroid.h"
// Unreal
#include "RHI.h"
#include "ExternalTexture.h"
#include "Misc/AssertionMacros.h"
// CriWare
#include "CriWareAndroidPrivate.h"
#include "CriWareApi.h"
#include "CriWareInitializer.h"
#include "CriWarePluginSettings.h"
#include "ManaTextureResource.h"
//#include "ExternalOESTextureRenderer.h"


namespace CriJavaMediaCodec
{
	bool SetTexture(CriManaPlayerHn ManaPlayer, FTextureRHIRef Texture)
	{
		CriBool Result = CRI_FALSE;
		int32 TextureId = *reinterpret_cast<int32*>(Texture->GetNativeResource());
#if defined(XPT_TGT_ANDROID)
		//Result = criManaPlayer_AttachToGLContextForJNIMediaCodec_ANDROID(ManaPlayer, TextureId);
		Result = criManaPlayer_SetGLTextureForJNIMediaCodec_ANDROID(ManaPlayer, TextureId);
#endif
		//UE_LOG(LogCriWareAndroid, Log, TEXT("SetTexture %s [%d]"), Result == CRI_FALSE ? TEXT("NG") : TEXT("OK"), TextureId);

		return Result == CRI_TRUE;
	}

	bool UpdateTexture(CriManaPlayerHn ManaPlayer, const UManaTexture& ManaTexture, float& UScale, float& VScale, float& UOffset, float& VOffset)
	{
		CriBool Result = CRI_FALSE;
		//#if defined(XPT_TGT_ANDROID)

		criManaPlayer_UpdateTextureImageForJNIMediaCodec_ANDROID(ManaPlayer);

		float Transfrom[16];
		Result = criManaPlayer_GetTransMatrixForJNIMediaCodec_ANDROID(ManaPlayer, Transfrom);

		if (Result == CRI_TRUE) {
			// 取得した変換行列に、ムービサイズが中途半端な場合にエンコード時に付加される黒枠の領域をクリップするための行列を適用
			//   次式を展開:  transfrom = mul(transfrom, mul(UvScale, UvOffset))
			FIntPoint DispDimensions = ManaTexture.GetMovie()->GetVideoTrackDisplayDimensions(0);
			FIntPoint TexDimensions = ManaTexture.GetMovie()->GetVideoTrackTextureDimensions(0);
			const float DispUScale = DispDimensions.X != TexDimensions.X ? ((float)DispDimensions.X - 0.5) / (float)TexDimensions.X : 1.0f;
			const float DispVScale = DispDimensions.Y != TexDimensions.Y ? ((float)DispDimensions.Y - 0.5) / (float)TexDimensions.Y : 1.0f;
			const float DispVOffset = 1.0f - DispVScale;

			// 不要な計算もあるが、簡単のため省略しない
			Transfrom[12] += Transfrom[4] * DispVOffset;
			Transfrom[13] += Transfrom[5] * DispVOffset;
			Transfrom[14] += Transfrom[6] * DispVOffset;
			Transfrom[15] += Transfrom[7] * DispVOffset;

			Transfrom[0] *= DispUScale;
			Transfrom[1] *= DispUScale;
			Transfrom[2] *= DispUScale;
			Transfrom[3] *= DispUScale;

			Transfrom[4] *= DispVScale;
			Transfrom[5] *= DispVScale;
			Transfrom[6] *= DispVScale;
			Transfrom[7] *= DispVScale;

			// 変換行列が Scale, Offset の成分のみかをチェック
			const size_t ElementIndices[12] = { 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 14, 15 };
			for (size_t i = 0; i < 12; ++i) {
				float const Expected = ((ElementIndices[i] == 10) || (ElementIndices[i] == 15)) ? 1.0f : 0.0f;
				// FLT_EPSILON が適切かは微妙だが、問題無いだろう
				if (fabs(Expected - Transfrom[ElementIndices[i]]) > FLT_EPSILON) {
					// 回転などには対応出来ると思われるが、実質非対応でも問題ないだろう
					UScale = 1.0f;
					VScale = -1.0f;
					UOffset = 0.0f;
					VOffset = 1.0f;

					UE_LOG(LogCriWareAndroid, Error, TEXT("Unsupported transform matrix!"));

					return CRI_FALSE;
				}
			}

			// get scale and offset from matrix  
			UScale = Transfrom[0];
			VScale = -Transfrom[5];
			UOffset = Transfrom[12];
			VOffset = (1.0f - Transfrom[13]);
		}
		//#endif

		//UE_LOG(LogCriWareAndroid, Log, TEXT("UpdateTexture %s (%f %f %f %f)"), Result == CRI_FALSE ? TEXT("NG") : TEXT("OK"), UScale, VScale, UOffset, VOffset);

		return Result == CRI_TRUE;
	}

	bool Reset(CriManaPlayerHn ManaPlayer, FTextureRHIRef Texture)
	{
		CriBool Result = CRI_FALSE;
#if defined(XPT_TGT_ANDROID)
		//Result = criManaPlayer_DetachFromGLContextForJNIMediaCodec_ANDROID(ManaPlayer);
#endif
		//UE_LOG(LogCriWareAndroid, Log, TEXT("Reset %s"), Result == CRI_FALSE ? TEXT("NG") : TEXT("OK"));

		return Result == CRI_TRUE;
	}
} // namespace

// ctor
FCriWareManaAndroid::FCriWareManaAndroid()
	: ICriWarePlatformMana()
{
	// Register this class to CriWareRuntime module for Mana platform specific.
	UCriWareInitializer::Platform.RegisterPlatformManaInterface(this);
}

// dtor
FCriWareManaAndroid::~FCriWareManaAndroid()
{
	UCriWareInitializer::Platform.UnregisterPlatformManaInterface();
}

void FCriWareManaAndroid::InitializeManaLibrary()
{
	// Load CriWare Settings if not already loaded
	GetMutableDefault<UCriWarePluginSettings>()->LoadConfig();
	// get settings
	auto Settings = GetDefault<UCriWarePluginSettings>();

	if (Settings->InitializeMana && Settings->UseH264Decoder) {
#if defined(XPT_TGT_ANDROID)
		// Set up H.264 decoder for Android
    	CriManaJNIMeidiaCodecH264DecoderConfig_ANDROID config;
    	criMana_SetDefaultJNIMeidiaCodecH264DecoderConfig_ANDROID(&config);
    	criMana_SetupJNIMeidiaCodecH264Decoder_ANDROID(&config, NULL, 0);
		//criMana_SetDecodeThreadPriority_ANDROID(10);
#endif
	}
}

void FCriWareManaAndroid::FinalizeManaLibrary()
{
	auto Settings = GetDefault<UCriWarePluginSettings>();

	if (Settings->InitializeMana && Settings->UseH264Decoder) {
#if defined(XPT_TGT_ANDROID)
#endif
	}
}

FTextureResource* FCriWareManaAndroid::CreateManaTextureResource(UManaTexture* ManaTexture)
{
	return new FAndroidManaTextureSampler(ManaTexture);
}

FAndroidManaTextureSampler::FAndroidManaTextureSampler(const UManaTexture* InOwner)
	: FManaTextureResource(InOwner)
{
	RegisterUpdateTextureFunction([this](const UManaTexture& InOwner,
		const CriManaTextureBuffer& InManaTextureBuffer,
		EManaComponentTextureType InComponentType,
		const FIntPoint& InDimension,
		FRHITexture2D* InTexture) -> FRHITexture2D*
	{
		return UpdateComponentTexture(InOwner, InManaTextureBuffer, InComponentType, InDimension, InTexture);
	});
}

void FAndroidManaTextureSampler::InitDynamicRHI()
{
	FManaTextureResource::InitDynamicRHI();

	Guid = FGuid::NewGuid();
	UScale = 0.0f;
	VScale = 0.0f;
	UOffset = 0.0f;
	VOffset = 0.0f;
	VideoTexture = nullptr;

	auto* Owner = GetOwner();

	//UE_LOG(LogTemp, Warning, TEXT(" ManaTexture <%p>"), Owner);
	//UE_LOG(LogTemp, Warning, TEXT(" ManaComponent <%p>"), Owner->GetManaPlayer());

	if (Owner->GetManaPlayer() && Owner->GetManaPlayer()->GetManaPlayer())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		FRHIResourceCreateInfo CreateInfo;
		VideoTexture = RHICmdList.CreateTextureExternal2D(1, 1, PF_R8G8B8A8, 1, 1, ETextureCreateFlags::TexCreate_None, CreateInfo);
		CriJavaMediaCodec::SetTexture(Owner->GetManaPlayer()->GetManaPlayer(), VideoTexture);

		if (VideoTexture == nullptr) {
			UE_LOG(LogCriWareAndroid, Warning, TEXT("CreateTextureExternal2D failed!"));
		}
	}
}

void FAndroidManaTextureSampler::ReleaseDynamicRHI()
{
	FManaTextureResource::ReleaseDynamicRHI();

	if (VideoTexture != nullptr) {

		auto* Owner = GetOwner();

		if (Owner->GetManaPlayer() && Owner->GetManaPlayer()->GetManaPlayer())
		{
			CriJavaMediaCodec::Reset(Owner->GetManaPlayer()->GetManaPlayer(), VideoTexture);
		}

		//FExternalTextureRegistry::Get().UnregisterExternalTexture(Guid);

		// @todo: this causes a crash (UE4 team, see MediaCodec source)
		//VideoTexture->Release();
		
		VideoTexture = nullptr;
	}
}

FRHITexture2D* FAndroidManaTextureSampler::UpdateComponentTexture(
	const UManaTexture& InOwner,
	const CriManaTextureBuffer& ManaTextureBuffer,
	EManaComponentTextureType ComponentType,
	const FIntPoint& InDimension,
	FRHITexture2D* Texture)
{
	check(Texture != nullptr);
	check(ManaTextureBuffer.imagebuf != nullptr);

	if (ComponentType == EManaComponentTextureType::Texture_External)
	{
		float NewUScale, NewVScale, NewUOffset, NewVOffset;

		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		
		// Enqueue an RHI thread command to update the mediacodec surface with correct context
		RHICmdList.EnqueueLambda([&](FRHICommandListImmediate& RHICmdList)
		{
			CriJavaMediaCodec::UpdateTexture(InOwner.GetManaPlayer()->GetManaPlayer(), InOwner, NewUScale, NewVScale, NewUOffset, NewVOffset);
		});

		// Flush to ensure the above command has completed
		RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);

		//int32 TextureId = *reinterpret_cast<int32*>(ComponentAndroid->VideoTexture->GetNativeResource());
		//bool TextureCopyResult = ComponentAndroid->ExternalRenderer->CopyFrameTexture(TextureId, NativeBuffer, Dimension, nullptr);
		//if (!TextureCopyResult)
		//{
		//	UE_LOG(LogCriWareAndroid, Error, TEXT("CopyFrameTexture failed"));
		//}

		// return the new texture pointer to stream
		Texture = VideoTexture->GetTexture2D();

		// TICK if changed
		if (NewUScale != UScale || NewVScale != VScale
			|| NewUOffset != UOffset || NewVOffset != VOffset)
		{
			FSamplerStateInitializerRHI ExternalSamplerStateInitializer(SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp);
			FSamplerStateRHIRef ExternalSamplerStateRHI = RHICreateSamplerState(ExternalSamplerStateInitializer);
			//FExternalTextureRegistry::Get().RegisterExternalTexture(ComponentAndroid->Guid, ComponentAndroid->VideoTexture, SamplerStateRHI, FLinearColor(NewUScale, 0.0f, 0.0f, NewVScale), FLinearColor(NewUOffset, NewVOffset, 0.0f, 0.0f));

			UManaComponentTexture* ComponentTexture = InOwner.GetComponentTexture(UManaComponentTexture::GetPlaneIndexFromType(ComponentType));
			static_cast<FManaComponentTextureResource*>(ComponentTexture->Resource)->RegisterExternalTexture(VideoTexture, ExternalSamplerStateRHI, FLinearColor(NewUScale, 0.0f, 0.0f, NewVScale), FLinearColor(NewUOffset, NewVOffset, 0.0f, 0.0f));

			UScale = NewUScale;
			VScale = NewVScale;
			UOffset = NewUOffset;
			VOffset = NewVOffset;
		}


	}
	else
	{
		// Others - Update texture to GPU
		RHIUpdateTexture2D(
			Texture,
			0,
			FUpdateTextureRegion2D(
				0, 0, 0, 0,
				InDimension.X,
				InDimension.Y
			),
			ManaTextureBuffer.pitch,
			ManaTextureBuffer.imagebuf
		);
	}

	return Texture;
}