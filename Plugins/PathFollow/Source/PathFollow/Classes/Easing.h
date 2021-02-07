#pragma once

#include "Math/UnrealMathUtility.h"
#include "Easing.generated.h"

/*
* Easing types.
*/
UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EEasingType : uint8
{
	Linear UMETA(DisplayName = "Linear"),

	CircularIn UMETA(DisplayName = "Circular In"),
	CircularOut UMETA(DisplayName = "Circular Out"),
	CircularInOut UMETA(DisplayName = "Circular InOut"),

	EaseIn UMETA(DisplayName = "Ease In"),
	EaseOut UMETA(DisplayName = "Ease Out"),
	EaseInOut UMETA(DisplayName = "Ease InOut"),

	ExpoIn UMETA(DisplayName = "Expo In"),
	ExpoOut UMETA(DisplayName = "Expo Out"),
	ExpoInOut UMETA(DisplayName = "Expo InOut"),

	SinIn UMETA(DisplayName = "Sin In"),
	SinOut UMETA(DisplayName = "Sin Out"),
	SinInOut UMETA(DisplayName = "Sin InOut")
};

static float Ease(EEasingType EasingType, float CurrAlpha, float CurveDegree = 2.0f)
{
	switch (EasingType)
	{
	case EEasingType::Linear:
		return CurrAlpha;
		break;

	case EEasingType::CircularIn:
		return FMath::InterpCircularIn(0.f, 1.f, CurrAlpha);
		break;
	case EEasingType::CircularOut:
		return FMath::InterpCircularOut(0.f, 1.f, CurrAlpha);
		break;
	case EEasingType::CircularInOut:
		return FMath::InterpCircularInOut(0.f, 1.f, CurrAlpha);
		break;

	case EEasingType::EaseIn:
		return FMath::InterpEaseIn(0.f, 1.f, CurrAlpha, CurveDegree);
		break;
	case EEasingType::EaseOut:
		return FMath::InterpEaseOut(0.f, 1.f, CurrAlpha, CurveDegree);
		break;
	case EEasingType::EaseInOut:
		return FMath::InterpEaseInOut(0.f, 1.f, CurrAlpha, CurveDegree);
		break;

	case EEasingType::ExpoIn:
		return FMath::InterpExpoIn(0.f, 1.f, CurrAlpha);
		break;
	case EEasingType::ExpoOut:
		return FMath::InterpExpoOut(0.f, 1.f, CurrAlpha);
		break;
	case EEasingType::ExpoInOut:
		return FMath::InterpExpoInOut(0.f, 1.f, CurrAlpha);
		break;

	case EEasingType::SinIn:
		return FMath::InterpSinIn(0.f, 1.f, CurrAlpha);
		break;
	case EEasingType::SinOut:
		return FMath::InterpSinOut(0.f, 1.f, CurrAlpha);
		break;
	case EEasingType::SinInOut:
		return FMath::InterpSinInOut(0.f, 1.f, CurrAlpha);
		break;
	default:
		return CurrAlpha;
		break;
	}
}
