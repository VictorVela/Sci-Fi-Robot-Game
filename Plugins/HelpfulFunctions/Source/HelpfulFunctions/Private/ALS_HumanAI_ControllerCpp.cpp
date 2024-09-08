// Copyright Jakub W, All Rights Reserved. 


#include "ALS_HumanAI_ControllerCpp.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#include "HelpfulFunctionsBPLibrary.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "ALS_HumanAI_InterfaceCpp.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"

AALS_HumanAI_ControllerCpp::AALS_HumanAI_ControllerCpp()
{
	PrimaryActorTick.bCanEverTick = true;
}
// |||||||||||||||||||||||||||| B E G I N   P L A Y   E V E N T ||||||||||||||||||||||||||||
void AALS_HumanAI_ControllerCpp::BeginPlay()
{
	Super::BeginPlay();
	//Own = (ACharacter*)Self->GetOwner();
}
// |||||||||||||||||||||||||||| O N   P O S S E S   E V E N T ||||||||||||||||||||||||||||
void AALS_HumanAI_ControllerCpp::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	Own = (ACharacter*)InPawn;
	Self = this;
}
// |||||||||||||||||||||||||||| T I C K   E V E N T ||||||||||||||||||||||||||||
void AALS_HumanAI_ControllerCpp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FRotator VisionMeshRot;
	VisionMeshRot = FRotator(0, Own->GetControlRotation().Yaw - 90, 0);
	if (IsValid(VisionStaticMeshC) == true)
	{ VisionStaticMeshC->SetWorldRotation(UKismetMathLibrary::Conv_RotatorToQuaternion(VisionMeshRot), false); }

}

// '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
// '''''''''''''''''''''''' F U N C T I O N S   L I B R A R Y ''''''''''''''''''''''''
// '''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

// FUNCTION PURE - Interface Test
bool AALS_HumanAI_ControllerCpp::InterfaceTest(ACharacter* Char)
{
	if (IsValid(Char) == true)
	{
		IALS_HumanAI_InterfaceCpp* CppInter = Cast<IALS_HumanAI_InterfaceCpp>(Char);
		bool Death;
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Orange, UKismetSystemLibrary::GetObjectName(Char));
		if (IsValid(Char) == true)
		{
			CppInter->Execute_HAI_GetDeathState(Char,Death);
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FVector(10000,10000,10000).ToString());
			return Death;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FVector(-1, -1, -1).ToString());
		}
	}
	return false;
}

// FUNCTION CALLABLE - Custom Sight Perception
void AALS_HumanAI_ControllerCpp::SightPerceptionByMeshFast(FName HeadSocketName, bool& AndDetected, TArray<AActor*>& CharactersList, int DebugIndex, float MaxDistance)
{
	//Local Variables
	TArray<AActor*> ValidCharacters = {};
	FRotator Orientation = FRotator(0,0,0);
	TArray<AActor*> ToIgnore = {};
	TArray<AActor*> OverlapedActors = {};
	ACharacter* OverlapedChar = nullptr;
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;;
	//Local Interface Variables
	bool DeathState = false;
	bool EnemyState = false;

	//Choose Debug Mode For Trace
	if (DebugIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	ToIgnore.Add(Own);
	ETraceTypeQuery MyTraceQuery = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	VisionStaticMeshC->GetOverlappingActors(OverlapedActors, Own->GetClass());

	for (AActor* DetectedActor : OverlapedActors)
	{
		OverlapedChar = Cast<ACharacter>(DetectedActor);
		DeathState = false;
		EnemyState = false;
		if (IsValid(OverlapedChar) == true && OverlapedChar!=Own)
		{
			IALS_HumanAI_InterfaceCpp* CharInterface = Cast<IALS_HumanAI_InterfaceCpp>(OverlapedChar);
			if (UKismetSystemLibrary::DoesImplementInterface(OverlapedChar, UALS_HumanAI_InterfaceCpp::StaticClass()))
			{
				CharInterface->Execute_HAI_GetDeathState(OverlapedChar, DeathState);
				//CharInterface->Execute_HAI_GetEnemyState(OverlapedChar, EnemyState);
				EnemyState = UHelpfulFunctionsBPLibrary::GetIsEnemyState(Own, Own, OverlapedChar); // [NEW ENEMY STATE FINDING]
			}
			if (DeathState == false && EnemyState == true && UKismetMathLibrary::Vector_Distance2D(Own->GetActorLocation(),OverlapedChar->GetActorLocation())<MaxDistance 
				&& CheckIsInFoliage(OverlapedChar) == false)
			{
				ToIgnore.Add(OverlapedChar);
				FVector TStart = Own->GetMesh()->GetSocketLocation(HeadSocketName);
				FVector TEnd = OverlapedChar->GetMesh()->GetSocketLocation(HeadSocketName);
				TEnd = TEnd + (VisionStaticMeshC->GetRightVector()*-14);
				FHitResult SphereHitResult;
				const bool SphereHitValid = UKismetSystemLibrary::SphereTraceSingle(Own, TStart, TEnd, 6.0, MyTraceQuery, false, ToIgnore, TraceType, SphereHitResult, 
											true, FLinearColor(0.16f, 0.0f, 0.04f, 1.0f), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), 0.05);
				if (SphereHitValid == false && ValidCharacters.Find(OverlapedChar) == -1)
				{ ValidCharacters.Add(OverlapedChar); }
				ToIgnore.Remove(OverlapedChar);
			}
		}
	}
	CharactersList = ValidCharacters;
	if (ValidCharacters.Num() > 0)
	{ AndDetected = true; }
	return;
}

