// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraLifeSiphon.h"

FString UAuraLifeSiphon::GetDescription(int32 Level)
{
	const float SiphonPercentage = LifeSiphonCurve->GetFloatValue(Level) * 100;
	return FString::Printf(TEXT(
			"<Title>LIFE SIPHON</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>Siphon Amount: </><ManaCost>%.1f%%</>\n\n"

			"<Default>Siphons "
			"</><Damage>%.1f%%</><Default> of the enemies health on death, and adds the amount to your health.</>"
			),
			Level,
			SiphonPercentage,
			SiphonPercentage);
}

FString UAuraLifeSiphon::GetNextLevelDescription(int32 Level)
{
	const float SiphonPercentage = LifeSiphonCurve->GetFloatValue(Level) * 100;
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>Siphon Amount: </><ManaCost>%.1f%%</>\n\n"

			"<Default>Siphons "
			"</><Damage>%.1f%%</><Default> of the enemies health on death, and adds the amount to your health.</>"
			),
			Level,
			SiphonPercentage,
			SiphonPercentage);
}
