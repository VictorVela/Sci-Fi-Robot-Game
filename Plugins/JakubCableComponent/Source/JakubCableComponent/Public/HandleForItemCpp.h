

#pragma once

#include "CoreMinimal.h"
#include "JakubSimpleParticleComponent.h"
#include "HandleForItemCpp.generated.h"

UENUM(BlueprintType)
namespace EIWALS_HandleItemType
{
	enum Type : int
	{
		None				UMETA(DisplayName = "None"),
		Binoculars			UMETA(DisplayName = "Binoculars"),
		Bow					UMETA(DisplayName = "Bow"),
		Axe					UMETA(DisplayName = "Axe"),
		Knife				UMETA(DisplayName = "Knife"),
		Sword				UMETA(DisplayName = "Sword"),
		Food				UMETA(DisplayName = "Food"),
		FirstAidKit			UMETA(DisplayName = "FirstAidKit"),
		Grenade_1			UMETA(DisplayName = "Grenade_1"),
		Grenade_2			UMETA(DisplayName = "Grenade_2"),
		Prop_1				UMETA(DisplayName = "Prop_1"),
		Prop_2				UMETA(DisplayName = "Prop_2")
	};
}


UCLASS(Blueprintable, ClassGroup = (Physics), meta = (BlueprintSpawnableComponent))
class JAKUBCABLECOMPONENT_API UHandleForItemCpp : public UJakubSimpleParticleComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle Config", meta = (DisplayName = "Socket Type", Keywords = "Physic Handle Item"))
		TEnumAsByte<EIWALS_HandleItemType::Type> SocketTypeC = EIWALS_HandleItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle Config", meta = (DisplayName = "Attach Socket Name", Keywords = "Physic Handle Item"))
		FName AttachSocketNameC = TEXT("Socket_For_Item_1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle Config", meta = (DisplayName = "Attach Rule", Keywords = "Physic Handle Item"))
		EAttachmentRule AttachRuleC = EAttachmentRule::SnapToTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle Config", meta = (DisplayName = "Attach To Backpack", Keywords = "Physic Handle Item"))
		bool AttachToBackpackC = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle Config", meta = (DisplayName = "Can Equip Item", Keywords = "Physic Handle Item"))
		bool CanEquipItemC = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle Config", meta = (DisplayName = "Prop Attach Offset", Keywords = "Physic Handle Item"))
		FTransform PropAttachOffsetC = FTransform(FRotator(0, 0, 0), FVector(0, 0, 0), FVector(1, 1, 1));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handle Config", meta = (DisplayName = "Drop Item Delay", Keywords = "Physic Handle Item"))
		float DropItemDelay = 0.0;

	//System Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "Is In Equipment", Keywords = "Physic Handle Item"))
		bool IsInEquipmentC = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "Character", Keywords = "Physic Handle Item"))
		ACharacter* CharacterC = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "Item To Pick", Keywords = "Physic Handle Item"))
		AActor* ItemToPickC = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System", meta = (DisplayName = "Backpack Actor", Keywords = "Physic Handle Item"))
		AActor* BackpackActorC = nullptr;



};
