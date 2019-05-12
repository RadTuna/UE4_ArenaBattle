// Fill out your copyright notice in the Description page of Project Settings.

#include "ABCharacter.h"
#include "ABAnimInstance.h"
#include "ABWeapon.h"
#include "DrawDebugHelpers.h"


// Sets default values
AABCharacter::AABCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRING_ARM"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));

	GetMesh()->AddRelativeLocation(FVector(0.f, 0.f, -88.f));
	GetMesh()->AddRelativeRotation(FRotator(0.f, -90.f, 0.f));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	Camera->SetupAttachment(SpringArm);
	
	SpringArm->TargetArmLength = 400.f;
	SpringArm->AddRelativeLocation(FVector(0.f, 0.f, 44.f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh>
		SK_MESH(TEXT("SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard'"));
	if (SK_MESH.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SK_MESH.Object);
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	static ConstructorHelpers::FClassFinder<UAnimInstance>
		ANIM_ASSET(TEXT("AnimBlueprint'/Game/Animations/WarriorAnimBlueprint.WarriorAnimBlueprint_C'"));
	if (ANIM_ASSET.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(ANIM_ASSET.Class);
	}

	SetControlMode(EControlMode::GTA);

	ArmLengthSpeed = 3.f;
	ArmRotationSpeed = 10.f;

	GetCharacterMovement()->JumpZVelocity = 400.f;

	IsAttacking = false;

	MaxCombo = 4;
	AttackEndComboState();

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("ABCharacter"));

	AttackRange = 200.f;
	AttackRadius = 50.f;
}

// Called when the game starts or when spawned
void AABCharacter::BeginPlay()
{
	Super::BeginPlay();
	/*
	FName WeaponSocket(TEXT("hand_rSocket"));
	if (GetMesh()->DoesSocketExist(WeaponSocket))
	{
		AABWeapon* CurWeapon = GetWorld()->SpawnActor<AABWeapon>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (CurWeapon != nullptr)
		{
			CurWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
		}
	}
	*/
}

// Called every frame
void AABCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, ArmLengthTo, DeltaTime, ArmLengthSpeed);

	if (CurrentControlMode == EControlMode::DIABLO)
	{
		SpringArm->RelativeRotation = FMath::RInterpTo(SpringArm->RelativeRotation, ArmRotationTo, DeltaTime, ArmRotationSpeed);
	}

	if (CurrentControlMode == EControlMode::DIABLO && DirectionMove.SizeSquared() > 0.f)
	{
		GetController()->SetControlRotation(FRotationMatrix::MakeFromX(DirectionMove).Rotator());
		AddMovementInput(DirectionMove);
	}

}

// Called to bind functionality to input
void AABCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("ViewChange"), EInputEvent::IE_Pressed, this, &AABCharacter::ViewChange);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AABCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Attack"), EInputEvent::IE_Pressed, this, &AABCharacter::Attack);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AABCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AABCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AABCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AABCharacter::Turn);
}

void AABCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ABAnim = Cast<UABAnimInstance>(GetMesh()->GetAnimInstance());
	ABCHECK(ABAnim != nullptr);

	ABAnim->OnMontageEnded.AddDynamic(this, &AABCharacter::OnAttackMontageEnded);

	ABAnim->OnNextAttackCheck.AddLambda([this]() -> void {
		ABLOG(Warning, TEXT("OnNextAttackCheck"));
		CanNextCombo = false;
		if (IsComboInputOn == true)
		{
			AttackStartComboState();
			ABAnim->JumpToAttackMontageSection(CurrentCombo);
		}
	});
	
	ABAnim->OnAttackHitCheck.AddUObject(this, &AABCharacter::AttackCheck);
}

void AABCharacter::MoveForward(float NewAxisValue)
{
	if (CurrentControlMode == EControlMode::GTA)
	{
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X), NewAxisValue);
	}
	else if (CurrentControlMode == EControlMode::DIABLO)
	{
		DirectionMove.X = NewAxisValue;
	}
}

void AABCharacter::MoveRight(float NewAxisValue)
{
	if (CurrentControlMode == EControlMode::GTA)
	{
		AddMovementInput(FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::Y), NewAxisValue);
	}
	else if (CurrentControlMode == EControlMode::DIABLO)
	{
		DirectionMove.Y = NewAxisValue;
	}
}

