/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2015-2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : YUVToRGB Color Transform Shaders
 * File     : ManaColorTransformShader.h
 *
 ****************************************************************************/

#pragma once

#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "RenderResource.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"
#include "RHIResources.h"
#include "RHIStaticStates.h"
#include "Runtime/Launch/Resources/Version.h"
#include <array>

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 22
 /* YUV平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferYUV, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_Y)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_U)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_V)
SHADER_PARAMETER_SAMPLER(SamplerState, LumaSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, CbSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, CrSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferYUVA, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_Y)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_U)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_V)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_A)
SHADER_PARAMETER_SAMPLER(SamplerState, LumaSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, CbSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, CrSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, AlphaSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferNV12, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_Y)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_UV)
SHADER_PARAMETER_SAMPLER(SamplerState, LumaSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, CbCrSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferNV12A, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_Y)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_UV)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_A)
SHADER_PARAMETER_SAMPLER(SamplerState, LumaSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, CbCrSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, AlphaSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/* RGB平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferRGB, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_RGB)
SHADER_PARAMETER_SAMPLER(SamplerState, ColorSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/* RGB平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferRGBA, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_RGB)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_A)
SHADER_PARAMETER_SAMPLER(SamplerState, ColorSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, AlphaSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/* ExternalTexture平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferExternal, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(TextureExternal, Texture_RGB)
SHADER_PARAMETER_SAMPLER(SamplerState, ColorSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

/* ExternalTexture平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferExternalA, )
SHADER_PARAMETER(float, GammaCorrection)
SHADER_PARAMETER_TEXTURE(TextureExternal, Texture_RGB)
SHADER_PARAMETER_TEXTURE(Texture2D, Texture_A)
SHADER_PARAMETER_SAMPLER(SamplerState, ColorSampler)
SHADER_PARAMETER_SAMPLER(SamplerState, AlphaSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

#elif ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 20
 /* YUV平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUV, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_Y)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_U)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_V)
UNIFORM_MEMBER_SAMPLER(SamplerState, LumaSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, CbSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, CrSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUV)

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUVA, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_Y)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_U)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_V)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_A)
UNIFORM_MEMBER_SAMPLER(SamplerState, LumaSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, CbSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, CrSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUVA)

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_Y)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_UV)
UNIFORM_MEMBER_SAMPLER(SamplerState, LumaSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, CbCrSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12)

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12A, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_Y)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_UV)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_A)
UNIFORM_MEMBER_SAMPLER(SamplerState, LumaSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, CbCrSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12A)

/* RGB平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGB, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_RGB)
UNIFORM_MEMBER_SAMPLER(SamplerState, ColorSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGB)

/* RGB平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGBA, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_RGB)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_A)
UNIFORM_MEMBER_SAMPLER(SamplerState, ColorSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGBA)

/* External平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternal, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(TextureExternal, Texture_RGB)
UNIFORM_MEMBER_SAMPLER(SamplerState, ColorSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternal)

/* External平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternalA, )
UNIFORM_MEMBER(float, GammaCorrection)
UNIFORM_MEMBER_TEXTURE(TextureExternal, Texture_RGB)
UNIFORM_MEMBER_TEXTURE(Texture2D, Texture_A)
UNIFORM_MEMBER_SAMPLER(SamplerState, ColorSampler)
UNIFORM_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternalA)

#else 
 /* YUV平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUV, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_Y)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_U)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_V)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, LumaSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, CbSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, CrSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUV)

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUVA, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_Y)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_U)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_V)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_A)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, LumaSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, CbSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, CrSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUVA)

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_Y)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_UV)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, LumaSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, CbCrSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12)

/* YUV平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12A, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_Y)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_UV)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_A)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, LumaSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, CbCrSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12A)

/* RGB平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGB, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_RGB)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, ColorSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGB)

/* RGB平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGBA, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_RGB)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_A)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, ColorSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGBA)

/* External平面をシェーダーへ渡すための構造体を定義（アルファなし） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternal, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_RGB)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, ColorSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternal)

/* External平面をシェーダーへ渡すための構造体を定義（アルファあり） */
BEGIN_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternalA, )
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(float, GammaCorrection)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(TextureExternal, Texture_RGB)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_TEXTURE(Texture2D, Texture_A)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, ColorSampler)
DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_SAMPLER(SamplerState, AlphaSampler)
END_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternalA)
#endif


