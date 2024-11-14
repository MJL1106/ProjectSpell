// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraArcaneDecay.h"

FString UAuraArcaneDecay::GetDescription(int32 Level)
{
    const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
    const float ManaCost = FMath::Abs(GetManaCost(Level));
    const float Cooldown = GetCooldown(Level);
    return FString::Printf(TEXT(
            "<Title>ARCANE DECAY</>\n\n"

            "<Small>Level: </><Level>%d</>\n"
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

            "<Default>Summons a pool of arcane decay, "
            "causing repeated decay damage of "
            "</><Damage>%d</><Default> to any enemy in the area.</>"
            ),
            Level,
            ManaCost,
            Cooldown,
            ScaledDamage);
}

FString UAuraArcaneDecay::GetNextLevelDescription(int32 Level)
{
    const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
    const float ManaCost = FMath::Abs(GetManaCost(Level));
    const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"

			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			"<Default>Summons a pool of arcane decay, "
			"causing repeated decay damage of "
			"</><Damage>%d</><Default> to any enemy in the area.</>"
			),
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
}