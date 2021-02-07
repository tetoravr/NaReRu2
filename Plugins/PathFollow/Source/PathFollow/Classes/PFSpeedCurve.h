// Copyright 2015-2017 Jiri Dockal (pathfollow@jiridockal.com). All rights reserved.

#pragma once

#include "PFSpeedCurve.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PFSpeedCurve, Display, All);

USTRUCT()
struct PATHFOLLOW_API FPFSpeedPointsDrawConfig
{
	GENERATED_BODY()

	FPFSpeedPointsDrawConfig();

	UPROPERTY(EditAnywhere, Category = Speed)
	bool bHideSpeedPoints = false;

	UPROPERTY(EditAnywhere, Category = Speed)
	bool bHideSpeedPointInfoText = false;

	//#if WITH_EDITOR
	UPROPERTY(EditAnywhere, Category = Speed)
	FColor SpeedPointsColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category = Speed)
	float SpeedPointHitProxySize = 20.f;

	UPROPERTY(EditAnywhere, Category = Speed, meta = (DisplayName = "Sprite"))
	UTexture2D* _SpeedPointSpriteTexture = nullptr;

	UPROPERTY(EditAnywhere, Category = Speed)
	bool bVisualizeSpeed = false;

	UPROPERTY(EditAnywhere, Category = Speed)
	FLinearColor LowSpeedColor = FLinearColor::Green;

	UPROPERTY(EditAnywhere, Category = Speed)
	FLinearColor HighSpeedColor = FLinearColor::Red;
	
};

/**
 *
 */
USTRUCT(BlueprintType)
struct PATHFOLLOW_API FPFSpeedCurve
{
	GENERATED_BODY()

	FPFSpeedCurve();

	static float GetSpeedAtDistance(const FInterpCurveFloat& Curve, float Distance);
	/*void Clear();
	void Dump();*/

	static TTuple<float, float> GetMinMaxSpeed(const FInterpCurveFloat& Curve);

	const FInterpCurveFloat& GetCurve() const { return _speedCurve; }
	FInterpCurveFloat& GetCurve() { return _speedCurve; }

	UPROPERTY(EditAnywhere, Category = Speed)
	FInterpCurveFloat _speedCurve;
};
