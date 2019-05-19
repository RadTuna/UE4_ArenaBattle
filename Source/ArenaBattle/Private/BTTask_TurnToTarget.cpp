// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TurnToTarget.h"
#include "ABCharacter.h"
#include "ABAIController.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = TEXT("TurnToTarget");
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	AABCharacter* TargetCharacter = Cast<AABCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(AABAIController::TargetKey));
	if (TargetCharacter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AABCharacter* OwnerCharacter = Cast<AABCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	if (OwnerCharacter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FVector LookVector = TargetCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation();

	FRotator TargetRotator = FRotationMatrix::MakeFromX(LookVector).Rotator();
	OwnerCharacter->SetActorRotation(FMath::RInterpTo(OwnerCharacter->GetActorRotation(), TargetRotator, GetWorld()->GetDeltaSeconds(), 2.f));
	
	return EBTNodeResult::Succeeded;
}

