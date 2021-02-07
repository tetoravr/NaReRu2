/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2017-2018 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Movie Scene Atom Track
 * File     : MovieSceneAtomTrack.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "Tracks/MovieSceneAtomTrack.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareRuntimePrivatePCH.h"
#include "SoundAtomCue.h"
#include "Evaluation/MovieSceneAtomTemplate.h"

/* Unreal Engine 4関連ヘッダ */
#include "Audio.h"
#include "MovieScene.h"
#include "Sections/MovieSceneAtomSection.h"
#include "Kismet/GameplayStatics.h"
#include "Evaluation/MovieSceneSegment.h"
#include "Compilation/MovieSceneSegmentCompiler.h"
#include "Compilation/MovieSceneCompilerRules.h"
#include "MovieSceneCommonHelpers.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/
#define LOCTEXT_NAMESPACE "MovieSceneAtomTrack"

/***************************************************************************
 *      処理マクロ
 *      Macro Functions
 ***************************************************************************/

/***************************************************************************
 *      データ型宣言
 *      Data Type Declarations
 ***************************************************************************/

/***************************************************************************
 *      変数宣言
 *      Prototype Variables
 ***************************************************************************/

/***************************************************************************
 *      クラス宣言
 *      Prototype Classes
 ***************************************************************************/

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/***************************************************************************
 *      変数定義
 *      Variable Definition
 ***************************************************************************/

/***************************************************************************
 *      クラス定義
 *      Class Definition
 ***************************************************************************/
UMovieSceneAtomTrack::UMovieSceneAtomTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedBlendTypes.Add(EMovieSceneBlendType::Absolute);
#if WITH_EDITORONLY_DATA
	TrackTint = FColor(93, 95, 136);
	RowHeight = 50;
#endif
}

const TArray<UMovieSceneSection*>& UMovieSceneAtomTrack::GetAllSections() const
{
	return AtomSections;
}

bool UMovieSceneAtomTrack::SupportsMultipleRows() const
{
	return true;
}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26
bool UMovieSceneAtomTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneAtomSection::StaticClass();
}

FMovieSceneEvalTemplatePtr UMovieSceneAtomTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FMovieSceneAtomSectionTemplate(*CastChecked<UMovieSceneAtomSection>(&InSection));
}
#endif

void UMovieSceneAtomTrack::RemoveAllAnimationData()
{
	AtomSections.Empty();
}

bool UMovieSceneAtomTrack::HasSection(const UMovieSceneSection& Section) const
{
	return AtomSections.Contains(&Section);
}

void UMovieSceneAtomTrack::AddSection(UMovieSceneSection& Section)
{
	AtomSections.Add(&Section);
}

void UMovieSceneAtomTrack::RemoveSection(UMovieSceneSection& Section)
{
	AtomSections.Remove(&Section);
}

bool UMovieSceneAtomTrack::IsEmpty() const
{
	return AtomSections.Num() == 0;
}

UMovieSceneSection* UMovieSceneAtomTrack::AddNewSoundOnRow(USoundAtomCue* Sound, FFrameNumber Time, int32 RowIndex)
{
	check(Sound);

	/* フレームレートの取得 */
	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();

	/* 音声データの長さ（秒単位）を取得 */
	float SoundDuration = Sound->GetDuration(); // seconds
	if (SoundDuration == INDEFINITELY_LOOPING_DURATION) {
		SoundDuration = Sound->GetFirstWaveDuration(); // if not a wave this return INDEFINITELY_LOOPING_DURATION
		if (SoundDuration == INDEFINITELY_LOOPING_DURATION) {
			SoundDuration = 0.0f;
		}
	}

	/* フレーム単位に換算 */
	FFrameTime DurationToUse = SoundDuration * FrameRate;

	/* セクションの追加 */
	UMovieSceneAtomSection* NewSection = NewObject<UMovieSceneAtomSection>(this);
	NewSection->InitialPlacementOnRow(AtomSections, Time, DurationToUse.FrameNumber.Value, RowIndex);
	NewSection->SetSound(Sound);

	AtomSections.Add(NewSection);

	return NewSection;
}

bool UMovieSceneAtomTrack::IsAMasterTrack() const
{
	UMovieScene* MovieScene = Cast<UMovieScene>(GetOuter());
	return MovieScene ? MovieScene->IsAMasterTrack(*this) : false;
}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26
UMovieSceneSection* UMovieSceneAtomTrack::CreateNewSection()
{
	return NewObject<UMovieSceneAtomSection>(this, NAME_None, RF_Transactional);
}
#endif

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 19
FMovieSceneTrackRowSegmentBlenderPtr UMovieSceneAtomTrack::GetRowSegmentBlender() const
{
	struct FBlender : FMovieSceneTrackRowSegmentBlender
	{
		virtual void Blend(FSegmentBlendData& BlendData) const override
		{
			// Run the default high pass filter for overlap priority
			MovieSceneSegmentCompiler::FilterOutUnderlappingSections(BlendData);

			// Weed out based on array index (legacy behaviour)
			MovieSceneSegmentCompiler::BlendSegmentLegacySectionOrder(BlendData);
		}
	};
	return FBlender();
}
#else
TInlineValue<FMovieSceneSegmentCompilerRules> UMovieSceneAtomTrack::GetRowCompilerRules() const
{
	struct FCompilerRules : FMovieSceneSegmentCompilerRules
	{
		virtual void BlendSegment(FMovieSceneSegment& Segment, const TArrayView<const FMovieSceneSectionData>& SourceData) const
		{
			// Run the default high pass filter for overlap priority
			MovieSceneSegmentCompiler::BlendSegmentHighPass(Segment, SourceData);

			// Weed out based on array index (legacy behaviour)
			MovieSceneSegmentCompiler::BlendSegmentLegacySectionOrder(Segment, SourceData);
		}
	};
	return FCompilerRules();
}
#endif

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/

#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
