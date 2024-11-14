// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraHaloOfProtection.h"

FString UAuraHaloOfProtection::GetDescription(int32 Level)
{
	const float ProtectionChance = HaloOfProtectionCurve->GetFloatValue(Level) * 100;
	return FString::Printf(TEXT(
			"<Title>HALO OF PROTECTION</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>Protection Chance: </><ManaCost>%.1f%%</>\n\n"

			"<Default>When active, there is a chance of "
			"</><Damage>%.1f%%</><Default> to survive after death, adding a small amount of health back.</>"
			),
			Level,
			ProtectionChance,
			ProtectionChance);
}

FString UAuraHaloOfProtection::GetNextLevelDescription(int32 Level)
{
	const float ProtectionChance = HaloOfProtectionCurve->GetFloatValue(Level) * 100;
	return FString::Printf(TEXT(
			"<Title>HALO OF PROTECTION</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>Protection Chance: </><ManaCost>%.1f%%</>\n\n"

			"<Default>When active, there is a chance of "
			"</><Damage>%.1f%%</><Default> to survive after death, adding a small amount of health back.</>"
			),
			Level,
			ProtectionChance,
			ProtectionChance);
}
