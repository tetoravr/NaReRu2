/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2015-2020 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : YUVToRGB Color Transform Shaders
 * File     : ManaColorTransformShader.cpp
 *
 ****************************************************************************/

#include "ManaColorTransformShader.h"

#include "CriWareRuntimePrivatePCH.h"

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 22
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferYUV, "ManaUniformBufferYUV");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferYUVA, "ManaUniformBufferYUVA");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferNV12, "ManaUniformBufferNV12");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferNV12A, "ManaUniformBufferNV12A");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferRGB, "ManaUniformBufferRGB");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferRGBA, "ManaUniformBufferRGBA");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferExternal, "ManaUniformBufferExternal");
IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FManaUniformBufferExternalA, "ManaUniformBufferExternalA");
#else 
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUV, TEXT("ManaUniformBufferYUV"));
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferYUVA, TEXT("ManaUniformBufferYUVA"));
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12, TEXT("ManaUniformBufferNV12"));
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferNV12A, TEXT("ManaUniformBufferNV12A"));
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGB, TEXT("ManaUniformBufferRGB"));
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferRGBA, TEXT("ManaUniformBufferRGBA"));
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternal, TEXT("ManaUniformBufferExternal"));
IMPLEMENT_UNIFORM_BUFFER_STRUCT(FManaUniformBufferExternalA, TEXT("ManaUniformBufferExternalA"));
#endif

/* usfファイル名とシェーダー関数名を紐付け */
IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_YUV_to_RGB,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_YUV_to_RGB"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_YUVA_to_RGBA,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_YUVA_to_RGBA"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_NV12_to_RGB,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_NV12_to_RGB"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_NV12A_to_RGBA,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_NV12A_to_RGBA"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_RGB_to_RGB,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_RGB_to_RGB"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_RGBA_to_RGBA,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_RGBA_to_RGBA"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_External_to_RGB,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_External_to_RGB"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaColorTransformPixelShader_External_to_RGBA,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("PixelShader_External_to_RGBA"),
					  SF_Pixel);

IMPLEMENT_SHADER_TYPE(,
					  FManaPaththroughVertexShader,
					  TEXT("/Plugin/CriWare/Private/ManaColorTransform.usf"),
					  TEXT("MainVertexShader"),
					  SF_Vertex);
