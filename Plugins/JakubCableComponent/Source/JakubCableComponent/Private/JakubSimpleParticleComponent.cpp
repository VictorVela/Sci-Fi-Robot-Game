// Copyright Jakub W, All Rights Reserved.

// plik .h zawsze na górze
#include "JakubSimpleParticleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"



UJakubSimpleParticleComponent::UJakubSimpleParticleComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    ParticleLocation = FVector(MaxDistanceBetweenParticles, 0, 0); // initial position 10 units along the x-axis from the Root
    PreviousParticleLocation = ParticleLocation;
    InitialParticleLocation = ParticleLocation;
    ArrowSize = 0.75;
    ArrowLength = (MaxDistanceBetweenParticles + 20) * FMath::Clamp<float>(1+ (1- ArrowSize),0,1000);
    ArrowColor = FColor::Cyan;


}

void UJakubSimpleParticleComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize Root and PreviousRoot locations
    RootLocation = GetComponentLocation();
    PreviousRootLocation = RootLocation;
}

void UJakubSimpleParticleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    dt = DeltaTime;

    // Update the Root's position
    PreviousRootLocation = RootLocation;
    RootLocation = GetComponentLocation();

    VerletIntegrate(DeltaTime);

    // Now apply the angular constraint after the Verlet integration
    ConstrainAngles();
}

void UJakubSimpleParticleComponent::ConstrainAngles()
{
    FVector ToParticle = ParticleLocation - RootLocation; // wektor od root do cz¹stki

    FVector ForwardVector = GetForwardVector(); // forward vector root

    // ZnajdŸ rotacjê miêdzy ForwardVector a ToParticle.
    FQuat RotationBetween = FQuat::FindBetweenNormals(ForwardVector, ToParticle.GetSafeNormal());

    // Roz³ó¿ tê rotacjê na k¹ty Eulera (Yaw, Pitch, Roll)
    FRotator EulerRotation = RotationBetween.Rotator();

    // SprawdŸ, czy którykolwiek z k¹tów przekracza limit
    if (FMath::Abs(EulerRotation.Pitch) > MaxAngleX ||
        FMath::Abs(EulerRotation.Yaw) > MaxAngleY ||
        FMath::Abs(EulerRotation.Roll) > MaxAngleZ)
    {
        // Dostosuj k¹ty, aby nie przekracza³y limitów
        EulerRotation.Pitch = FMath::Clamp(EulerRotation.Pitch, -MaxAngleX, MaxAngleX);
        EulerRotation.Yaw = FMath::Clamp(EulerRotation.Yaw, -MaxAngleY, MaxAngleY);
        EulerRotation.Roll = FMath::Clamp(EulerRotation.Roll, -MaxAngleZ, MaxAngleZ);
        
        // Konwertuj z powrotem na kwaternion
        FQuat ClampedRotation = FQuat(EulerRotation);

        // Oblicz now¹ pozycjê cz¹stki
        FVector NewToParticle = ClampedRotation.RotateVector(ForwardVector);
        ParticleLocation = RootLocation + NewToParticle * MaxDistanceBetweenParticles; // zak³adamy, ¿e odleg³oœæ wynosi 10 jednostek
    }
}

void UJakubSimpleParticleComponent::VerletIntegrate(float DeltaTime)
{
    FVector Gravity = GravityForce; // Sta³a grawitacji

    FVector RootVelocity = (RootLocation - PreviousRootLocation) / DeltaTime;
    FVector ParticleVelocity = (ParticleLocation - PreviousParticleLocation) / DeltaTime;
    
    //ParticleVelocity = ParticleVelocity.GetClampedToSize(0, 250);
    //GEngine->AddOnScreenDebugMessage(0, 0, FColor::Red, FVector(0,0,ParticleVelocity.Size()).ToString());

    // Podstawowe ograniczenie: Zachowaj sta³¹ odleg³oœæ pomiêdzy cz¹stkami
    FVector ToParticle = ParticleLocation - RootLocation;
    float CurrentDistance = ToParticle.Size();
    FVector ToParticleNormalized = ToParticle / CurrentDistance;
    FVector ConstraintForce = (CurrentDistance - MaxDistanceBetweenParticles) * ToParticleNormalized;

    // Sta³a sprê¿ystoœci
    float k = SpringForceFactor;
    // Oblicz si³ê sprê¿ystoœci
    FVector SpringForce = -k * (ParticleLocation - (GetComponentLocation() + GetForwardVector()*MaxDistanceBetweenParticles));

    FVector Acceleration = ConstraintForce + Gravity; 
    FVector NewParticleLocation = ParticleLocation + ScalarDampingFactor * ParticleVelocity * DeltaTime + 0.5f * Acceleration * DeltaTime * DeltaTime + SpringForce * DeltaTime * DeltaTime;

    // Update the particle's position
    PreviousParticleLocation = ParticleLocation;
    ParticleLocation = NewParticleLocation;

    // Rozwi¹¿ ograniczenia liniowe
       // Keep the distance constraint, move the particle back to within 10 units of the Root if needed
    for (int i = 0; i <= DistanceSolverIterations; i++)
    {
        FVector CorrectedToParticle = ParticleLocation - RootLocation;
        CorrectedToParticle.Normalize();
        ParticleLocation = RootLocation + MaxDistanceBetweenParticles * CorrectedToParticle;
    }

    //Update Relative Offset For Attached Components
    MoveAttachedToParticle();

    //Draw Debug Shapes
    UWorld* W = GetWorld();
    if (W && DrawDebugShapes)
    {
        DrawDebugSphere(W, ParticleLocation, 5.0, 8, FColor::Orange, false, -1, 0, 1.0);
        DrawDebugSphere(W, RootLocation, 5.0, 8, FColor::Green, false, -1, 0, 1.0);
        DrawDebugLine(W, RootLocation, ParticleLocation, FColor::Red, false, -1, 0, 1.5);
    }

}

