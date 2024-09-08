
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Components/MeshComponent.h"
#include "JakubCablePhysic.generated.h"

class FPrimitiveSceneProxy;

/** Struct containing information about a point along the cable */
struct FCableSingleParticle
{
	FCableSingleParticle()
		: bFree(true)
		, Position(0, 0, 0)
		, OldPosition(0, 0, 0)
		, OldVelocity(0, 0, 0)
		, bIsColliding(false)
		, TargetLocation(0, 0, 0)
	{}
	bool bFree;
	FVector Position;
	FVector OldPosition;
	FVector OldVelocity;
	bool bIsColliding;
	FVector TargetLocation;
};

USTRUCT(BlueprintType)
struct FCableAttachPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int ParticleIndex = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FComponentReference ComponentRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName SocketName = TEXT("None");
};

USTRUCT(BlueprintType)
struct FExposedCableParticle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector Position = FVector(0, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector OldPosition = FVector(0, 0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsColliding = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsFree = false;
};

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(hidecategories = (Object, Physics, Activation, "Components|Activation"), editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class JAKUBCABLECOMPONENT_API UJakubCablePhysic : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

public:

	//~ Begin UActorComponent Interface.
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	virtual void ApplyWorldOffset(const FVector& InOffset, bool bWorldShift) override;
	//~ End UActorComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
	virtual bool HasAnySockets() const override;
	virtual bool DoesSocketExist(FName InSocketName) const override;
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;
	virtual void OnVisibilityChanged() override;
	//~ End USceneComponent Interface.

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.


	/**
	 *	Should we fix the start to something, or leave it free.
	 *	If false, component transform is just used for initial location of start of cable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
		bool bAttachStart;

	UPROPERTY(EditAnywhere, Category = "Cable")
		TArray<FCableAttachPoint> AttachToArray;

	/** End location of cable, relative to AttachEndTo (or AttachEndToSocketName) if specified, otherwise relative to cable component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable", meta = (MakeEditWidget = true))
		FVector EndLocation;

	/* Attach the choosed by index particle to specified component*/
	UFUNCTION(BlueprintCallable, Category = "Cable")
		void SetAttachIndexToComponent(USceneComponent* Component, FName SocketName = NAME_None, int ParticleIndex = 1);

	/* Attach the particle to the specified actor with have a component declared name */
	UFUNCTION(BlueprintCallable, Category = "Cable")
		void SetAttachIndexToActor(AActor* Actor, FName ComponentProperty, FName SocketName = NAME_None, int ParticleIndex = 1);

	/** Gets the Actor that the cable is attached to **/
	UFUNCTION(BlueprintCallable, Category = "Cable")
		TArray<AActor*> GetAttachedActors() const;

	/** Gets the specific informations about attached to something particles **/
	UFUNCTION(BlueprintCallable, Category = "Cable")
		TArray<FCableAttachPoint> GetAttachStructure() const;

	/* Detach from constraint selected particle. If index is out of array size, then try detach last particle */
	UFUNCTION(BlueprintCallable, Category = "Cable")
		bool DetachParticleFrom(int ParticleIndex = 1);

	/** Get array of locations of particles (in world space) making up the cable simulation. */
	UFUNCTION(BlueprintCallable, Category = "Cable")
		void GetCableParticleLocations(TArray<FVector>& Locations) const;

	/** Get array of Particles Structure (in world space) */
	UFUNCTION(BlueprintPure, Category = "Cable")
		TArray<FExposedCableParticle> GetCableParticlesStructure() const;

	/* Get the targets lengths of cable segments. If the cable is attached to only one or two constraint, then the size of array is 1. */
	UFUNCTION(BlueprintCallable, Category = "Cable")
		TArray<float> GetCableInitSegmentsLength() const;

	/* Get the current lengths of cable segments (calculated by sum of distances between particles). 
	If the cable is attached to only one or two constraint, then the size of array is 1.*/
	UFUNCTION(BlueprintCallable, Category = "Cable")
		TArray<float> GetCableCurrentSegmentsLength() const;

	UFUNCTION(BlueprintCallable, Category = "Cable")
		TArray<int> GetIndicesOfAttachedPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Cable")
		void UpdateSubstepTime(float NewSubstepTime = 0.01, bool FixVelocity = true);

	UFUNCTION(BlueprintCallable, Category = "Cable")
		void AddImpulseToParticle(FVector Impulse = FVector(0,0,0), int ParticleIndex = 1, bool UseDistributedForce = false, int UnfoldingRange = 3);

	UFUNCTION(BlueprintCallable, Category = "Cable")
		bool ApplyConstForceToParticle(FVector Force = FVector(0,0,0), int ParticleIndex = 1, bool ExpadLinear = false, int ExpandLenght = 3, float ExpandMinRange = 0.1);

	/* Specified Function deticated to rope swinging system */
	UFUNCTION(BlueprintCallable, Category = "Cable")
		bool GetCableSpaceLenghtForSwinging(int& ParticlesCount, FExposedCableParticle& FirstParticle, FExposedCableParticle& LastParticle, TArray<FExposedCableParticle>& ParticlesInSpace, bool DrawDebug = false);

	UFUNCTION(BlueprintCallable, Category = "Cable")
		void SetParticleLocation(FVector Location = FVector(0, 0, 0), int ParticleIndex = 1);

	UFUNCTION(BlueprintCallable, Category = "Cable")
		void UpdateCableLength(float NewLenght = 100.0, bool RefreshCableInitValues = true);

	UFUNCTION(BlueprintCallable, Category = "Cable")
		TArray<float> GetStretchTolleranceValuePerSegment();

	/* Auto detach the target attach constraint when the segment is out tolerance. Note: The tolerance value must be 
	selected depending on many factors. One of them is the SubstepTime value*/
	UFUNCTION(BlueprintCallable, Category = "Cable")
		bool BreakAttachIfSegmentIsStretched(int& ParticleIndex, float StretchTolerance = 14.0);

	UFUNCTION(BlueprintPure, Category = "Cable")
		bool GetAnyPointIsColliding(int RangeMin = 0, int RangeMax = 100) const;

	UFUNCTION(BlueprintCallable, Category = "Cable")
		void UseForceToTightenTheRope(int EndIndex = 0, int StartIndex = -1, float ForceStrenght = 1000, bool ApplyDistanceForce = false);

	UFUNCTION(BlueprintCallable, Category = "Cable")
		bool ReduceForceForParticles(int StartIndex = 0, int EndIndex = 100, float ReductionStrenght = 1, float dt = 0);

	// ----------------------------------------------------------------------------------------------------------------------------------------------

	/** Rest length of the cable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable", meta = (ClampMin = "10.0", UIMin = "10.0", UIMax = "5000.0"))
		float CableLength;

	/** How many segments the cable has */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cable", meta = (ClampMin = "1", UIMin = "1", UIMax = "100"))
		int32 NumSegments;

	/** Controls the simulation substep time for the cable */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "Cable", meta = (ClampMin = "0.003", UIMin = "0.003", UIMax = "0.1"))
		float SubstepTime;

	/** The number of solver iterations controls how 'stiff' the cable is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable", meta = (ClampMin = "1", ClampMax = "16"))
		int32 SolverIterations;

	/** The range defining how the rope speed damping is to take place. The 'X' value is the beginning of the rope and the 
	'Y' value is the end of the rope. Lower value, that make the the rope will move slower If the vector has the value [1,1], 
	it will mean that the rope has infinite energy, as a consequence, the movement will never be damped.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable", meta = (ClampMin = "0.5", ClampMax = "1.0", UIMin = "0.9", UIMax = "1.0"))
		FVector2D DampingFactorRange;

	/* EXPERIMENTAL!!! - Allows to damp forces by calculating acceleration. It can lead to instability in the simulation.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
		float DampingByAcceleration;

	/** Add stiffness constraints to cable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable")
		bool bEnableStiffness;

	/** When false, will still wait for SubstepTime to elapse before updating, but will only run the cable simulation once using all of accumulated simulation time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable")
		bool bUseSubstepping = true;

	/* Causes the simulation to be updated with each frame. Enabling this option makes the simulation as stable as possible. 
	Stability comes with greater computational demand. This option is recommended if the cable operate with hight velocities.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable")
		bool bSolveSimulationPerFrame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable")
		bool bSkipCableUpdateWhenNotVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable")
		bool bSkipCableUpdateWhenNotOwnerRecentlyRendered = false;

	/* Limit the refresh rate to 60 frames per second. Enabling this option may have a positive effect on the stability of the simulation. 
	If you use the "AddImpulseToParticle" function, changing the value of this parameter to true is recommended.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable")
		bool bLimitTimeTo60FPS = false;

	/** Perform sweeps for each cable particle, each substep, to avoid collisions with the world. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable")
		bool bEnableCollision;

	/** If collision is enabled, control how much sliding friction is applied when cable is in contact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bEnableCollision"))
		float CollisionFriction;

	/* Specifies the maximum number of virtual particles. It allows for much more accurate collision detection, however, it can cause a large decrease in performance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable", meta = (ClampMin = "1", ClampMax = "30", EditCondition = "bEnableCollision"))
		int CollisionSubSegments;

	/* When this option is enabled, dividing to a virtual particles  starts when a collision is detected near the particles (n , n+1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable", meta = (EditCondition = "bEnableCollision"))
		bool OptymalizeSubSegments;

	/* Add actors to be ignored during collision detection*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Cable", meta = (EditCondition = "bEnableCollision"))
		TArray<AActor*> ActorsToIgnoreBySweep;

	/** Force vector (world space) applied to all particles in cable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Forces")
		FVector CableForce;

	/** Scaling applied to world gravity affecting this cable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Forces")
		float CableGravityScale;

	/** How wide the cable geometry is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Rendering", meta = (ClampMin = "0.01", UIMin = "0.01", UIMax = "50.0"))
		float CableWidth;

	/** Number of sides of the cable geometry */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cable Rendering", meta = (ClampMin = "1", ClampMax = "16"))
		int32 NumSides;

	/** How many times to repeat the material along the length of the cable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Rendering", meta = (UIMin = "0.1", UIMax = "8"))
		float TileMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Optimalization")
		bool bUseUpdatingLODs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Optimalization", meta = (UIMin = "0.1", UIMax = "100", ClampMin = "0.1", ClampMax = "100"))
		float LodsRadiusRangeMultipler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable Rendering")
		FVector LastIndexOldPosition;

private:

	void SolveConstraints(); //rozwiaz ograniczenia - sprowadz czastki to stalej odleglosci
	void VerletIntegrate(float InSubstepTime, const FVector& Gravity); //calkowanie Varela
	void PerformCableCollision(); //rozwiarz kolizje dla symulacji
	void PerformSubstep(float InSubstepTime, const FVector& Gravity); //uruchom odswierzanie
	void RefreshParticlesIsFree();
	void GetStartPosition(FVector& OutStartPosition);
	FVector GetAttachPosition(FCableAttachPoint& in);
	bool CheckSubSegmentCollision(FCableSingleParticle& ParticleA, FCableSingleParticle& ParticleB, float Alpha); 	// Solve a more detailed collision detection
	void RefreshLodState();
	FExposedCableParticle PrivateToExposedStructure(FCableSingleParticle& InStruct);
	float TimeRemainder;
	int LodCurrentState;
	float DefSubstepTime;
	TArray<float> SegmentsLength;
	TArray<float> CurrentSegmentsLength;
	TArray<FCableSingleParticle> Particles;
	TArray<FVector> AdditiveForces;

	friend class FCableSceneProxy;
};



