// created by JakubW

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "IWALS_BaseAttributeSet.generated.h"

//Uses macro from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class IWALS_ABILITYSYSTEM_API UIWALS_BaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UIWALS_BaseAttributeSet(); //Constructor

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//ATTRIBUTE -> Health
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
		FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UIWALS_BaseAttributeSet, Health);
	UFUNCTION()
		virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	//ATTRIBUTE -> Armor
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Armor)
		FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UIWALS_BaseAttributeSet, Armor);
	UFUNCTION()
		virtual void OnRep_Armor(const FGameplayAttributeData& OldArmor);

	//ATTRIBUTE -> AttackMultiply
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackMultiply)
		FGameplayAttributeData AttackMultiply;
	ATTRIBUTE_ACCESSORS(UIWALS_BaseAttributeSet, AttackMultiply);
	UFUNCTION()
		virtual void OnRep_AttackMultiply(const FGameplayAttributeData& OldAttackMultiply);

	//ATTRIBUTE -> Experience
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Experience)
		FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS(UIWALS_BaseAttributeSet, Experience);
	UFUNCTION()
		virtual void OnRep_Experience(const FGameplayAttributeData& OldExperience);
	
};
