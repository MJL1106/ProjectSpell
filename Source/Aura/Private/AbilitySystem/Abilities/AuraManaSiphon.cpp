// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraManaSiphon.h"

FString UAuraManaSiphon::GetDescription(int32 Level)
{
	const float SiphonPercentage = ManaSiphonCurve->GetFloatValue(Level) * 100;
	return FString::Printf(TEXT(
		"<Title>MANA SIPHON</>\n\n"

		"<Small>Level: </><Level>%d</>\n"
		"<Small>Siphon Amount: </><ManaCost>%.1f%%</>\n\n"

		"<Default>Siphons "
		"</><Damage>%.1f%%</><Default> of the enemies mana on death, and adds the amount to your mana.</>"
		),
		Level,
		SiphonPercentage,
		SiphonPercentage);
}

FString UAuraManaSiphon::GetNextLevelDescription(int32 Level)
{
	const float SiphonPercentage = ManaSiphonCurve->GetFloatValue(Level) * 100;
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>Protection Chance: </><ManaCost>%.1f%%</>\n\n"

			"<Default>Siphons "
			"</><Damage>%.1f%%</><Default> of the enemies mana on death, and adds the amount to your mana.</>"
			),
			Level,
			SiphonPercentage,
			SiphonPercentage);
}
