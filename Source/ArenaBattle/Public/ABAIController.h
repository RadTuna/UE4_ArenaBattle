// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArenaBattle.h"
#include "AIController.h"
#include "ABAIController.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AABAIController();

public:
	static const FName HomePosKey;
	static const FName PatrolPosKey;
	static const FName TargetKey;

	virtual void OnPossess(APawn* Pawn) override;

	void RunAI();
	void StopAI();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class UBehaviorTree* BTAsset;

	UPROPERTY()
	class UBlackboardData* BBAsset;

	float RepeatInterval;
};
