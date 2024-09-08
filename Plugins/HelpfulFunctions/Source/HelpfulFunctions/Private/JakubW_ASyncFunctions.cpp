


#include "JakubW_ASyncFunctions.h"


UJakubW_ASyncFunctions::UJakubW_ASyncFunctions(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer), WorldContextObject(nullptr), MyFloatInput(0.0f)
{
}

UJakubW_ASyncFunctions* UJakubW_ASyncFunctions::WaitForOneFrame(const UObject* WorldContextObject, const float SomeInputVariables)
{
	UJakubW_ASyncFunctions* BlueprintNode = NewObject<UJakubW_ASyncFunctions>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->MyFloatInput = SomeInputVariables;
	return BlueprintNode;
}

void UJakubW_ASyncFunctions::Activate()
{
	// Any safety checks should be performed here. Check here validity of all your pointers etc.
	// You can log any errors using FFrame::KismetExecutionMessage, like that:
	// FFrame::KismetExecutionMessage(TEXT("Valid Player Controller reference is needed for ... to start!"), ELogVerbosity::Error);
	// return;

	WorldContextObject->GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UJakubW_ASyncFunctions::ExecuteAfterOneFrame);
}

void UJakubW_ASyncFunctions::ExecuteAfterOneFrame()
{
	AfterOneFrame.Broadcast(MyFloatInput + 1.0f, MyFloatInput + 2.0f);
}