// FUNCTION CALLABLE - Custom Sight Perception
void AALS_HumanAI_ControllerCpp::SightPerceptionByTraceFast(FName HeadSocketName, bool& AndDetected, TArray<AActor*>& CharactersList, int DebugIndex, float MaxDistance)
{
	//Const Parameters
	const float MaxVisionAngle = 75.0;
	const float MinUpVisionSize = 150;
	const float MaxUpVisionSize = 500;
	//Local Variables
	TArray<AActor*> ValidCharacters = {};
	TArray<AActor*> ToIgnore = {};
	TArray<AActor*> OverlapedActors = {};
	ACharacter* OverlapedChar = nullptr;
	EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None;
	//Local Interface Variables
	bool DeathState = false;
	bool EnemyState = false;
	//Choose Debug Mode For Trace
	if (DebugIndex == 1)
	{ TraceType = EDrawDebugTrace::ForOneFrame; }
	else if (DebugIndex == 2)
	{ TraceType = EDrawDebugTrace::ForDuration; }
	ToIgnore.Add(Own);
	//Make Objects Array For Trace And Get Visibility Trace Channel
	EObjectTypeQuery MyTraceQuery =  UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
	TArray<TEnumAsByte<EObjectTypeQuery>, FDefaultAllocator> TraceObjectsArray;
	TraceObjectsArray.Add(MyTraceQuery);
	ETraceTypeQuery VisibilityTrace = UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility);
	// Run Sphere Overlap For Finding Characters
	UKismetSystemLibrary::SphereOverlapActors(Own, Own->GetActorLocation(), MaxDistance, TraceObjectsArray, TSubclassOf<ACharacter>(), ToIgnore, OverlapedActors);
	CharactersList.SetNum(0); // Clear Returning Characters List
	if (OverlapedActors.Num() > 0)
	{
		float AngleBetweenChar = 0;
		float UpOffsetBetweenChar = 0;
		for (AActor* DetectedActor : OverlapedActors) //For Each Loop
		{
			OverlapedChar = Cast<ACharacter>(DetectedActor);
			DeathState = false;
			EnemyState = false;
			FRotator TargetVisionRot = FRotator(0, Own->GetControlRotation().Yaw, 0);
			if (IsValid(OverlapedChar) == true && OverlapedChar != Own)
			{
				AngleBetweenChar = UHelpfulFunctionsBPLibrary::GetAngleBetween(UKismetMathLibrary::GetForwardVector(FRotator(0, UKismetMathLibrary::FindLookAtRotation
				(OverlapedChar->GetActorLocation(), Own->GetActorLocation()).Yaw, 0)), UKismetMathLibrary::GetForwardVector(TargetVisionRot)*-1.0);
				AngleBetweenChar = UKismetMathLibrary::RadiansToDegrees(AngleBetweenChar); //Convert Angle In Radians To Degree
				UpOffsetBetweenChar = abs(OverlapedChar->GetActorLocation().Z - Own->GetActorLocation().Z); // Get Distance Between Actors On Z Axis
				if (AngleBetweenChar < MaxVisionAngle && UpOffsetBetweenChar< UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Vector_Distance2D(
					OverlapedChar->GetActorLocation(), Own->GetActorLocation()), 50, MaxDistance, MinUpVisionSize, MaxUpVisionSize)) //Check Array Element is In Valid Range
				{
					IALS_HumanAI_InterfaceCpp* CharInterface = Cast<IALS_HumanAI_InterfaceCpp>(OverlapedChar);
					if (UKismetSystemLibrary::DoesImplementInterface(OverlapedChar, UALS_HumanAI_InterfaceCpp::StaticClass()))
					{   CharInterface->Execute_HAI_GetDeathState(OverlapedChar, DeathState);    // Check Interface State
						//CharInterface->Execute_HAI_GetEnemyState(OverlapedChar, EnemyState);
						EnemyState = UHelpfulFunctionsBPLibrary::GetIsEnemyState(Own, Own, OverlapedChar); // [NEW ENEMY STATE]
					}  
					//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, OverlapedChar->GetActorLocation().ToString());
					if (DeathState == false && EnemyState == true && CheckIsInFoliage(OverlapedChar) == false)
					{
						ToIgnore.Emplace(OverlapedChar);
						FVector TStart = Own->GetMesh()->GetSocketLocation(HeadSocketName);
						FVector TEnd = OverlapedChar->GetMesh()->GetSocketLocation(HeadSocketName);
						TEnd = TEnd + (UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(TStart,TEnd))*-10.0);
						FHitResult SphereHitResult;
						const bool SphereHitValid = UKismetSystemLibrary::SphereTraceSingle(Own, TStart, TEnd, 6.0, VisibilityTrace, false, ToIgnore, TraceType, SphereHitResult,
							true, FLinearColor(0.16f, 0.0f, 0.04f, 1.0f), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), 0.05);
						if (SphereHitValid == false)
						{ ValidCharacters.AddUnique(OverlapedChar); }
						ToIgnore.Remove(OverlapedChar);
					}
	}}}}
	CharactersList = ValidCharacters; //Set Raturn Array Value
	if (ValidCharacters.Num() > 0) 
	{ AndDetected = true; }
	return; //Finish Function
}