#if	ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 25
#define MINIMUM_RHI_FEATURE_LEVEL ERHIFeatureLevel::ES3_1
#else 
#define MINIMUM_RHI_FEATURE_LEVEL ERHIFeatureLevel::ES2
#endif

class FManaColorTransformPixelShader : public FGlobalShader
{
public:
	FManaColorTransformPixelShader() {}
	
	explicit FManaColorTransformPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCache(EShaderPlatform Platform) { return IsFeatureLevelSupported(Platform, MINIMUM_RHI_FEATURE_LEVEL); }

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 19
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, MINIMUM_RHI_FEATURE_LEVEL);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
#endif

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 25
	virtual bool Serialize(FArchive& Ar) override
	{
		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
		return bShaderHasOutdatedParameters;
	}
#endif
};

/* YUVをRGBへ色変換するピクセルシェーダ（アルファなし） */
class FManaColorTransformPixelShader_YUV_to_RGB : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_YUV_to_RGB, Global);

public:
	FManaColorTransformPixelShader_YUV_to_RGB() {}
	explicit FManaColorTransformPixelShader_YUV_to_RGB(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& YUVTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferYUV UniformBuffer;
		UniformBuffer.Texture_Y = YUVTextures[0];
		UniformBuffer.Texture_U = YUVTextures[1];
		UniformBuffer.Texture_V = YUVTextures[2];
		UniformBuffer.LumaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.CbSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.CrSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferYUV>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferYUV>(), Data);
	}
};

/* YUVAをRGBAへ色変換するピクセルシェーダ（アルファあり） */
class FManaColorTransformPixelShader_YUVA_to_RGBA : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_YUVA_to_RGBA, Global);

public:
	FManaColorTransformPixelShader_YUVA_to_RGBA() {}
	explicit FManaColorTransformPixelShader_YUVA_to_RGBA(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& YUVTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferYUVA UniformBuffer;
		UniformBuffer.Texture_Y = YUVTextures[0];
		UniformBuffer.Texture_U = YUVTextures[1];
		UniformBuffer.Texture_V = YUVTextures[2];
		UniformBuffer.Texture_A = YUVTextures[3];
		UniformBuffer.LumaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.CbSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.CrSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.AlphaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferYUVA>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferYUVA>(), Data);
	}
};

/* NV12をRGBへ色変換するピクセルシェーダ（アルファなし） */
class FManaColorTransformPixelShader_NV12_to_RGB : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_NV12_to_RGB, Global);

public:
	FManaColorTransformPixelShader_NV12_to_RGB() {}
	explicit FManaColorTransformPixelShader_NV12_to_RGB(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& YUVTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferNV12 UniformBuffer;
		UniformBuffer.Texture_Y = YUVTextures[0];
		UniformBuffer.Texture_UV = YUVTextures[1];
		UniformBuffer.LumaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.CbCrSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferNV12>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferNV12>(), Data);
	}
};

/* NV12AをRGBAへ色変換するピクセルシェーダ（アルファなし） */
class FManaColorTransformPixelShader_NV12A_to_RGBA : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_NV12A_to_RGBA, Global);

public:
	FManaColorTransformPixelShader_NV12A_to_RGBA() {}
	explicit FManaColorTransformPixelShader_NV12A_to_RGBA(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& YUVTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferNV12A UniformBuffer;
		UniformBuffer.Texture_Y = YUVTextures[0];
		UniformBuffer.Texture_UV = YUVTextures[1];
		UniformBuffer.Texture_A = YUVTextures[2];
		UniformBuffer.LumaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.CbCrSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.AlphaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferNV12A>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferNV12A>(), Data);
	}
};

