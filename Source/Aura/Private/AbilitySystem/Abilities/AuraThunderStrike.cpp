// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraThunderStrike.h"

FString UAuraThunderStrike::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>THUNDER STRIKE</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Summons a large lightning bolt, "
			"causing significant damage of "
			"</><Damage>%d</><Default> to any enemy struck by it's power.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}

FString UAuraThunderStrike::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Summons a large lightning bolt, "
			"causing significant damage of "
			"</><Damage>%d</><Default> to any enemy struck by it's power.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}