// FUNCTION PURE - 
void AALS_HumanAI_ControllerCpp::GetBestEnemyFromSight(float PlayerDetectionPriority, bool& ReturnValid, ACharacter*& ReturnCharacter)
{
	//Local Variables
	int BestCharIndex = 0;
	int ValidCharIndex = 0;
	float BestCharDistance = 0.0;
	TArray<AActor*> OverlapedActors;
	TArray<ACharacter*> LocalCharactersArray;
	TArray<AActor*> ToIgnore;
	FAIStimulus Stimulus;
	ACharacter* OverlapedChar = nullptr;
	ACharacter* PlayerCharacter = nullptr;
	TArray<float> DistancesArray;
	//Local Interface Variables
	bool DeathState = false;
	bool EnemyState = false;
	bool EnemyDetectedEnemy=false;
	bool EnemyIsSelf = false;

	const TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(Own, Stimulus);
	if (SightComponentC)
	{
		// SIGHT COMPONENT
		SightComponentC->GetCurrentlyPerceivedActors(SenseClass, OverlapedActors); //Get Actors Array From Sight Component
		if (OverlapedActors.Num() > 0)
		{
			for (AActor* DetectedActor : OverlapedActors) //For Each Loop
			{
				DeathState = false;
				EnemyState = false;
				OverlapedChar = Cast<ACharacter>(DetectedActor);
				if (IsValid(OverlapedChar) == true && OverlapedChar != Own)
				{
					IALS_HumanAI_InterfaceCpp* CharInterface = Cast<IALS_HumanAI_InterfaceCpp>(OverlapedChar); //Cas To Interface
					if (UKismetSystemLibrary::DoesImplementInterface(OverlapedChar, UALS_HumanAI_InterfaceCpp::StaticClass()))
					{
						CharInterface->Execute_HAI_GetDeathState(OverlapedChar, DeathState); //Check Interface State
						//CharInterface->Execute_HAI_GetEnemyState(OverlapedChar, EnemyState); //Check Interface State
						EnemyState = UHelpfulFunctionsBPLibrary::GetIsEnemyState(Own, Own, OverlapedChar); // [NEW ENEMY STATE]
					}
					if (DeathState == false && EnemyState == true && CheckIsInFoliage(OverlapedChar)==false)
					{
						LocalCharactersArray.Emplace(OverlapedChar); //Add Current Checked Character To Array
						DistancesArray.Emplace(UKismetMathLibrary::Vector_Distance(OverlapedChar->GetActorLocation(), Own->GetActorLocation()));
						if (OverlapedChar->IsPlayerControlled() == true)
						{
							PlayerCharacter = OverlapedChar;
						}
					}
				}	
			}
		}
		else
		{
			// TRACE CONFIG (MAINLY FOR FINDING ZOMBIE CHARACTERS IN SMALL RADIUS)
			ToIgnore.Add(Own); // Init Local Variable
			bool DrawTraces = false; // Init Local Variable
			EDrawDebugTrace::Type TraceType = EDrawDebugTrace::None; // Init Local Variable
			TArray<FHitResult> SphereHitResult; // Init Local Variable
			//Make Objects Array For Trace
			EObjectTypeQuery MyTraceQuery =  UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
			TArray<TEnumAsByte<EObjectTypeQuery>, FDefaultAllocator> TraceObjectsArray;
			TraceObjectsArray.Add(MyTraceQuery);
			//Get Trace Debug From Inteface
			Cast<IALS_HumanAI_InterfaceCpp>(Own)->Execute_HAI_DrawDebugTraces(Own, DrawTraces); //Get Trace Index From Interface
			if (DrawTraces == true) //By Int Value Choose Enum State
			{ TraceType = EDrawDebugTrace::ForOneFrame; }
			// Run Sphere Trace
			const bool SphereHitValid = UKismetSystemLibrary::SphereTraceMultiForObjects(Own, Own->GetActorLocation(), Own->GetActorLocation(), 200, TraceObjectsArray, 
			false, ToIgnore, TraceType, SphereHitResult, true, FLinearColor(0.16f, 0.0f, 0.04f, 1.0f), FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), 0.05);
			if (SphereHitValid == true)
			{
				for (FHitResult SingleHitResult : SphereHitResult)
				{
					OverlapedChar = Cast<ACharacter>(SingleHitResult.GetActor());
					DeathState = false; EnemyState = false; EnemyIsSelf = false; EnemyDetectedEnemy = false; //Clear Interface Values
					if (IsValid(OverlapedChar) == true && OverlapedChar != Own)
					{
						IALS_HumanAI_InterfaceCpp* CharInterface = Cast<IALS_HumanAI_InterfaceCpp>(OverlapedChar); //Cas To Interface
						if (UKismetSystemLibrary::DoesImplementInterface(OverlapedChar, UALS_HumanAI_InterfaceCpp::StaticClass()))
						{
							CharInterface->Execute_HAI_GetDeathState(OverlapedChar, DeathState); //Check Interface State
							//CharInterface->Execute_HAI_GetEnemyState(OverlapedChar, EnemyState); //Check Interface State
							EnemyState = UHelpfulFunctionsBPLibrary::GetIsEnemyState(Own, Own, OverlapedChar); // [NEW ENEMY STATE]
							CharInterface->Execute_HAI_GetDetectedEnemy(OverlapedChar, EnemyDetectedEnemy, EnemyIsSelf); //Checki Interface State
						}
						if (DeathState == false && EnemyState == true && CheckIsInFoliage(OverlapedChar) == false && 
							OverlapedChar->IsPlayerControlled() == false && EnemyDetectedEnemy == true && EnemyIsSelf == true)
						{
							LocalCharactersArray.Emplace(OverlapedChar); //Add Current Checked Character To Array
							DistancesArray.Emplace(UKismetMathLibrary::Vector_Distance(OverlapedChar->GetActorLocation(), Own->GetActorLocation()));
						}
					}
				}
			}
			else
			{
				ReturnValid = false;
				ReturnCharacter = nullptr;
				return;
			}
		}
		// Try Return Best Enemy From Characters ARRAY
		if (LocalCharactersArray.Num() == 0)
		{ ReturnValid = false; return; }

		UKismetMathLibrary::MinOfFloatArray(DistancesArray, BestCharIndex, BestCharDistance);
		if (IsValid(PlayerCharacter) == true)
		{
			if (UKismetMathLibrary::NearlyEqual_FloatFloat(BestCharDistance,DistancesArray[LocalCharactersArray.Find(PlayerCharacter)],3.0f)==true)
			{
				ValidCharIndex = UKismetMathLibrary::ClampInt64(BestCharIndex, 0, LocalCharactersArray.Num());
				if (LocalCharactersArray.IsValidIndex(ValidCharIndex) == true)
				{ ReturnCharacter = LocalCharactersArray[ValidCharIndex]; }
				ReturnValid = IsValid(ReturnCharacter);
				return; // Finish Function
			} //---------------------------------------------------------------------------------
			else
			{
				const float ArrayIndexFloat = UKismetMathLibrary::Lerp(BestCharIndex * 1.0f, LocalCharactersArray.Find(PlayerCharacter) * 1.0f, PlayerDetectionPriority);
				ReturnCharacter = LocalCharactersArray[UKismetMathLibrary::ClampInt64(UKismetMathLibrary::Round(ArrayIndexFloat), 0, LocalCharactersArray.Num())];
				ReturnValid = IsValid(ReturnCharacter);
				return; // Finish Function
			} //---------------------------------------------------------------------------------
		}
		else
		{
			ReturnCharacter = LocalCharactersArray[UKismetMathLibrary::ClampInt64(BestCharIndex, 0, LocalCharactersArray.Num())];
			ReturnValid = IsValid(ReturnCharacter);
			return; // Finish Function
		} //---------------------------------------------------------------------------------
	}
	ReturnValid = false; //When Sight Component Is NOT Valid then finish function
	return;
}

