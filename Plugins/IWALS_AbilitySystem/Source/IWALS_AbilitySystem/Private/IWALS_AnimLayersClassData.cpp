// JakubW

#include "IWALS_AnimLayersClassData.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"


UIWALS_AnimLayersClassData::UIWALS_AnimLayersClassData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UIWALS_AnimLayersClassData::ApplyAnimsLayerForAnimInst(ACharacter* TargetChar, FName OverlayTag) const
{
	if (TargetChar)
	{
		UAnimInstance* AnimInst = TargetChar->GetMesh()->GetAnimInstance();
		if (AnimInst)
		{
			//AnimInst->LinkAnimClassLayers()
			if (AnimInst->GetLinkedAnimGraphInstanceByTag(OverlayTag))
			{
				for (const TSubclassOf<UAnimInstance>& Layer : OverlayStatesFirstLayer)
				{
					AnimInst->GetLinkedAnimGraphInstanceByTag(OverlayTag)->LinkAnimClassLayers(Layer);
				}
				for (const TSubclassOf<UAnimInstance>& Layer2 : OverlayStatesSecondLayer)
				{
					AnimInst->GetLinkedAnimGraphInstanceByTag(OverlayTag)->LinkAnimClassLayers(Layer2);
				}
				for (const TSubclassOf<UAnimInstance>& Layer3 : OverlayStatesThirdLayer)
				{
					AnimInst->GetLinkedAnimGraphInstanceByTag(OverlayTag)->LinkAnimClassLayers(Layer3);
				}
			}
		}
	}
}
