

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "JakubW_ASyncFunctions.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDelayOneFrameOutputPin, float, InputFloatPlusOne, float, InputFloatPlusTwo);

UCLASS()
class HELPFULFUNCTIONS_API UJakubW_ASyncFunctions : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(BlueprintAssignable)
		FDelayOneFrameOutputPin AfterOneFrame;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "JakubW_FunctionsLibrary| Flow Control")
		static UJakubW_ASyncFunctions* WaitForOneFrame(const UObject* WorldContextObject, const float SomeInputVariables);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	//~UBlueprintAsyncActionBase interface
private:
	UFUNCTION()
		void ExecuteAfterOneFrame();


private:
	const UObject* WorldContextObject;
	float MyFloatInput;
};