// FUNCTION PURE - Check Enemy Is In Folage (by Trace)
bool AALS_HumanAI_ControllerCpp::CheckIsInFoliage(ACharacter* EnemyCharacter)
{
	if (DetectedEnemyTimeC < 0.2)
	{
		ECollisionChannel FoliageChannel = static_cast<ECollisionChannel>(FoliageChannelIndexC);
		bool IsNotInFoliage = UHelpfulFunctionsBPLibrary::IsNotHidingInFoliage(Self, Own, EnemyCharacter, FoliageChannel, 0);
		return !IsNotInFoliage;
	}
	else
	{ return false; }
}

bool AALS_HumanAI_ControllerCpp::UseTraceSightPercept()
{
	UBlackboardComponent* BB;
	BB = GetBlackboardComponent();
	if (IsValid(BB) == true)
	{
		if (BB->GetValueAsBool(FName(TEXT("MoveToCoverWithSmallWall"))) == true && LocomotionModeIndexC == 1 && RotationModeIndexC == 2)
		{ return true;}
	}
	return false;
}

// FUNCTION CALLABLE - Find Other Comrate Characters By Using Sphere Overlap
void AALS_HumanAI_ControllerCpp::FindOtherComrate(bool& ReturnValid, TArray<ACharacter*>& ReturnCharacters, float Radius)
{
	TArray<AActor*> OverlapedActors; //Actors Array
	TArray<ACharacter*> LocalCharactersArray; //Characters Array
	TArray<AActor*> ToIgnore; //Actors Array For Setup Sphere Overlap
	ACharacter* OverlapedChar = nullptr; //For Loop Character
	bool DeathState = false; //Interface Value
	bool EnemyState = false; //Interface Value
	EObjectTypeQuery MyTraceQuery = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn); 
	TArray<TEnumAsByte<EObjectTypeQuery>, FDefaultAllocator> TraceObjectsArray;
	TraceObjectsArray.Add(MyTraceQuery); //Make Objects Array For Trace
	ToIgnore.Add(Own);
	ReturnCharacters.SetNum(0); // Clear Returning Characters List
	// Run Sphere Overlap For Characters
	const bool OverlapValid = UKismetSystemLibrary::SphereOverlapActors(Own, Own->GetActorLocation(), Radius, TraceObjectsArray, TSubclassOf<ACharacter>(), ToIgnore, OverlapedActors);
	if (OverlapValid == true)
	{
		for (AActor* ArrElement : OverlapedActors)
		{
			OverlapedChar = Cast<ACharacter>(ArrElement);
			DeathState = false; EnemyState = false; //Clear Interface Values
			if (IsValid(OverlapedChar) == true && OverlapedChar != Own && ArrElement->ActorHasTag(FName(TEXT("Human_AI"))) == true)
			{
				IALS_HumanAI_InterfaceCpp* CharInterface = Cast<IALS_HumanAI_InterfaceCpp>(OverlapedChar); //Cas To Interface
				if (UKismetSystemLibrary::DoesImplementInterface(OverlapedChar, UALS_HumanAI_InterfaceCpp::StaticClass()))
				{
					CharInterface->Execute_HAI_GetDeathState(OverlapedChar, DeathState); //Check Interface State
					//CharInterface->Execute_HAI_GetEnemyState(OverlapedChar, EnemyState); //Check Interface State
					EnemyState = UHelpfulFunctionsBPLibrary::GetIsEnemyState(Own, Own, OverlapedChar);
				}
				if (DeathState == false && EnemyState == false)
				{
					LocalCharactersArray.Emplace(OverlapedChar);
				}
			}
		}
		ReturnValid = LocalCharactersArray.Num() > 0;
		ReturnCharacters = LocalCharactersArray;
		return;
	}
	ReturnCharacters.SetNum(0);
	ReturnValid = false;
	return;
}

