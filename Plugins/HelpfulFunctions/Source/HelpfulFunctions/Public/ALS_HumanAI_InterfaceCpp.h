// Copyright Jakub W, All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALS_HumanAI_InterfaceCpp.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UALS_HumanAI_InterfaceCpp : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HELPFULFUNCTIONS_API IALS_HumanAI_InterfaceCpp
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AI States")
		void HAI_GetDeathState(bool& IsDeath);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AI States")
		void HAI_GetEnemyState(bool& IsEnemy);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AI States")
		void HAI_GetCharacterType(bool& IsSolider, bool& IsZombie);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AI States")
		void HAI_GetHumanAIValues(bool& HavePistol, bool& HaveRifle, bool& IsSiting, bool& IsHostage);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AI States")
		void HAI_GetDetectedEnemy(bool& DetectedEnemy, bool& IsSelfEnemy);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "AI States")
		void HAI_DrawDebugTraces(bool& DrawDebug);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "AI States")
		void HAI_GetControllerSmallValues(bool& DetectedEnemy, float& DetectedEnemyTime , ACharacter*& EnemyActor);


};
