// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameResultWidget.h"
#include "Components/TextBlock.h"
#include "ABGameState.h"


void UABGameResultWidget::BindGameState(class AABGameState* GameState)
{
	ABCHECK(GameState != nullptr);
	CurrentGameState = GameState;
}

void UABGameResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ABCHECK(CurrentGameState != nullptr);

	UTextBlock* Result = Cast<UTextBlock>(GetWidgetFromName(TEXT("txtResult")));
	ABCHECK(Result != nullptr);
	if (CurrentGameState->IsGameCleared() == true)
	{
		Result->SetText(FText::FromString(TEXT("Mission Complete")));
	}
	else
	{
		Result->SetText(FText::FromString(TEXT("Mission Failed")));
	}

	UTextBlock* TotalScore = Cast<UTextBlock>(GetWidgetFromName(TEXT("txtTotalScore")));
	ABCHECK(TotalScore != nullptr);
	TotalScore->SetText(FText::FromString(FString::FromInt(CurrentGameState->GetTotalGameScore())));
}