// FUNCTION CALLABLE - Base Tick Function - Choose Best Enemy Actor And Save It To Variable
void AALS_HumanAI_ControllerCpp::ChooseEnemyActor(float InterpSpeedMultiplyDown, float InterpSpeedMultiplyUp, float DetectedStateTolerance)
{
	float ConstEnemtyDetectionSpeed = InterpSpeedMultiplyUp;
	ACharacter* LocalCharacter = nullptr;
	bool BestTargetEnemyValid = false;
	ACharacter* EnemyFromCompSight = nullptr;
	bool LODsStateValid = LODsStateIndexC == 0 || LODsStateIndexC == 1;
	bool EnemyIsValidCheck=false;
	float DetectedTargetTime=0.0;
	// Step 1) Choose Enemy Actor (Combine Two Methods - Trace Or Sight Pereption)
	if (UseTraceSightPercept() == true)
	{ LocalCharacter = EnemyFromTraceC; }
	else
	{	GetBestEnemyFromSight(0, BestTargetEnemyValid, EnemyFromCompSight);
		if (BestTargetEnemyValid==true)
		{ LocalCharacter = EnemyFromCompSight; }
		else
		{   if (LODsStateValid && TracePerceptionEnemyValidC==true && IsValid(EnemyFromTraceC)==true)
			{ LocalCharacter = EnemyFromTraceC; }
			else
			{ LocalCharacter = EnemyFromCompSight; } 
		}
	}
	// Step 2)
	if (UseTraceSightPercept() == true)
	{ EnemyIsValidCheck = TracePerceptionEnemyValidC; }
	else
	{
		if (LODsStateValid && TracePerceptionEnemyValidC == true && IsValid(EnemyFromTraceC) == true)
		{ EnemyIsValidCheck = TracePerceptionEnemyValidC; }
		else
		{ EnemyIsValidCheck = BestTargetEnemyValid && IsValid(EnemyFromCompSight); }
	}
	// Step 3)
	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, UKismetSystemLibrary::GetObjectName(LocalCharacter));
	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, UKismetSystemLibrary::GetObjectName(EnemyFromTraceC));
	//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Blue, FVector(0, UseTraceSightPercept()*1.0, 0).ToString());
	if (EnemyIsValidCheck == true && IsValid(LocalCharacter))
	{   // When See Choosed Enemy Character
		DetectedTargetTime = ConstEnemtyDetectionSpeed * UKismetMathLibrary::SelectFloat(4.0, 8.0, LocalCharacter->IsPlayerControlled());
		DetectedTargetTime = DetectedTargetTime * UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Vector_Distance(LocalCharacter->GetActorLocation(), Own->GetActorLocation()), 50, 400, 4, 1);
		DetectedEnemyTimeC = UKismetMathLibrary::FInterpTo(DetectedEnemyTimeC, 1.0, UGameplayStatics::GetWorldDeltaSeconds(Self), DetectedTargetTime);
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FVector(0,DetectedTargetTime, UGameplayStatics::GetWorldDeltaSeconds(Self)).ToString());
	}
	else
	{   // When Not See Any Enemy Character
		DetectedEnemyTimeC = UKismetMathLibrary::FInterpTo(DetectedEnemyTimeC, 0.0, UGameplayStatics::GetWorldDeltaSeconds(Self), 0.2*InterpSpeedMultiplyDown);
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, FVector(0, DetectedTargetTime, UGameplayStatics::GetWorldDeltaSeconds(Self)).ToString());
	}
	//Step 4)
	DetectedEnemyC = DetectedEnemyTimeC > DetectedStateTolerance;
	// Step 5)
	if (IsValid(LocalCharacter) == true)
	{ TargetEnemyActorC = LocalCharacter; }
	// Step 6)
	CombatModeTimeC = UKismetMathLibrary::FInterpTo(CombatModeTimeC, DetectedEnemyC * 1.0, UGameplayStatics::GetWorldDeltaSeconds(Self), 
					  UKismetMathLibrary::SelectFloat(10.0, UKismetMathLibrary::Lerp(0.08, 0.015, InteligenceC),DetectedEnemyC));
}

