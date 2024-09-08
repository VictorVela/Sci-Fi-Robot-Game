

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "ModifyClimbingParamsVolume.generated.h"

UCLASS(HideCategories = (Navigation, HLOD, Input, NetWorking, Replication, Mobile, DataLayers))
class HELPFULFUNCTIONS_API AModifyClimbingParamsVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AModifyClimbingParamsVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*It affects the number of iterations of the ledge detection function in order to analyze more collisions in the Z axis. 
	It allows you to potentially increase the speed of ledge detection. Note: This value affects performance.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "1. Config", DisplayName = "Ledge Up Offset Iterations"))
	int LedgeUpOffsetIterations = 1;

	/*Specifies at what value the starting position of the ledge search function will be moved in the Z axis. 
	If value < 0 then variable is NOT Override value in Climbing Component*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "1. Config", DisplayName = "Up Offset Delta Value"))
	float UpOffsetDeltaValue = 15;

	/* If value < 0 then variable is NOT Override value in Climbing Component*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "1. Config", DisplayName = "Ledge Search Forward Range"))
	float LedgeSearchForwardRange = -1.0;

	/* If the current character velocity Z is lower that this value the modify is not changing parameters for ledge. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True", Category = "1. Config", DisplayName = "Vertical Velocity Tollerance"))
	float VerticalVelocityTollerance = -999.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	UBoxComponent* BoxComponent;

	// Function to handle the begin overlap event
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Function to handle the end overlap event
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool PlayerInVolume = false;
	ACharacter* OverlapeCharacter = nullptr;


	bool CheckThePlayerInVolume(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp);

};
