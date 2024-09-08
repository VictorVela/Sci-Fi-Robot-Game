// Copyright Jakub W, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "JakubSimpleParticleComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Physics), meta = (BlueprintSpawnableComponent, Thumbnail = "Resources/JakubSimpleParticleComponent_64x.png"))
class JAKUBCABLECOMPONENT_API UJakubSimpleParticleComponent : public UArrowComponent
{
    GENERATED_BODY()

public:
    UJakubSimpleParticleComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintPure, Category = "Particle")
        FVector GetParticleCurrentLocation();

    UFUNCTION(BlueprintCallable, Category = "Particle")
        bool AttachComponentToParticle(USceneComponent* Parent, EAttachmentRule LocationRule, EAttachmentRule RotationRule, EAttachmentRule ScaleRule);

    UFUNCTION(BlueprintCallable, Category = "Particle")
        void SetAttachToParticleOffset(FTransform RelativeOffset);

    UFUNCTION(BlueprintPure, Category = "Particle")
        FQuat FindBetweenNormals(FVector V1, FVector V2);

    /* The maximum angle to which a virtual particle can reach.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
        float MaxAngleX = 45.f;

    /* The maximum angle to which a virtual particle can reach.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
        float MaxAngleY = 45.f;

    /* The maximum angle to which a virtual particle can reach.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "0.0", ClampMax = "180.0", UIMin = "0.0", UIMax = "180.0"))
        float MaxAngleZ = 45.f;

    /* Set the elasticity coefficient for the spring simulation. A value of 0 means no elastic force.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "0.0", ClampMax = "500.0", UIMin = "0.0", UIMax = "500.0"))
        float SpringForceFactor = 0.0;

    /* The value affects the rate of decay of the forces acting on the particle (velocity and acceleration). 
    The smaller the value, the greater the damping. A value of 1 means no damping.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "0.0", ClampMax = "1", UIMin = "0.0", UIMax = "1"))
        float ScalarDampingFactor = 0.98;
    
    /* Specify the distance that the virtual particle should be from the beginning of the component's transformation.*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "1", ClampMax = "100", UIMin = "1", UIMax = "100"))
        float MaxDistanceBetweenParticles = 10.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = "1", ClampMax = "10", UIMin = "1", UIMax = "10"))
        int DistanceSolverIterations = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Force")
        FVector GravityForce = FVector(0,0,-980);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
        bool DrawDebugShapes = false;

    /* If = 0.0 then disabled interpolation*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attach Config")
        float RotationUpdateInterpSpeed = 0.0;

    /* If = 0.0 then disabled interpolation*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attach Config")
        float LocationUpdateInterpSpeed = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attach Config")
        bool UseRelativeRotationUpdate = true;

private:
    FVector ParticleLocation;
    FVector PreviousParticleLocation;
    FVector RootLocation;
    FVector PreviousRootLocation;
    FVector InitialParticleLocation;
    FTransform RelativeAttachOffset;
    FQuat PrevRotation;
    float dt;

    void VerletIntegrate(float DeltaTime);

    // Constrain the angles based on provided max angles in degrees
    void ConstrainAngles();

    void MoveAttachedToParticle();
};