// FUNCTION CALLABLE
void AALS_HumanAI_ControllerCpp::SetFocusActorTransformFast(AActor* FocusActor, FTransform NewTransform, float InterpSpeedA, float InterpSpeedB, bool PrintInfo)
{
	if (FocusActor)
	{
		float MappedSpeed = 1;
		float MappedDistanceTo = 0;
		float MultiplyByRotMode = 1;
		if (RotationModeIndexC == 0 || RotationModeIndexC==1)
		{ MultiplyByRotMode = 1; }
		else
		{ MultiplyByRotMode = 1.1; }
		if (IsValid(TargetEnemyActorC))
		{
			MappedDistanceTo = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Vector_Distance(TargetEnemyActorC->GetActorLocation(), Own->GetActorLocation()), 300, 3000, 1, 0.8);
			MappedSpeed = MappedDistanceTo * UKismetMathLibrary::SelectFloat(1.0, 0.8, DetectedEnemyC);
			MappedSpeed = MappedSpeed * MultiplyByRotMode;
			if (AttackStateIndexC == 0 || AttackStateIndexC == 1)
			{ MappedSpeed = MappedSpeed - 0.15; }
			MappedSpeed = UKismetMathLibrary::FClamp(MappedSpeed, 0.1, 1);
		}
		else
		{
			MappedSpeed = 0.8 * MultiplyByRotMode;
			if (AttackStateIndexC == 0 || AttackStateIndexC == 1)
			{ MappedSpeed = MappedSpeed - 0.15; }
			MappedSpeed = UKismetMathLibrary::FClamp(MappedSpeed, 0.1, 1);
		}
		if (PrintInfo == true)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, FVector(0, MappedSpeed, 0).ToString());
		}
		FTransform TargetTransform = UKismetMathLibrary::TInterpTo(FocusActor->GetActorTransform(), NewTransform, UGameplayStatics::GetWorldDeltaSeconds(Self), 
									 UKismetMathLibrary::MapRangeClamped(MappedSpeed,0.1,1,InterpSpeedA,InterpSpeedB));
		FocusActor->SetActorTransform(TargetTransform, false);
	}
	else
	{ return; }
}

