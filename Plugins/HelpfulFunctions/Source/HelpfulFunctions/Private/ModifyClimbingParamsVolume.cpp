


#include "ModifyClimbingParamsVolume.h"
#include "Cpp_DynamicClimbingComponent.h"

// Sets default values
AModifyClimbingParamsVolume::AModifyClimbingParamsVolume()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;


	// Inicjalizacja BoxComponent
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));

	RootComponent = BoxComponent;
	BoxComponent->Mobility = EComponentMobility::Static;

	// Opcjonalnie: ustawienia dla BoxComponent, np. rozmiar pude³ka
	BoxComponent->InitBoxExtent(FVector(50.0f, 200.0f, 100.0f));

	BoxComponent->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	BoxComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	BoxComponent->ShapeColor = FColor::Blue;
	BoxComponent->SetLineThickness(0.8);

	// Bind the overlap events to functions
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AModifyClimbingParamsVolume::OnOverlapBegin);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AModifyClimbingParamsVolume::OnOverlapEnd);

	SetActorTickEnabled(false);

}

// Called when the game starts or when spawned
void AModifyClimbingParamsVolume::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AModifyClimbingParamsVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void AModifyClimbingParamsVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	PlayerInVolume = CheckThePlayerInVolume(OverlappedComp, OtherActor, OtherComp);
	if (PlayerInVolume == true)
	{
		OverlapeCharacter = Cast<ACharacter>(OtherActor);
		UCpp_DynamicClimbingComponent* ClimbComponent = Cast<UCpp_DynamicClimbingComponent>(OverlapeCharacter->GetComponentByClass(UCpp_DynamicClimbingComponent::StaticClass()));
		if (ClimbComponent)
		{
			ClimbComponent->CurrentModifyVolume = this;
		}
	}
}

void AModifyClimbingParamsVolume::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (PlayerInVolume == true)
	{
		PlayerInVolume = false;
		UCpp_DynamicClimbingComponent* ClimbComponent = Cast<UCpp_DynamicClimbingComponent>(OverlapeCharacter->GetComponentByClass(UCpp_DynamicClimbingComponent::StaticClass()));
		if (ClimbComponent)
		{
			ClimbComponent->CurrentModifyVolume = nullptr;
		}
		ClimbComponent = nullptr;
		OverlapeCharacter = nullptr;
	}
}

bool AModifyClimbingParamsVolume::CheckThePlayerInVolume(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (IsValid(OtherActor) == false) { return false; }

	ACharacter* IsCharacter = Cast<ACharacter>(OtherActor);

	if (IsCharacter)
	{
		if (IsCharacter->IsPlayerControlled() == true)
		{
			return true;
		}
	}
	return false;
}