/* RGBをRGBへ色変換するピクセルシェーダ（アルファなし） */
class FManaColorTransformPixelShader_RGB_to_RGB : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_RGB_to_RGB, Global);

public:
	FManaColorTransformPixelShader_RGB_to_RGB() {}
	explicit FManaColorTransformPixelShader_RGB_to_RGB(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& RGBTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferRGB UniformBuffer;
		UniformBuffer.Texture_RGB = RGBTextures[0];
		UniformBuffer.ColorSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferRGB>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferRGB>(), Data);
	}
};

/* RGBAをRGBAへ色変換するピクセルシェーダ（アルファなし） */
class FManaColorTransformPixelShader_RGBA_to_RGBA : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_RGBA_to_RGBA, Global);

public:
	FManaColorTransformPixelShader_RGBA_to_RGBA() {}
	explicit FManaColorTransformPixelShader_RGBA_to_RGBA(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& RGBTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferRGBA UniformBuffer;
		UniformBuffer.Texture_RGB = RGBTextures[0];
		UniformBuffer.Texture_A = RGBTextures[1];
		UniformBuffer.ColorSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.AlphaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferRGBA>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferRGBA>(), Data);
	}
};

class FManaColorTransformPixelShader_External_to_RGB : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_External_to_RGB, Global);

public:
	FManaColorTransformPixelShader_External_to_RGB() {}
	explicit FManaColorTransformPixelShader_External_to_RGB(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& RGBTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferExternal UniformBuffer;
		UniformBuffer.Texture_RGB = RGBTextures[0];
		UniformBuffer.ColorSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferExternal>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferExternal>(), Data);
	}
};

class FManaColorTransformPixelShader_External_to_RGBA : public FManaColorTransformPixelShader
{
	DECLARE_SHADER_TYPE(FManaColorTransformPixelShader_External_to_RGBA, Global);

public:
	FManaColorTransformPixelShader_External_to_RGBA() {}
	explicit FManaColorTransformPixelShader_External_to_RGBA(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FManaColorTransformPixelShader(Initializer)
	{
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList,
		const TShaderRHIParamRef ShaderRHI,
		const std::array<FTexture2DRHIRef, 6>& RGBTextures,
		float InGammaCorrection)
	{
		FManaUniformBufferExternalA UniformBuffer;
		UniformBuffer.Texture_RGB = RGBTextures[0];
		UniformBuffer.Texture_A = RGBTextures[1];
		UniformBuffer.ColorSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.AlphaSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		UniformBuffer.GammaCorrection = InGammaCorrection;

		auto Data = TUniformBufferRef<FManaUniformBufferExternalA>::CreateUniformBufferImmediate(UniformBuffer, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(RHICmdList, ShaderRHI, GetUniformBufferParameter<FManaUniformBufferExternalA>(), Data);
	}
};

/* 頂点情報をパススルーする頂点シェーダ */
class FManaPaththroughVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FManaPaththroughVertexShader, Global);

public:
	FManaPaththroughVertexShader() {}
	explicit FManaPaththroughVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 19
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, MINIMUM_RHI_FEATURE_LEVEL);
	}
#endif

	static bool ShouldCache(EShaderPlatform Platform) { return true; }
};

/* 頂点シェーダに渡す、全画面の四角形を表すデータ */
struct FManaElementVertex
{
	FVector4 Position;
	FVector2D UV;

	FManaElementVertex() { }

	FManaElementVertex(const FVector4& InPosition, const FVector2D& InTextureCoordinate)
		: Position(InPosition)
		, UV(InTextureCoordinate)
	{ }
};

/* 頂点シェーダへ渡すパラメータ */
class FManaVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FManaElementVertex);
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FManaElementVertex, Position), VET_Float4, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FManaElementVertex, UV), VET_Float2, 1, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};