//FUNCTION PURE
void AALS_HumanAI_ControllerCpp::GetEnemyFromOtherAIs(bool CheckDistanceToPlayer, bool& ReturnValid, ACharacter*& ReturnCharacter)
{
	TArray<AActor*> OverlapedActors; //Actors Array
	TArray<AActor*> ToIgnore; //Actors Array For Setup Sphere Overlap
	TArray<AActor*> ActorsList; //Valid Characters List With Detected Enemy
	ACharacter* OverlapedChar = nullptr; //For Loop Character
	ACharacter* EnemyActor = nullptr; //Interface Value
	bool LocDetectedEnemy = false;
	float LocDetectedTime = 0;
	EObjectTypeQuery MyTraceQuery = UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn);
	TArray<TEnumAsByte<EObjectTypeQuery>, FDefaultAllocator> TraceObjectsArray;
	TraceObjectsArray.Add(MyTraceQuery); //Make Objects Array For Trace
	ToIgnore.Add(Own);

	if (CheckDistanceToPlayer == true)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(Self, 0);
		if (UKismetMathLibrary::Vector_Distance(PlayerPawn->GetActorLocation(), Own->GetActorLocation()) > 2000)
		{ ReturnValid = false; ReturnCharacter = nullptr; return; }
		// Run Sphere Overlap For Characters
		const bool OverlapValid = UKismetSystemLibrary::SphereOverlapActors(Own, Own->GetActorLocation(), 1000, TraceObjectsArray, TSubclassOf<ACharacter>(), ToIgnore, OverlapedActors);
		if (OverlapValid == true)
		{
			for (AActor* ArrElement : OverlapedActors)
			{
				LocDetectedTime = 0; //Set Default Value
				LocDetectedEnemy = false; //Set Default Value
				EnemyActor = nullptr; //Set Default Value
				OverlapedChar = Cast<ACharacter>(ArrElement);
				if (IsValid(OverlapedChar) == true)
				{
					IALS_HumanAI_InterfaceCpp* CharInterface = Cast<IALS_HumanAI_InterfaceCpp>(OverlapedChar); //Cas To Interface
					if (UKismetSystemLibrary::DoesImplementInterface(OverlapedChar, UALS_HumanAI_InterfaceCpp::StaticClass()))
					{
						CharInterface->Execute_HAI_GetControllerSmallValues(OverlapedChar, LocDetectedEnemy, LocDetectedTime, EnemyActor);
						if (LocDetectedTime > 0.6 && IsValid(EnemyActor) == true)
						{
							ActorsList.Emplace(OverlapedChar);
						}
					}
				}
			}
			if (ActorsList.Num() == 0)
			{ ReturnValid = false; ReturnCharacter = nullptr; return; }
			bool ReturnNearFromList = false;
			AActor* ReturnNearActor = nullptr;
			UHelpfulFunctionsBPLibrary::GetNearFromActorsArray(Self, ActorsList, Own->GetActorLocation(), ReturnNearFromList, ReturnNearActor);
			IALS_HumanAI_InterfaceCpp* CharBestInterface = Cast<IALS_HumanAI_InterfaceCpp>(ReturnNearActor); //Cas To Interface
			CharBestInterface->Execute_HAI_GetControllerSmallValues(ReturnNearActor, LocDetectedEnemy, LocDetectedTime, EnemyActor);
			ReturnValid = IsValid(EnemyActor);
			ReturnCharacter = Cast<ACharacter>(EnemyActor);
			return;
		}
	}
	return;
}