FVector UJakubSimpleParticleComponent::GetParticleCurrentLocation()
{
    return ParticleLocation;
}

bool UJakubSimpleParticleComponent::AttachComponentToParticle(USceneComponent* Parent, EAttachmentRule LocationRule, EAttachmentRule RotationRule, EAttachmentRule ScaleRule)
{
    const bool AttachResult = K2_AttachToComponent(Parent, TEXT("VirtualParticle"), LocationRule, RotationRule, ScaleRule, true);
    return AttachResult;
}

void UJakubSimpleParticleComponent::MoveAttachedToParticle()
{
    TArray<USceneComponent*> Childrens = {};
    GetChildrenComponents(false, Childrens);
    //Dont make calculations when childrens is empty
    if (Childrens.Num() == 0)
    { return; }

    // Nie umiem zrobiæ lepszej tej rotacji. Skomplikowana sprawa to jest. Niby jako tako jest. Tylko kiedy zmienia siê rotacja komponent to widaæ jakieœ dziwne przesuniecia

    FVector Direction = ParticleLocation - GetComponentLocation();
    Direction.Normalize();

    FQuat LookAtQuat = FQuat::FindBetweenNormals(GetForwardVector(), Direction);

    //LookAtQuat = FQuat::Slerp(PrevRotation, LookAtQuat, dt);

    FRotator TargetRotation = LookAtQuat.Rotator();

    TargetRotation = UKismetMathLibrary::ComposeRotators(TargetRotation, RelativeAttachOffset.Rotator());
    
    FTransform RelativeOffset = FTransform(TargetRotation, ParticleLocation, FVector(1, 1, 1)) * GetComponentTransform().Inverse();

    for (USceneComponent* Child : Childrens)
    {
        if (Child)
        {
            if (Child->GetAttachSocketName() == TEXT("ParticleLocation"))
            {
                if (LocationUpdateInterpSpeed > 0)
                {
                    Child->SetRelativeLocation(UKismetMathLibrary::VInterpTo(Child->GetRelativeLocation(), RelativeOffset.GetLocation() + RelativeAttachOffset.GetLocation(), 
                    dt, LocationUpdateInterpSpeed), false);
                }
                else
                {
                    Child->SetRelativeLocation(RelativeOffset.GetLocation() + RelativeAttachOffset.GetLocation(), false);
                }
                
                if (RotationUpdateInterpSpeed > 0 && UseRelativeRotationUpdate == true)
                {
                    Child->SetRelativeRotation(UKismetMathLibrary::RInterpTo(Child->GetRelativeRotation(), TargetRotation, dt, RotationUpdateInterpSpeed), false);
                }
                else if(UseRelativeRotationUpdate == true)
                {
                    Child->SetRelativeRotation(TargetRotation, false);
                }
            }
        }
    }
    PrevRotation = LookAtQuat;
    return;
}

void UJakubSimpleParticleComponent::SetAttachToParticleOffset(FTransform RelativeOffset)
{
    RelativeAttachOffset = RelativeOffset;
}

FQuat UJakubSimpleParticleComponent::FindBetweenNormals(FVector V1, FVector V2)
{
    return FQuat::FindBetweenNormals(V1, V2);
}
