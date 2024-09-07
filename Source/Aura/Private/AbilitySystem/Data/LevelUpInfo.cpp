// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP)
{
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		// LevelUpInfo[1] = Level 1 Information
		// LevelUpInfo[2] = Level 2 Information
		if (LevelUpInfo.Num() - 1 <= Level)
		{
			return Level;
		}
		
		if (XP >= LevelUpInfo[Level].LevelUpRequirement)
		{
			Level++;
		}
		else
		{
			bSearching = false;
		}
	}
	return Level;
}