//FUNCTION PURE
float AALS_HumanAI_ControllerCpp::GetTheAverageValueOfSpottedEnemiesFast(float InRadius)
{
	bool OtherValid = false;
	float LocalFloat = 0;
	TArray<ACharacter*> ComrateActorsArray;
	ACharacter* OverlapedChar = nullptr;
	FindOtherComrate(OtherValid, ComrateActorsArray, InRadius);
	ACharacter* EnemyActor = nullptr; //Interface Value
	bool LocDetectedEnemy = false; //Interface Value
	float LocDetectedTime = 0; //Interface Value

	if (ComrateActorsArray.Num() > 0)
	{
		for (AActor* ArrElement : ComrateActorsArray)
		{
			LocDetectedTime = 0; //Set Default Value
			LocDetectedEnemy = false; //Set Default Value
			EnemyActor = nullptr; //Set Default Value
			OverlapedChar = Cast<ACharacter>(ArrElement);
			IALS_HumanAI_InterfaceCpp* CharInterface = Cast<IALS_HumanAI_InterfaceCpp>(OverlapedChar); //Cas To Interface
			if (UKismetSystemLibrary::DoesImplementInterface(OverlapedChar, UALS_HumanAI_InterfaceCpp::StaticClass()))
			{
				CharInterface->Execute_HAI_GetControllerSmallValues(OverlapedChar, LocDetectedEnemy, LocDetectedTime, EnemyActor);
				if (LocDetectedEnemy == true && IsValid(EnemyActor) == true)
				{
					LocalFloat = LocalFloat + 1.0;
				}
			}
		}
		return UKismetMathLibrary::SafeDivide(LocalFloat, ComrateActorsArray.Num() * 1.0);
	}
	return 0.0;
}