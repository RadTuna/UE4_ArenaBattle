// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameInstance.h"

UABGameInstance::UABGameInstance()
{
	FString CharacterDataPath = TEXT("/Game/Book/Data/ABCharacterData.ABCharacterData");
	static ConstructorHelpers::FObjectFinder<UDataTable>
		DT_CHARACTER(*CharacterDataPath);
	if (DT_CHARACTER.Succeeded())
	{
		ABLOG_S(Warning);
		ABCharacterTable = DT_CHARACTER.Object;
		ABLOG(Warning, TEXT("%d"), ABCharacterTable->GetRowMap().Num());
		ABCHECK(ABCharacterTable->GetRowMap().Num() > 0);
	}
}

void UABGameInstance::Init()
{
	Super::Init();

	ABLOG(Warning, TEXT("DropExp of Level 20 ABCharacter : %d"), GetABCharacterData(20)->DropExp);
}

FABCharacterData* UABGameInstance::GetABCharacterData(int32 Level)
{
	return ABCharacterTable->FindRow<FABCharacterData>(*FString::FromInt(Level), TEXT(""));
}