void AABCharacter::LookUp(float NewAxisValue)
{
	if (CurrentControlMode == EControlMode::GTA)
	{
		AddControllerPitchInput(NewAxisValue);
	}
}

void AABCharacter::Turn(float NewAxisValue)
{
	if (CurrentControlMode == EControlMode::GTA)
	{
		AddControllerYawInput(NewAxisValue);
	}
}

void AABCharacter::SetControlMode(EControlMode NewControlMode)
{
	CurrentControlMode = NewControlMode;

	if (NewControlMode == EControlMode::GTA)
	{
		//SpringArm->TargetArmLength = 450.f;
		//SpringArm->SetRelativeRotation(FRotator::ZeroRotator);
		ArmLengthTo = 450.f;
		SpringArm->bUsePawnControlRotation = true;
		SpringArm->bInheritPitch = true;
		SpringArm->bInheritRoll = true;
		SpringArm->bInheritYaw = true;
		SpringArm->bDoCollisionTest = true;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
	else if (NewControlMode == EControlMode::DIABLO)
	{
		//SpringArm->TargetArmLength = 800.f;
		//SpringArm->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
		ArmLengthTo = 800.f;
		ArmRotationTo = FRotator(-45.f, 0.f, 0.f);
		SpringArm->bUsePawnControlRotation = false;
		SpringArm->bInheritPitch = false;
		SpringArm->bInheritRoll = false;
		SpringArm->bInheritYaw = false;
		SpringArm->bDoCollisionTest = false;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
	}
}

void AABCharacter::ViewChange()
{
	if (CurrentControlMode == EControlMode::GTA)
	{
		GetController()->SetControlRotation(GetActorRotation());
		SetControlMode(EControlMode::DIABLO);
	}
	else if (CurrentControlMode == EControlMode::DIABLO)
	{
		GetController()->SetControlRotation(SpringArm->RelativeRotation);
		SetControlMode(EControlMode::GTA);
	}
}

void AABCharacter::Attack()
{
	if (IsAttacking == true)
	{
		ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 1, MaxCombo));
		if (CanNextCombo == true)
		{
			IsComboInputOn = true;
		}
	}
	else
	{
		ABCHECK(CurrentCombo == 0);
		AttackStartComboState();
		ABAnim->PlayAttackMontage();
		ABAnim->JumpToAttackMontageSection(CurrentCombo);
		IsAttacking = true;
	}
}

void AABCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	ABCHECK(IsAttacking == true);
	ABCHECK(CurrentCombo > 0);

	IsAttacking = false;
	AttackEndComboState();
}

void AABCharacter::AttackStartComboState()
{
	CanNextCombo = true;
	IsComboInputOn = false;
	ABCHECK(FMath::IsWithinInclusive<int32>(CurrentCombo, 0, MaxCombo - 1));
	CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, MaxCombo);
}

void AABCharacter::AttackEndComboState()
{
	IsComboInputOn = false;
	CanNextCombo = false;
	CurrentCombo = 0;
}


void AABCharacter::AttackCheck()
{
	FHitResult HitResult;
	FCollisionQueryParams Params(NAME_None, false, this);
	bool bResult = GetWorld()->SweepSingleByChannel(
		HitResult,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * 200.f,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(50.f),
		Params);

#if ENABLE_DRAW_DEBUG

	FVector TraceVec = GetActorForwardVector() * AttackRange;
	FVector Center = GetActorLocation() + TraceVec * 0.5f;
	float HalfHeight = AttackRange * 0.5f + AttackRadius;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.0f;

	DrawDebugCapsule(GetWorld(),
		Center,
		HalfHeight,
		AttackRadius,
		CapsuleRot,
		DrawColor,
		false,
		DebugLifeTime);

#endif
	
	if (bResult == true)
	{
		if (HitResult.Actor.IsValid())
		{
			ABLOG(Warning, TEXT("Hit Actor Name : %s"), *HitResult.Actor->GetName());

			FDamageEvent DamageEvent;
			HitResult.Actor->TakeDamage(50.f, DamageEvent, GetController(), this);
		}
	}
}

float AABCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	ABLOG(Warning, TEXT("Actor : %s took Damage : %f"), *GetName(), FinalDamage);
	if (FinalDamage > 0.f)
	{
		ABAnim->SetDeadAnim();
		SetActorEnableCollision(false);
	}
	return FinalDamage;
}
