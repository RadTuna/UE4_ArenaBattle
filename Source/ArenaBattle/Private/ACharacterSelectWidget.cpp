// Fill out your copyright notice in the Description page of Project Settings.


#include "ACharacterSelectWidget.h"
#include "ABCharacterSetting.h"
#include "ABGameInstance.h"
#include "EngineUtils.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "ABSaveGame.h"
#include "ABPlayerState.h"

void UACharacterSelectWidget::NextCharacter(bool bForward)
{
	if (bForward == true)
	{
		CurrentIndex++;
	}
	else
	{
		CurrentIndex--;
	}

	if (CurrentIndex == -1)
	{
		CurrentIndex = MaxIndex - 1;
	}
	if (CurrentIndex == MaxIndex)
	{
		CurrentIndex = 0;
	}

	const UABCharacterSetting* CharacterSetting = GetDefault<UABCharacterSetting>();
	FSoftObjectPath AssetRef = CharacterSetting->CharacterAssets[CurrentIndex];

	UABGameInstance* ABGameInstance = GetWorld()->GetGameInstance<UABGameInstance>();
	ABCHECK(ABGameInstance != nullptr);
	ABCHECK(TargetComponent.IsValid());

	USkeletalMesh* Asset = ABGameInstance->StreamableManager.LoadSynchronous<USkeletalMesh>(AssetRef);
	if (Asset != nullptr)
	{
		TargetComponent->SetSkeletalMesh(Asset);
	}
}

void UACharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentIndex = 0;
	const UABCharacterSetting* CharacterSetting = GetDefault<UABCharacterSetting>();
	MaxIndex = CharacterSetting->CharacterAssets.Num();

	for (TActorIterator<ASkeletalMeshActor> It(GetWorld()); It; ++It)
	{
		TargetComponent = It->GetSkeletalMeshComponent();
		break;
	}

	PrevButton = Cast<UButton>(GetWidgetFromName(TEXT("btnPrev")));
	ABCHECK(PrevButton != nullptr);

	NextButton = Cast<UButton>(GetWidgetFromName(TEXT("btnNext")));
	ABCHECK(NextButton != nullptr);

	TextBox = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("edtPlayerName")));

	ConfirmButton = Cast<UButton>(GetWidgetFromName(TEXT("btnConfirm")));
	ABCHECK(ConfirmButton != nullptr);

	PrevButton->OnClicked.AddDynamic(this, &UACharacterSelectWidget::OnPrevClicked);
	NextButton->OnClicked.AddDynamic(this, &UACharacterSelectWidget::OnNextClicked);
	ConfirmButton->OnClicked.AddDynamic(this, &UACharacterSelectWidget::OnConfirmClicked);

}

void UACharacterSelectWidget::OnPrevClicked()
{
	NextCharacter(true);
}

void UACharacterSelectWidget::OnNextClicked()
{
	NextCharacter(false);
}

void UACharacterSelectWidget::OnConfirmClicked()
{
	FString CharacterName = TextBox->GetText().ToString();
	if (CharacterName.Len() <= 0 || CharacterName.Len() > 10)
	{
		return;
	}

	UABSaveGame* NewPlayerData = NewObject<UABSaveGame>();
	NewPlayerData->PlayerName = CharacterName;
	NewPlayerData->Level = 1;
	NewPlayerData->Exp = 0;
	NewPlayerData->HighScore = 0;
	NewPlayerData->CharacterIndex = CurrentIndex;

	const AABPlayerState* ABPlayerState = GetDefault<AABPlayerState>();
	if (UGameplayStatics::SaveGameToSlot(NewPlayerData, ABPlayerState->SaveSlotName, 0))
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("GamePlay"));
	}
	else
	{
		ABLOG(Error, TEXT("SaveGame Error!"));
	}
}

