// Fill out your copyright notice in the Description page of Project Settings.


#include "ABItemBox.h"
#include "ABCharacter.h"
#include "ABWeapon.h"

// Sets default values
AABItemBox::AABItemBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BOX"));
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TRIGGER"));

	RootComponent = Trigger;
	Box->SetupAttachment(Trigger);

	Trigger->SetBoxExtent(FVector(40.f, 42.f, 30.f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		SM_BOX(TEXT("StaticMesh'/Game/InfinityBladeGrassLands/Environments/Breakables/StaticMesh/Box/SM_Env_Breakables_Box1.SM_Env_Breakables_Box1'"));
	if (SM_BOX.Succeeded())
	{
		Box->SetStaticMesh(SM_BOX.Object);
	}
	Box->SetRelativeLocation(FVector(0.f, -3.5f, -30.f));

	Trigger->SetCollisionProfileName(TEXT("ItemBox"));
	Box->SetCollisionProfileName(TEXT("NoCollision"));
}

// Called when the game starts or when spawned
void AABItemBox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AABItemBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AABItemBox::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &AABItemBox::OnCharacterOverlap);
}

void AABItemBox::OnCharacterOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AABCharacter* Character = Cast<AABCharacter>(OtherActor);
	if (Character != nullptr)
	{
		FName WeaponSocket(TEXT("hand_rSocket"));
		if (Character->GetMesh()->DoesSocketExist(WeaponSocket))
		{
			AABWeapon* CurWeapon = GetWorld()->SpawnActor<AABWeapon>(FVector::ZeroVector, FRotator::ZeroRotator);
			if (CurWeapon != nullptr)
			{
				CurWeapon->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
			}
		}
	}
	ABLOG_S(Warning);
}

