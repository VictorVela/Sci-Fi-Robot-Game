
// Copyright Epic Games, Inc (MODIFIED by JakubW). All Rights Reserved. 

#include "JakubCablePhysic.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "MaterialDomain.h"
#include "SceneManagement.h"
#include "Engine/CollisionProfile.h"
#include "Materials/Material.h"
#include "Materials/MaterialRenderProxy.h"
#include "Engine/Engine.h"
#include "DynamicMeshBuilder.h"
#include "StaticMeshResources.h"
#include "SceneInterface.h"
#include "JakubCableStats.h"
#include "DrawDebugHelpers.h"
#include "RayTracingInstance.h"
#include "Kismet/GameplayStatics.h"

DEFINE_RENDER_COMMAND_PIPE(JakubCable, ERenderCommandPipeFlags::None);

static TAutoConsoleVariable<int32> CVarRayTracingCableMeshes(
	TEXT("r.RayTracing.Geometry.Cable"),
	1,
	TEXT("Include Cable meshes in ray tracing effects (default = 1 (cable meshes enabled in ray tracing))"));

static TAutoConsoleVariable<int32> CVarRayTracingCableMeshesWPO(
	TEXT("r.RayTracing.Geometry.Cable.WPO"),
	1,
	TEXT("World position offset evaluation for cable meshes with EvaluateWPO enabled in ray tracing effects.\n")
	TEXT(" 0: Cable meshes with world position offset visible in ray tracing, WPO evaluation disabled.\n")
	TEXT(" 1: Cable meshes with world position offset visible in ray tracing, WPO evaluation enabled (default).\n")
);

static TAutoConsoleVariable<int32> CVarRayTracingCableMeshesWPOCulling(
	TEXT("r.RayTracing.Geometry.Cable.WPO.Culling"),
	1,
	TEXT("Enable culling for WPO evaluation for cable meshes in ray tracing (default = 1 (Culling enabled))"));

static TAutoConsoleVariable<float> CVarRayTracingCableMeshesWPOCullingRadius(
	TEXT("r.RayTracing.Geometry.Cable.WPO.CullingRadius"),
	12000.0f, // 120 m
	TEXT("Do not evaluate world position offset for cable meshes outside of this radius in ray tracing effects (default = 12000 (120m))"));

DECLARE_CYCLE_STAT(TEXT("Cable Sim"), STAT_Cable_SimTime, STATGROUP_JakubCablePhysic);
DECLARE_CYCLE_STAT(TEXT("Cable Solve"), STAT_Cable_SolveTime, STATGROUP_JakubCablePhysic);
DECLARE_CYCLE_STAT(TEXT("Cable Collision"), STAT_Cable_CollisionTime, STATGROUP_JakubCablePhysic);
DECLARE_CYCLE_STAT(TEXT("Cable Integrate"), STAT_Cable_IntegrateTime, STATGROUP_JakubCablePhysic);

static FName CableEndSocketName(TEXT("CableEnd"));
static FName CableStartSocketName(TEXT("CableStart"));

//////////////////////////////////////////////////////////////////////////

/** Index Buffer */
class FJakubCableIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		FRHIResourceCreateInfo CreateInfo(TEXT("FJakubCableIndexBuffer"));
		IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(int32), NumIndices * sizeof(int32), BUF_Dynamic, CreateInfo);
	}
	// POMNOZONO NumIndices RAZY 2 W CELU ZWIÊKSZENIA PAMIÊCI!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
	int32 NumIndices;
};

/** Dynamic data sent to render thread */
struct FCableDynamicData
{
	/** Array of points */
	TArray<FVector> CablePoints;
};

//////////////////////////////////////////////////////////////////////////
// FCableSceneProxy

class FJakubCableSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FJakubCableSceneProxy(UJakubCablePhysic* Component)
		: FPrimitiveSceneProxy(Component)
		, Material(NULL)
		, VertexFactory(GetScene().GetFeatureLevel(), "FJakubCableSceneProxy")
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, NumSegments(Component->NumSegments)
		, CableWidth(Component->CableWidth)
		, NumSides(Component->NumSides)
		, TileMaterial(Component->TileMaterial)
	{
		VertexBuffers.InitWithDummyData(&VertexFactory, GetRequiredVertexCount());

		IndexBuffer.NumIndices = GetRequiredIndexCount();

		// Enqueue initialization of render resource
		BeginInitResource(&IndexBuffer);

		// Grab material
		Material = Component->GetMaterial(0);
		if (Material == NULL)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}

#if RHI_RAYTRACING
		bSupportRayTracing = IsRayTracingEnabled();
		bDynamicRayTracingGeometry = false;
		bNeedsDynamicRayTracingGeometries = false;

		bNeedsToUpdateRayTracingCache = true;

		if (IsRayTracingAllowed() && bSupportRayTracing)
		{
			const bool bWantsRayTracingWPO = MaterialRelevance.bUsesWorldPositionOffset;

			if (bWantsRayTracingWPO)
			{
				bDynamicRayTracingGeometry = true;
				bNeedsDynamicRayTracingGeometries = true;
			}
		}
#endif

		ENQUEUE_RENDER_COMMAND(InitCableResources)(UE::RenderCommandPipe::JakubCable,
			[this](FRHICommandList& RHICmdList)
			{
				IndexBuffer.InitResource(RHICmdList);

#if RHI_RAYTRACING

				if (bSupportRayTracing)
				{
					FRayTracingGeometry& RayTracingGeometry = StaticRayTracingGeometry;
					UpdateRayTracingGeometry_RenderingThread(RayTracingGeometry, RHICmdList);
				}

				if (IsRayTracingAllowed() && bNeedsDynamicRayTracingGeometries)
				{
					check(bDynamicRayTracingGeometry);
					CreateDynamicRayTracingGeometries(RHICmdList);
				}
#endif
			});
	}

	virtual ~FJakubCableSceneProxy()
	{
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();

#if RHI_RAYTRACING
		StaticRayTracingGeometry.ReleaseResource();
		DynamicRayTracingGeometry.ReleaseResource();
#endif
	}

	int32 GetRequiredVertexCount() const
	{
		const int32 SegmentCount = NumSegments;
		const int32 NumRingVerts = NumSides + 1;

		// We generate two rings of vertices for each segment
		return SegmentCount * 2 * NumRingVerts;
	}

	int32 GetRequiredIndexCount() const
	{
		return (NumSegments * 2 * NumSides * 3) + ((NumSegments - 1) * NumSides * 2 * 3);
	}

	int32 GetVertIndex(int32 AlongIdx, int32 AroundIdx) const
	{
		return (AlongIdx * (NumSides + 1)) + AroundIdx;
	}

	void BuildCableMesh(const TArray<FVector>& InPoints, TArray<FDynamicMeshVertex>& OutVertices, TArray<int32>& OutIndices)
	{
		const FColor VertexColor(255, 255, 255);
		const int32 NumPoints = InPoints.Num();
		const int32 SegmentCount = NumPoints - 1;

		// Build vertices

		// We double up the first and last vert of the ring, because the UVs are different
		int32 NumRingVerts = NumSides + 1;

		// For each section along the spline..
		for (int32 SectionIdx = 0; SectionIdx < SegmentCount; SectionIdx++)
		{
			// Compute the direction vectors for this section
			FVector ForwardDir = (InPoints[SectionIdx + 1] - InPoints[SectionIdx]).GetSafeNormal();
			FQuat DeltaQuat = FQuat::FindBetween(FVector(0, 0, -1), ForwardDir);
			FVector RightDir = DeltaQuat.RotateVector(FVector(0, 1, 0));
			FVector UpDir = DeltaQuat.RotateVector(FVector(1, 0, 0));

			// Generate two rings of vertices for this section
			for (int32 RingIdx = 0; RingIdx < 2; RingIdx++)
			{
				// Compute the position of this ring along the section
				FVector RingPosition = FMath::Lerp(InPoints[SectionIdx], InPoints[SectionIdx + 1], (float)RingIdx);
				float AlongFrac = ((float)SectionIdx + (float)RingIdx) / (float)SegmentCount;

				// Generate the vertices for this ring
				for (int32 VertIdx = 0; VertIdx < NumRingVerts; VertIdx++)
				{
					const float AroundFrac = float(VertIdx) / float(NumSides);
					// Find angle around the ring
					const float RadAngle = 2.f * PI * AroundFrac;
					// Find direction from center of cable to this vertex
					const FVector OutDir = (FMath::Cos(RadAngle) * UpDir) + (FMath::Sin(RadAngle) * RightDir);

					FDynamicMeshVertex Vert;
					Vert.Position = FVector3f(RingPosition + (OutDir * 0.5f * CableWidth));
					Vert.TextureCoordinate[0] = FVector2f(AlongFrac * TileMaterial, AroundFrac);
					Vert.Color = VertexColor;
					Vert.SetTangents((FVector3f)ForwardDir, FVector3f(OutDir ^ ForwardDir), (FVector3f)OutDir);
					OutVertices.Add(Vert);
				}
			}
		}

		// Build triangles
		for (int32 SegIdx = 0; SegIdx < SegmentCount; SegIdx++)
		{
			for (int32 SideIdx = 0; SideIdx < NumSides; SideIdx++)
			{
				int32 TL = GetVertIndex(SegIdx * 2, SideIdx);
				int32 BL = GetVertIndex(SegIdx * 2, SideIdx + 1);
				int32 TR = GetVertIndex(SegIdx * 2 + 1, SideIdx);
				int32 BR = GetVertIndex(SegIdx * 2 + 1, SideIdx + 1);
				int32 TR2 = GetVertIndex((SegIdx + 1) * 2, SideIdx); // top right vertex of next segment
				int32 BR2 = GetVertIndex((SegIdx + 1) * 2, SideIdx + 1); // bottom right vertex of next segment

				OutIndices.Add(TL);
				OutIndices.Add(BL);
				OutIndices.Add(TR);

				OutIndices.Add(TR);
				OutIndices.Add(BL);
				OutIndices.Add(BR);

				// Connecting triangles
				if (SegIdx < SegmentCount - 1) {
					OutIndices.Add(TR);
					OutIndices.Add(BR);
					OutIndices.Add(TR2);

					OutIndices.Add(TR2);
					OutIndices.Add(BR);
					OutIndices.Add(BR2);
				}
			}
		}
	}

	/** Called on render thread to assign new dynamic data */
	void SetDynamicData_RenderThread(FCableDynamicData* NewDynamicData, FRHICommandListBase& RHICmdList)
	{
		check(IsInRenderingThread());

		if (NewDynamicData != nullptr)
		{
			// Build mesh from cable points
			TArray<FDynamicMeshVertex> Vertices;
			TArray<int32> Indices;
			BuildCableMesh(NewDynamicData->CablePoints, Vertices, Indices);

			check(Vertices.Num() == GetRequiredVertexCount());
			check(Indices.Num() == GetRequiredIndexCount());

			for (int i = 0; i < Vertices.Num(); i++)
			{
				const FDynamicMeshVertex& Vertex = Vertices[i];

				VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector3f(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector3f());
				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertex.TextureCoordinate[0]);
				VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
			}

			{
				auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
				RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
			}

			{
				auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
				RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
			}

			{
				auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
				RHICmdList.UnlockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
			}

			{
				auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
				void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
				RHICmdList.UnlockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
			}

			void* IndexBufferData = RHICmdList.LockBuffer(IndexBuffer.IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
			FMemory::Memcpy(IndexBufferData, &Indices[0], Indices.Num() * sizeof(int32));
			RHICmdList.UnlockBuffer(IndexBuffer.IndexBufferRHI);

#if RHI_RAYTRACING
			if (bSupportRayTracing && CVarRayTracingCableMeshes.GetValueOnRenderThread() != 0)
			{
				FRayTracingGeometry& RayTracingGeometry = StaticRayTracingGeometry;
				if (RayTracingGeometry.IsValid())
				{
					RayTracingGeometry.ReleaseResource();
					UpdateRayTracingGeometry_RenderingThread(RayTracingGeometry, RHICmdList);
					bNeedsToUpdateRayTracingCache = true;
				}
			}
#endif

			delete NewDynamicData;
			NewDynamicData = NULL;
		}
	}

	virtual void DrawStaticElements(FStaticPrimitiveDrawInterface* PDI) override
	{
		checkSlow(IsInParallelRenderingThread());

		if (!HasViewDependentDPG())
		{
			FMeshBatch Mesh;
			Mesh.VertexFactory = &VertexFactory;
			Mesh.MaterialRenderProxy = Material->GetRenderProxy();
			Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			Mesh.Type = PT_TriangleList;
			Mesh.DepthPriorityGroup = SDPG_World;
			Mesh.MeshIdInPrimitive = 0;
			Mesh.LODIndex = 0;
			Mesh.SegmentIndex = 0;

			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &IndexBuffer;
			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = GetRequiredIndexCount() / 3;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = GetRequiredVertexCount();

			PDI->DrawMesh(Mesh, FLT_MAX);
		}
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		//QUICK_SCOPE_CYCLE_COUNTER(STAT_CableSceneProxy_GetDynamicMeshElements);

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if (bWireframe)
		{
			MaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy();
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				// Draw the mesh.
				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;

				bool bHasPrecomputedVolumetricLightmap;
				FMatrix PreviousLocalToWorld;
				int32 SingleCaptureIndex;
				bool bOutputVelocity;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
				bOutputVelocity |= AlwaysHasVelocity();

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, bOutputVelocity, GetCustomPrimitiveData());
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = GetRequiredIndexCount() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = GetRequiredVertexCount();
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
				Collector.AddMesh(ViewIndex, Mesh);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				// Render bounds
				RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
#endif
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		const bool bAllowStaticLighting = IsStaticLightingAllowed();
		if (
#if !(UE_BUILD_SHIPPING) || WITH_EDITOR
			IsRichView(*View->Family) ||
			View->Family->EngineShowFlags.Collision ||
			View->Family->EngineShowFlags.Bounds ||
			View->Family->EngineShowFlags.VisualizeInstanceUpdates ||
#endif
#if WITH_EDITOR
			(IsSelected() && View->Family->EngineShowFlags.VertexColors) ||
			(IsSelected() && View->Family->EngineShowFlags.PhysicalMaterialMasks) ||
#endif
			// Force down dynamic rendering path if invalid lightmap settings, so we can apply an error material in DrawRichMesh
			(bAllowStaticLighting && HasStaticLighting() && !HasValidSettingsForStaticLighting()) ||
			HasViewDependentDPG()
			)
		{
			Result.bDynamicRelevance = true;
		}
		else
		{
			Result.bStaticRelevance = true;

#if WITH_EDITOR
			//only check these in the editor
			Result.bEditorVisualizeLevelInstanceRelevance = IsEditingLevelInstanceChild();
			Result.bEditorStaticSelectionRelevance = (IsSelected() || IsHovered());
#endif
		}

		MaterialRelevance.SetPrimitiveViewRelevance(Result);

		Result.bVelocityRelevance = DrawsVelocity() & Result.bOpaque & Result.bRenderInMainPass;

		return Result;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:

#if RHI_RAYTRACING
	virtual void GetDynamicRayTracingInstances(FRayTracingMaterialGatheringContext& Context, TArray<FRayTracingInstance>& OutRayTracingInstances) override
	{
		if (CVarRayTracingCableMeshes.GetValueOnRenderThread() == 0)
		{
			return;
		}

		if (!ensureMsgf(IsRayTracingRelevant(),
			TEXT("GetDynamicRayTracingInstances() is only expected to be called for scene proxies that are compatible with ray tracing. ")
			TEXT("RT-relevant primitive gathering code in FDeferredShadingSceneRenderer may be wrong.")))
		{
			return;
		}

		bool bEvaluateWPO = bDynamicRayTracingGeometry && CVarRayTracingCableMeshesWPO.GetValueOnRenderThread() == 1;

		if (bEvaluateWPO && CVarRayTracingCableMeshesWPOCulling.GetValueOnRenderThread() > 0)
		{
			const FVector ViewCenter = Context.ReferenceView->ViewMatrices.GetViewOrigin();
			const FVector MeshCenter = GetBounds().Origin;
			const float CullingRadius = CVarRayTracingCableMeshesWPOCullingRadius.GetValueOnRenderThread();
			const float BoundingRadius = GetBounds().SphereRadius;

			if (FVector(ViewCenter - MeshCenter).Size() > (CullingRadius + BoundingRadius))
			{
				bEvaluateWPO = false;
			}
		}

		if (!bEvaluateWPO)
		{
			if (!StaticRayTracingGeometry.IsValid())
			{
				return;
			}
		}

		FRayTracingGeometry& Geometry = bEvaluateWPO ? DynamicRayTracingGeometry : StaticRayTracingGeometry;

		if (Geometry.Initializer.TotalPrimitiveCount <= 0)
		{
			return;
		}

		FRayTracingInstance& RayTracingInstance = OutRayTracingInstances.AddDefaulted_GetRef();

		const int32 NumRayTracingMaterialEntries = 1;

		if (bNeedsToUpdateRayTracingCache)
		{
			CachedRayTracingMaterials.Reset();
			CachedRayTracingMaterials.Reserve(NumRayTracingMaterialEntries);

			FMeshBatch& MeshBatch = CachedRayTracingMaterials.AddDefaulted_GetRef();

			MeshBatch.VertexFactory = &VertexFactory;
			MeshBatch.MaterialRenderProxy = Material->GetRenderProxy();
			MeshBatch.SegmentIndex = 0;
			MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
			MeshBatch.Type = PT_TriangleList;
			MeshBatch.DepthPriorityGroup = SDPG_World;
			MeshBatch.bCanApplyViewModeOverrides = false;
			MeshBatch.CastRayTracedShadow = IsShadowCast(Context.ReferenceView);
			MeshBatch.DepthPriorityGroup = GetStaticDepthPriorityGroup();

			FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
			BatchElement.IndexBuffer = &IndexBuffer;

			bool bHasPrecomputedVolumetricLightmap;
			FMatrix PreviousLocalToWorld;
			int32 SingleCaptureIndex;
			bool bOutputVelocity;
			GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
			bOutputVelocity |= AlwaysHasVelocity();

			FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Context.RayTracingMeshResourceCollector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
			DynamicPrimitiveUniformBuffer.Set(Context.RHICmdList, GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), GetLocalBounds(), ReceivesDecals(), bHasPrecomputedVolumetricLightmap, bOutputVelocity, GetCustomPrimitiveData());
			BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = GetRequiredIndexCount() / 3;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = GetRequiredVertexCount();


			RayTracingInstance.MaterialsView = MakeArrayView(CachedRayTracingMaterials);
			bNeedsToUpdateRayTracingCache = false;
		}
		else
		{
			RayTracingInstance.MaterialsView = MakeArrayView(CachedRayTracingMaterials);

			// Skip computing the mask and flags in the renderer since we are using cached values.
			RayTracingInstance.bInstanceMaskAndFlagsDirty = false;
		}

		RayTracingInstance.Geometry = &Geometry;
		const FMatrix& ThisLocalToWorld = GetLocalToWorld();
		RayTracingInstance.InstanceTransformsView = MakeArrayView(&ThisLocalToWorld, 1);

		if (bEvaluateWPO && VertexFactory.GetType()->SupportsRayTracingDynamicGeometry())
		{
			// Use the shared vertex buffer - needs to be updated every frame
			FRWBuffer* VertexBuffer = nullptr;

			const uint32 VertexCount = VertexBuffers.PositionVertexBuffer.GetNumVertices() + 1;

			Context.DynamicRayTracingGeometriesToUpdate.Add(
				FRayTracingDynamicGeometryUpdateParams
				{
					CachedRayTracingMaterials, // TODO: this copy can be avoided if FRayTracingDynamicGeometryUpdateParams supported array views
					false,
					(uint32)VertexCount,
					uint32((SIZE_T)VertexCount * sizeof(FVector3f)),
					Geometry.Initializer.TotalPrimitiveCount,
					&Geometry,
					VertexBuffer,
					true
				}
			);
		}

		check(CachedRayTracingMaterials.Num() == RayTracingInstance.GetMaterials().Num());
		checkf(RayTracingInstance.Geometry->Initializer.Segments.Num() == CachedRayTracingMaterials.Num(), TEXT("Segments/Materials mismatch. Number of segments: %d. Number of Materials: %d."),
			RayTracingInstance.Geometry->Initializer.Segments.Num(),
			CachedRayTracingMaterials.Num());
	}

	virtual bool HasRayTracingRepresentation() const override { return bSupportRayTracing; }
	virtual bool IsRayTracingRelevant() const override { return true; }
	virtual bool IsRayTracingStaticRelevant() const override { return false; }

	void UpdateRayTracingGeometry_RenderingThread(FRayTracingGeometry& RayTracingGeometry, FRHICommandListBase& RHICmdList)
	{
		FRayTracingGeometryInitializer Initializer;
		static const FName DebugName("FCableSceneProxy");
		static int32 DebugNumber = 0;
		Initializer.DebugName = FDebugName(DebugName, DebugNumber++);
		Initializer.IndexBuffer = IndexBuffer.IndexBufferRHI;
		Initializer.TotalPrimitiveCount = IndexBuffer.NumIndices / 3;
		Initializer.GeometryType = RTGT_Triangles;
		Initializer.bFastBuild = true;
		Initializer.bAllowUpdate = false;

		FRayTracingGeometrySegment Segment;
		Segment.VertexBuffer = VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
		Segment.NumPrimitives = Initializer.TotalPrimitiveCount;
		Segment.MaxVertices = VertexBuffers.PositionVertexBuffer.GetNumVertices();
		Initializer.Segments.Add(Segment);

		RayTracingGeometry.SetInitializer(Initializer);
		RayTracingGeometry.InitResource(RHICmdList);
	}
#endif

private:

	UMaterialInterface* Material;
	FStaticMeshVertexBuffers VertexBuffers;
	FJakubCableIndexBuffer IndexBuffer;
	FLocalVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;

	int32 NumSegments;

	float CableWidth;

	int32 NumSides;

	float TileMaterial;

#if RHI_RAYTRACING
	void CreateDynamicRayTracingGeometries(FRHICommandListBase& RHICmdList)
	{
		FRayTracingGeometryInitializer Initializer = StaticRayTracingGeometry.Initializer;
		for (FRayTracingGeometrySegment& Segment : Initializer.Segments)
		{
			Segment.VertexBuffer = nullptr;
		}
		Initializer.bAllowUpdate = true;
		Initializer.bFastBuild = true;
		Initializer.Type = ERayTracingGeometryInitializerType::Rendering;

		DynamicRayTracingGeometry.SetInitializer(MoveTemp(Initializer));
		DynamicRayTracingGeometry.InitResource(RHICmdList);
	}

	bool bSupportRayTracing : 1;
	bool bDynamicRayTracingGeometry : 1;
	bool bNeedsDynamicRayTracingGeometries : 1;
	bool bNeedsToUpdateRayTracingCache : 1;

	FRayTracingGeometry StaticRayTracingGeometry;
	FRayTracingGeometry DynamicRayTracingGeometry;
	TArray<FMeshBatch> CachedRayTracingMaterials;
#endif
};



// [][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][] //
// [][][][][][][][][][][][][][]  Matematyczny model liny - dzialania na sprezynach [][][][][][][][][][][][][][] //
// [][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][] //

UJakubCablePhysic::UJakubCablePhysic(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
	bAttachStart = true;
	CableWidth = 4.f;
	NumSegments = 10;
	NumSides = 4;
	EndLocation = FVector(0, 0, -100.f);
	CableLength = 100.f;
	SubstepTime = 0.015f;
	SolverIterations = 2;
	DampingFactorRange = FVector2D(1.f, 0.99);
	DampingByAcceleration = 0.f;
	TileMaterial = 1.f;
	CollisionFriction = 1.f;
	CollisionSubSegments = 10;
	OptymalizeSubSegments = false;
	CableGravityScale = 1.f;
	bUseUpdatingLODs = false;
	LodsRadiusRangeMultipler = 1.0;

	SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
}

FPrimitiveSceneProxy* UJakubCablePhysic::CreateSceneProxy()
{
	return new FJakubCableSceneProxy(this);
}

int32 UJakubCablePhysic::GetNumMaterials() const
{
	return 1;
}

/* |||||||||||||||||||||||||||||||||||||  C A B L E   I N I T I A L I Z A T I O N   |||||||||||||||||||||||||||||||||||*/
void UJakubCablePhysic::OnRegister()
{
	Super::OnRegister();
	// Save Init SubstepTime to Variable
	DefSubstepTime = SubstepTime;
	const int32 NumParticles = NumSegments + 1;
	// ustaw w tablicy zadeklarowana ilosc czastek
	Particles.Reset();
	Particles.AddUninitialized(NumParticles);
	// definicja postawowych wektorow
	FVector CableStart, CableEnd;
	GetStartPosition(CableStart);
	CableEnd = CableStart + EndLocation;
	// oblicz dlogosc liny
	const FVector Delta = CableEnd - CableStart;
	// Sprawdz czy jakas czastka znajduje ma zostac ograniczona utwierdzeniem. Znajdz indeksy tych czastek
	TArray<int> NotFreePoints;
	for (int i = 0; i < AttachToArray.Num(); i++)
	{ NotFreePoints.AddUnique(AttachToArray[i].ParticleIndex); }
	// ustaw pozycje inicjujaca dla kazdej czastki
	for (int32 ParticleIdx = 0; ParticleIdx < NumParticles; ParticleIdx++)
	{
		FCableSingleParticle& Particle = Particles[ParticleIdx];
		const float Alpha = (float)ParticleIdx / (float)NumSegments;
		const FVector InitialPosition = CableStart + (Alpha * Delta);
		Particle.Position = InitialPosition;
		Particle.OldPosition = InitialPosition;
		Particle.OldVelocity = FVector(0, 0, 0);
		Particle.TargetLocation = FVector(0, 0, 0);
		if (NotFreePoints.Find(ParticleIdx) != -1)
		{ Particle.bFree = false; }
		else
		{ Particle.bFree = true; }
	}
	//Calculate Segments Length
	SegmentsLength.SetNum(0);
	if (NotFreePoints.Num() > 0)
	{
		NotFreePoints.Add(0);
		NotFreePoints.Sort();
		float PointsDiff = 0.0;
		for (int ii = 0; ii < NotFreePoints.Num() - 1; ii++)
		{
			PointsDiff = 1.0 * FMath::Clamp<int>(NotFreePoints[ii + 1] - NotFreePoints[ii], 0, Particles.Num());
			SegmentsLength.Add(PointsDiff * (CableLength / (float)NumSegments));
		}
	}
	else
	{ SegmentsLength.Add(CableLength); }
}

//Simple Refres Particles Free State
void UJakubCablePhysic::RefreshParticlesIsFree()
{
	TArray<int> NotFreePoints;
	for (int i = 0; i < AttachToArray.Num(); i++)
	{ NotFreePoints.AddUnique(AttachToArray[i].ParticleIndex); }
	for (int32 ParticleIdx = 0; ParticleIdx < Particles.Num(); ParticleIdx++)
	{
		FCableSingleParticle& Particle = Particles[ParticleIdx];
		if (NotFreePoints.Find(ParticleIdx) != -1)
		{ Particle.bFree = false; }
		else
		{ Particle.bFree = true; }
	}
	//Calculate Segments Length
	SegmentsLength.SetNum(0);
	if (NotFreePoints.Num() > 0)
	{
		NotFreePoints.Add(0);
		NotFreePoints.Sort();
		float PointsDiff = 0.0;
		for (int ii = 0; ii < NotFreePoints.Num()-1; ii++)
		{
			PointsDiff = 1.0 * FMath::Clamp<int>(NotFreePoints[ii + 1] - NotFreePoints[ii], 0, Particles.Num());
			SegmentsLength.Add(PointsDiff * (CableLength / (float)NumSegments));
		}
	}
	else
	{ SegmentsLength.Add(CableLength); }
}

void UJakubCablePhysic::VerletIntegrate(float InSubstepTime, const FVector& Gravity)
{
	//SCOPE_CYCLE_COUNTER(STAT_Cable_IntegrateTime);
	UWorld* World = GetWorld();
	const int32 NumParticles = NumSegments + 1;
	const float SubstepTimeSqr = InSubstepTime * InSubstepTime;
	//GEngine->AddOnScreenDebugMessage(-1, InSubstepTime, FColor::Red, (Gravity* SubstepTimeSqr).ToString());
	
	for (int32 ParticleIdx = 0; ParticleIdx < NumParticles; ParticleIdx++)
	{
		FCableSingleParticle& Particle = Particles[ParticleIdx];
		if (Particle.bFree && Particle.TargetLocation.IsNearlyZero(0.01))
		{
			// Calculate overall force
			FVector ParticleForce = CableForce + Gravity;
			if (LodCurrentState >= 3 && bUseUpdatingLODs == true)
			{ ParticleForce = ParticleForce * 0.1; }
			else if (LodCurrentState >= 2 && bUseUpdatingLODs == true)
			{ ParticleForce = ParticleForce * 0.8; }

			// Calculate Damping Factor for current Particle
			float DampingValue = FMath::GetMappedRangeValueClamped(FVector2D(0, NumParticles * 1.0), DampingFactorRange, ParticleIdx * 1.0);
			if (LodCurrentState > 1 && bUseUpdatingLODs == true)
			{ DampingValue = DampingValue * 0.75; }

			// Calculate velocity
			const FVector Vel = (Particle.Position - Particle.OldPosition);

			// Calculate Acceleration (Only When PlayTime > 0.1, saved velocity is bigger that 0 and LOD state is lower or equal 1) EXPERIMENTAL!!!
			FVector Acc = FVector(0, 0, 0);
			if (Particle.OldVelocity.Size() > 0.002 && LodCurrentState <= 1) { Acc = Vel - Particle.OldVelocity; }
			Particle.OldVelocity = Vel;

			//Apply Aditive Force To Particle But only when Index Is Valid
			FVector AdditiveForce = FVector(0, 0, 0);
			if (AdditiveForces.IsValidIndex(ParticleIdx) == true) { AdditiveForce = AdditiveForces[ParticleIdx] * SubstepTimeSqr; }

			// Update position         | Pozycja Cz¹stki  + Dodanie Prêdkoœci    + Dodanie si³y grawitacji          - Odjêcie Wartoœci przyœpieszenia + Dodatkow sila dla wybranej cz¹stki
			const FVector NewPosition = Particle.Position + (Vel * DampingValue) + (SubstepTimeSqr * ParticleForce) - (Acc * DampingByAcceleration) + AdditiveForce;

			// Save current Particle Position for next Frame Gen
			Particle.OldPosition = Particle.Position;

			// Apply New Position As Current Particle Position
			Particle.Position = NewPosition;
		}
	}
}

/** Solve a single distance constraint between a pair of particles */
static FORCEINLINE void SolveDistanceConstraint(FCableSingleParticle& ParticleA, FCableSingleParticle& ParticleB, float DesiredDistance)
{
	// Find current vector between particles
	FVector Delta = ParticleB.Position - ParticleA.Position;
	float CurrentDistance = Delta.Size();
	bool bNormalizedOK = Delta.Normalize();
	// If particles are right on top of each other, separate with an abitrarily-chosen direction
	FVector CorrectionDirection = bNormalizedOK ? Delta : FVector{ 1, 0, 0 };
	FVector VectorCorrection = (CurrentDistance - DesiredDistance) * CorrectionDirection;

	// Only move free particles to satisfy constraints
	if (ParticleA.bFree && ParticleB.bFree)
	{
		ParticleA.Position += 0.5f * VectorCorrection;
		ParticleB.Position -= 0.5f * VectorCorrection;
	}
	else if (ParticleA.bFree)
	{
		ParticleA.Position += VectorCorrection;
	}
	else if (ParticleB.bFree)
	{
		ParticleB.Position -= VectorCorrection;
	}
}

void UJakubCablePhysic::SolveConstraints()
{
	//SCOPE_CYCLE_COUNTER(STAT_Cable_SolveTime);

	const float SegmentLength = CableLength / (float)NumSegments;
	UWorld* World = GetWorld();
	// For each iteration..
	for (int32 IterationIdx = 0; IterationIdx < SolverIterations; IterationIdx++)
	{
		CurrentSegmentsLength.SetNum(0); //Delete All Elements In Array
		float DistBetween, DistancesSum = 0.0;
		// Solve distance constraint for each segment
		for (int32 SegIdx = 0; SegIdx < NumSegments; SegIdx++)
		{
			FCableSingleParticle& ParticleA = Particles[SegIdx];
			FCableSingleParticle& ParticleB = Particles[SegIdx + 1];
			//Save Particle Position Befor Solve Constraint 
			FVector BeforSolvePosition = ParticleA.Position;
			DistBetween = FVector::Distance(ParticleA.Position, ParticleB.Position);
			// Solve for this pair of particles
			if (ParticleB.TargetLocation.IsNearlyZero(0.01))
			{
				SolveDistanceConstraint(ParticleA, ParticleB, SegmentLength);
			}
			else
			{
				ParticleB.Position = ParticleB.TargetLocation; 
				ParticleB.OldPosition = ParticleB.TargetLocation; 
				ParticleB.TargetLocation = FVector(0, 0, 0);
			}

			DistancesSum = DistancesSum + DistBetween;
			if (ParticleB.bFree == false)
			{
				CurrentSegmentsLength.Add(DistancesSum);
				DistancesSum = 0.0;
			}
			LastIndexOldPosition = ParticleB.Position - BeforSolvePosition;
		}

		// If desired, solve stiffness constraints (distance constraints between every other particle)
		if (bEnableStiffness)
		{
			for (int32 SegIdx = 0; SegIdx < NumSegments - 1; SegIdx++)
			{
				FCableSingleParticle& ParticleA = Particles[SegIdx];
				FCableSingleParticle& ParticleB = Particles[SegIdx + 2];
				SolveDistanceConstraint(ParticleA, ParticleB, 2 * SegmentLength);
			}
		}
	}
	//GEngine->AddOnScreenDebugMessage(-1, SubstepTime, FColor::Green, FVector(AvgStretchValue, CableLength, (ParticleA.Position - ParticleB.Position).Size()).ToString());
}
// ------------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------   S O L V I N G   C O L L I S I O N   -------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------

// Nowa funkcja, która sprawdza kolizje dla wirtualnej cz¹stki
bool UJakubCablePhysic::CheckSubSegmentCollision(FCableSingleParticle& ParticleA, FCableSingleParticle& ParticleB, float Alpha)
{
	UWorld* World = GetWorld();
	// If we have a world, and collision is not disabled
	if (World && GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CableCollision));
		ECollisionChannel TraceChannel = GetCollisionObjectType();
		FCollisionResponseParams ResponseParams(GetCollisionResponseToChannels());
		Params.AddIgnoredActors(ActorsToIgnoreBySweep);

		FVector Position = FMath::Lerp(ParticleA.Position, ParticleB.Position, Alpha);
		FVector OldPosition = FMath::Lerp(ParticleA.OldPosition, ParticleB.OldPosition, Alpha);

		FHitResult Result;

		bool bHit = World->SweepSingleByChannel(Result, OldPosition, Position, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(0.5 * CableWidth), Params, ResponseParams);

		if (bHit)
		{
			// W przypadku kolizji, wykonujemy korektê na ParticleA i ParticleB, proporcjonalnie do Alpha.
			FVector Correction = (Result.bStartPenetrating) ? Result.Normal * Result.PenetrationDepth : Result.Location - Position;
			ParticleA.Position += (1.f - Alpha) * Correction;
			ParticleB.Position += Alpha * Correction;

			// I podobnie dla OldPosition
			FVector Delta = Position - OldPosition;
			float NormalDelta = Delta | Result.Normal;
			FVector PlaneDelta = Delta - (NormalDelta * Result.Normal);
			ParticleA.OldPosition += (1.f - Alpha) * (NormalDelta * Result.Normal);
			ParticleB.OldPosition += Alpha * (NormalDelta * Result.Normal);

			// Odpowiednia korekcja dla komponentu stycznej
			if (CollisionFriction > KINDA_SMALL_NUMBER)
			{
				FVector ScaledPlaneDelta = PlaneDelta * CollisionFriction;
				ParticleA.OldPosition += (1.f - Alpha) * ScaledPlaneDelta;
				ParticleB.OldPosition += Alpha * ScaledPlaneDelta;
			}

			return true;
		}
	}
	return false;
}

void UJakubCablePhysic::PerformCableCollision()
{
	SCOPE_CYCLE_COUNTER(STAT_Cable_CollisionTime);

	// Dla ka¿dej pary cz¹stek
	for (int32 SegIdx = 0; SegIdx < NumSegments; SegIdx++)
	{
		FCableSingleParticle& ParticleA = Particles[SegIdx];
		FCableSingleParticle& ParticleB = Particles[SegIdx + 1];

		if (OptymalizeSubSegments == false)
		{
			// SprawdŸ dodatkowe kolizje miêdzy cz¹stkami
			int32 NumSubSegments = CollisionSubSegments;
			// Ustaw stan dla informacji dotyczacej czy dana czastka styka sie z kolizja. Zresetuj wartosc
			bool HitCollision = false;
			ParticleA.bIsColliding = false;
			// zaczynamy od 1 i koñczymy przed ostatnim punktem, poniewa¿ prawdziwe cz¹stki s¹ ju¿ sprawdzane
			for (int32 SubSegIdx = 1; SubSegIdx < NumSubSegments; SubSegIdx++) 
			{
				float Alpha = (float)SubSegIdx / (float)NumSubSegments;
				HitCollision = CheckSubSegmentCollision(ParticleA, ParticleB, Alpha);
				if (HitCollision == true) ParticleA.bIsColliding = true;
			}
		}
		else
		{
			UWorld* World = GetWorld();
			FCollisionQueryParams Params(SCENE_QUERY_STAT(CableCollision));

			// Ustaw stan dla informacji dotyczacej czy dana czastka styka sie z kolizja. Zresetuj wartosc
			bool HitCollision = false;
			ParticleA.bIsColliding = false;
			ParticleB.bIsColliding = false;

			ECollisionChannel TraceChannel = GetCollisionObjectType();
			FCollisionResponseParams ResponseParams(GetCollisionResponseToChannels());
			Params.AddIgnoredActors(ActorsToIgnoreBySweep);
			FHitResult Result;
			bool bHit = World->SweepSingleByChannel(Result, ParticleA.Position, ParticleB.Position, FQuat::Identity, 
			TraceChannel, FCollisionShape::MakeSphere(FMath::Clamp<float>(0.5f * CableWidth*5,0.5, CableLength/NumSegments)), Params, ResponseParams);
			if (bHit == true)
			{
				// SprawdŸ dodatkowe kolizje miêdzy cz¹stkami
				int32 NumSubSegments = CollisionSubSegments;

				// zaczynamy od 1 i koñczymy przed ostatnim punktem, poniewa¿ prawdziwe cz¹stki s¹ ju¿ sprawdzane
				for (int32 SubSegIdx = 1; SubSegIdx < NumSubSegments; SubSegIdx++)
				{
					float Alpha = (float)SubSegIdx / (float)NumSubSegments;
					HitCollision = CheckSubSegmentCollision(ParticleA, ParticleB, Alpha);
					if (HitCollision == true) ParticleA.bIsColliding = true;
				}


			}

		}
	}
}

// ___________________ TICK FOR SOLVING ROPE BEHAVIOR _____________________________
void UJakubCablePhysic::PerformSubstep(float InSubstepTime, const FVector& Gravity)
{
	//SCOPE_CYCLE_COUNTER(STAT_Cable_SimTime);
	// -----> Varel Integrate <-----
	VerletIntegrate(InSubstepTime, Gravity);
	// -----> Solve Constraint <-----
	SolveConstraints();
	// -----> Perform Collision <-----
	if (bEnableCollision)
	{
		PerformCableCollision();
	}
}

// >>>>>>>>>>>>> DEFINE BASIC FUNCTION THAT CAN BE CALLED FROM BLUEPRINT <<<<<<<<<<<<<<
void UJakubCablePhysic::SetAttachIndexToComponent(USceneComponent* Component, FName SocketName, int ParticleIndex)
{
	int PIndex = FMath::Clamp<int>(ParticleIndex, 1, Particles.Num() - 1);

	for (int i = 0; i < AttachToArray.Num(); i++)
	{
		if (AttachToArray[i].ParticleIndex == PIndex)
		{
			AttachToArray[i].ParticleIndex = PIndex;
			AttachToArray[i].ComponentRef.OtherActor = Component ? Component->GetOwner() : nullptr;
			AttachToArray[i].ComponentRef.ComponentProperty = NAME_None;
			AttachToArray[i].ComponentRef.OverrideComponent = Component;
			AttachToArray[i].SocketName = SocketName;
			RefreshParticlesIsFree();
			return;
		}
	}
	FCableAttachPoint NewAttach = {};
	NewAttach.ParticleIndex = PIndex;
	NewAttach.ComponentRef.OtherActor = Component ? Component->GetOwner() : nullptr;
	NewAttach.ComponentRef.ComponentProperty = NAME_None;
	NewAttach.ComponentRef.OverrideComponent = Component;
	NewAttach.SocketName = SocketName;
	AttachToArray.Add(NewAttach);
	RefreshParticlesIsFree();
	return;
}

void UJakubCablePhysic::SetAttachIndexToActor(AActor* Actor, FName ComponentProperty, FName SocketName, int ParticleIndex)
{
	int PIndex = FMath::Clamp<int>(ParticleIndex, 1, Particles.Num() - 1);

	for (int i = 0; i < AttachToArray.Num(); i++)
	{
		if (AttachToArray[i].ParticleIndex == PIndex)
		{
			AttachToArray[i].ParticleIndex = PIndex;
			AttachToArray[i].ComponentRef.OtherActor = Actor;
			AttachToArray[i].ComponentRef.ComponentProperty = ComponentProperty;
			AttachToArray[i].SocketName = SocketName;
			RefreshParticlesIsFree();
			return;
		}
	}
	FCableAttachPoint NewAttach = {};
	NewAttach.ParticleIndex = PIndex;
	NewAttach.ComponentRef.OtherActor = Actor;
	NewAttach.ComponentRef.ComponentProperty = ComponentProperty;
	NewAttach.SocketName = SocketName;
	AttachToArray.Add(NewAttach);
	RefreshParticlesIsFree();
	return;
}

TArray<AActor*> UJakubCablePhysic::GetAttachedActors() const
{
	TArray<AActor*> ActorsList;
	for (int i = 0; i < AttachToArray.Num(); i++)
	{
		if (IsValid(AttachToArray[i].ComponentRef.OtherActor.Get()) == true)
		{
			ActorsList.AddUnique(AttachToArray[i].ComponentRef.OtherActor.Get());
		}
	}
	return ActorsList;
}

TArray<FCableAttachPoint> UJakubCablePhysic::GetAttachStructure() const
{
	return AttachToArray;
}

bool UJakubCablePhysic::DetachParticleFrom(int ParticleIndex)
{
	int PIndex = FMath::Clamp<int>(ParticleIndex, 1, Particles.Num() - 1);
	for (int i = 0; i < AttachToArray.Num(); i++)
	{
		if (AttachToArray[i].ParticleIndex == PIndex)
		{
			AttachToArray.RemoveAt(i);
			RefreshParticlesIsFree();
			return true;
		}
	}
	return false;
}

void UJakubCablePhysic::GetCableParticleLocations(TArray<FVector>& Locations) const
{
	Locations.Empty();
	for (const FCableSingleParticle& Particle : Particles)
	{
		Locations.Add(Particle.Position);
	}
}

TArray<FExposedCableParticle> UJakubCablePhysic::GetCableParticlesStructure() const
{
	TArray<FExposedCableParticle> LocalStruct;
	for (const FCableSingleParticle& Particle : Particles)
	{
		FExposedCableParticle LocalSingleData;
		LocalSingleData.Position = Particle.Position;
		LocalSingleData.OldPosition = Particle.OldPosition;
		LocalSingleData.bIsColliding = Particle.bIsColliding;
		LocalSingleData.bIsFree = Particle.bFree;
		LocalStruct.Add(LocalSingleData);
	}
	return LocalStruct;
}

TArray<float> UJakubCablePhysic::GetCableInitSegmentsLength() const
{
	return SegmentsLength;
}

TArray<float> UJakubCablePhysic::GetCableCurrentSegmentsLength() const
{
	return CurrentSegmentsLength;
}

TArray<int> UJakubCablePhysic::GetIndicesOfAttachedPoints() const
{
	TArray<FCableAttachPoint> s = GetAttachStructure();
	TArray<int> AttachIndices;

	if (bAttachStart == true)
	{ AttachIndices.Add(0); }

	if (s.Num() == 0)
	{ return AttachIndices; }

	for (const FCableAttachPoint& CurrentPoint : s)
	{
		AttachIndices.Add(CurrentPoint.ParticleIndex);
	}
	AttachIndices.Sort();
	return AttachIndices;
}

bool UJakubCablePhysic::BreakAttachIfSegmentIsStretched(int& ParticleIndex, float StretchTolerance)
{
	TArray<float> InitLengths = GetCableInitSegmentsLength();
	// if initialization cable segments length is not valid, then finish function
	if (InitLengths.Num() == 0)
	{ ParticleIndex = -1; return false; }
	// Save current cable segments length to local variable
	TArray<float> CurrentLengths = GetCableCurrentSegmentsLength();
	// Make sure the size of two arrays is same. If not finish function
	if (InitLengths.Num() != CurrentLengths.Num())
	{ ParticleIndex = -1; return false; }
	//Run loop
	for (int32 i = 0; i < GetCableCurrentSegmentsLength().Num(); i++)
	{
		float L = CurrentLengths[i] - InitLengths[i];
		L = L / ((InitLengths[i] * NumSegments) / CableLength);
		// If the L variable is exceeds tolerance  value detach particle from constraint
		if (L > StretchTolerance)
		{
			TArray<int> AttachIndices = GetIndicesOfAttachedPoints();
			int DetachIndex = AttachIndices[FMath::Clamp<int>(i + 1, 0, AttachIndices.Num() - 1)];
			DetachParticleFrom(DetachIndex);
			ParticleIndex = DetachIndex;
			return true;
		}
	}
	ParticleIndex = -1;
	return false;
}

bool UJakubCablePhysic::GetAnyPointIsColliding(int RangeMin, int RangeMax) const
{
	const int CorrectRange = FMath::Clamp<int>(RangeMax, 0, Particles.Num() - 1);
	for (int i = RangeMin; i <= CorrectRange; i++)
	{
		if (Particles[i].bIsColliding == true)
		{ return true; }
	}
	return false;
}

void UJakubCablePhysic::UseForceToTightenTheRope(int EndIndex, int StartIndex, float ForceStrenght, bool ApplyDistanceForce)
{
	int CorrectStartIndex = StartIndex;
	if (StartIndex < 0)
	{ CorrectStartIndex = Particles.Num() - 1; }

	for (int i = CorrectStartIndex; i > EndIndex; i--)
	{
		FVector Direction = Particles[i].Position - Particles[i - 1].Position;
		Direction.Normalize();

		FVector FinalForce = FVector::Zero();

		if (ApplyDistanceForce == true)
		{ FinalForce = Direction * (Particles[i].Position - Particles[i - 1].Position).Length() * ForceStrenght; }
		else
		{ FinalForce = Direction * ForceStrenght; }

		if(Particles[i].bFree == true) { ApplyConstForceToParticle(FinalForce, i, false, 2, 1); }
		
	}
}

bool UJakubCablePhysic::ReduceForceForParticles(int StartIndex, int EndIndex, float ReductionStrenght, float dt)
{
	int CorrectStartIndex = FMath::Clamp<int>(EndIndex,0,AdditiveForces.Num()-1);
	if (EndIndex < 0)
	{ CorrectStartIndex = AdditiveForces.Num() - 1; }

	if (dt <= 0) { return false; }
	if (AdditiveForces.Num() == 0) { return false; }

	for (int i = StartIndex; i <= CorrectStartIndex; i++)
	{
		if (AdditiveForces[i].Length() < 1.f) { AdditiveForces[i] = FVector(0, 0, 0); }
		else
		{
			AdditiveForces[i] = FMath::Lerp<FVector>(AdditiveForces[i], FVector(0, 0, 0), dt * ReductionStrenght);
		}
		
	}
	return true;
}

void UJakubCablePhysic::UpdateSubstepTime(float NewSubstepTime, bool FixVelocity)
{
	SubstepTime = FMath::Max(NewSubstepTime, 0.003f);
	if (FixVelocity == false)
	{ return; }
	for (int32 i = 0; i < Particles.Num(); i++)
	{
		if (Particles[i].bFree == true)
		{
			Particles[i].OldPosition = Particles[i].Position;
		}
	}
}

void UJakubCablePhysic::AddImpulseToParticle(FVector Impulse, int ParticleIndex, bool UseDistributedForce, int UnfoldingRange)
{
	int PIndex = FMath::Clamp<int>(ParticleIndex, 0, Particles.Num() - 1);
	// Pobierz cz¹stkê
	FCableSingleParticle& Particle = Particles[PIndex];
	if (Particle.bFree == false)
	{ return; }
	// Oblicz przybli¿on¹ prêdkoœæ cz¹stki
	FVector Velocity = Particle.Position - Particle.OldPosition;
	// Dodaj impuls do prêdkoœci cz¹stki
	Velocity += Impulse;
	// Aktualizuj pozycje cz¹stki
	Particle.OldPosition = Particle.Position;
	Particle.Position += Velocity;
	int DistRange = UnfoldingRange;
	if (UseDistributedForce == true)
	{
		for (int i = 1; i <= DistRange; i++)
		{
			if ((PIndex - i) > 0 && (PIndex - i) < Particles.Num())
			{
				Velocity = Particles[PIndex - i].Position - Particles[PIndex - i].OldPosition;
				Velocity += Impulse / (i + 1);
				Particles[PIndex - i].OldPosition = Particles[PIndex - i].Position;
				Particles[PIndex - i].Position = Particles[PIndex - i].Position + Velocity;
			}
			if ((PIndex + i) > 0 && (PIndex + i) < Particles.Num())
			{
				Velocity = Particles[PIndex + i].Position - Particles[PIndex + i].OldPosition;
				Velocity += Impulse / (i + 1);
				Particles[PIndex + i].OldPosition = Particles[PIndex + i].Position;
				Particles[PIndex + i].Position = Particles[PIndex + i].Position + Velocity;
			}
		}
	}
	return;
}

bool UJakubCablePhysic::ApplyConstForceToParticle(FVector Force, int ParticleIndex, bool ExpadLinear, int ExpandLenght, float ExpandMinRange)
{
	if (AdditiveForces.Num() == 0)
	{
		for (const FCableSingleParticle& Particle : Particles)
		{
			AdditiveForces.Add(FVector(0, 0, 0));
		}
		return false;
	}
	else
	{
		int ValidIndex = FMath::Clamp<int>(ParticleIndex,1,Particles.Num()-1);
		AdditiveForces[ValidIndex] = Force;

		if (ExpadLinear == true)
		{
			for (int i = 1; i <= ExpandLenght; i++)
			{
				if (ValidIndex + i < Particles.Num()) 
				{ 
					const float ScalingFactor = FMath::GetMappedRangeValueClamped(FVector2D(1, ExpandLenght), FVector2D(0.95, ExpandMinRange), i * 1.0);
					AdditiveForces[ValidIndex + i] = Force / (ScalingFactor + 0.05);
				}
				else { break; }
			}
			for (int i = 1; i <= ExpandLenght; i++)
			{
				if (ValidIndex - i >= 0)
				{
					const float ScalingFactor = FMath::GetMappedRangeValueClamped(FVector2D(1, ExpandLenght), FVector2D(0.95, ExpandMinRange), i * 1.0);
					AdditiveForces[ValidIndex - i] = Force / (ScalingFactor + 0.05);
				}
				else { break; }
			}
		}

		return true;
	}
}

bool UJakubCablePhysic::GetCableSpaceLenghtForSwinging(int& ParticlesCount, FExposedCableParticle& FirstParticle, FExposedCableParticle& LastParticle, 
	TArray<FExposedCableParticle>& ParticlesInSpace, bool DrawDebug)
{
	TArray<FExposedCableParticle> ParticlesData;

	for (int i = Particles.Num() - 1; i >= 0; i--) //Reversed Loop
	{
		ParticlesData.Add(PrivateToExposedStructure(Particles[i]));
		//GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Green, FVector(i, CableLength, 1.0).ToString());
		if (i < (Particles.Num() - 1)) {
		if (Particles[i].bIsColliding == true || Particles[i].bFree == false)
		{
			ParticlesCount = Particles.Num() - 1 - i;
			ParticlesInSpace = ParticlesData;
			if (ParticlesData.Num() > 0) LastParticle = ParticlesData[0]; FirstParticle = ParticlesData[ParticlesData.Num() - 1];
			break;
		}}
	}
	if (ParticlesData.Num() > 0)
	{
		if (DrawDebug == true)
		{ UWorld* World = GetWorld(); DrawDebugLine(World, FirstParticle.Position, LastParticle.Position, FColor::Red, false, 0.0, -1, 1.5); }
		return true;
	}
	else
	{ ParticlesCount = 0; return false; }
}

void UJakubCablePhysic::SetParticleLocation(FVector Location, int ParticleIndex)
{
	int SafeIndex = FMath::Clamp<int>(ParticleIndex, 0, Particles.Num() - 1);
	Particles[SafeIndex].TargetLocation = Location;
}

void UJakubCablePhysic::UpdateCableLength(float NewLenght, bool RefreshCableInitValues)
{
	if (NewLenght > 0)
	{
		CableLength = NewLenght;
		if (RefreshCableInitValues == false) return;
		//Calculate Segments Length
		TArray<int> NotFreePoints;
		NotFreePoints = GetIndicesOfAttachedPoints();
		SegmentsLength.SetNum(0);
		if (NotFreePoints.Num() > 0)
		{
			NotFreePoints.Add(0);
			NotFreePoints.Sort();
			float PointsDiff = 0.0;
			for (int ii = 0; ii < NotFreePoints.Num() - 1; ii++)
			{
				PointsDiff = 1.0 * FMath::Clamp<int>(NotFreePoints[ii + 1] - NotFreePoints[ii], 0, Particles.Num());
				SegmentsLength.Add(PointsDiff * (CableLength / (float)NumSegments));
			}
		}
		else
		{
			SegmentsLength.Add(CableLength);
		}
	}
	return;
}

TArray<float> UJakubCablePhysic::GetStretchTolleranceValuePerSegment()
{
	TArray<float> OutputArray = {};
	TArray<float> InitSegments = GetCableInitSegmentsLength();
	TArray<float> CurrentSegments = GetCableCurrentSegmentsLength();
	TArray<int> ParticlesPerSegment = GetIndicesOfAttachedPoints();
	float L = 0.0;

	const int AttachParticlesNum = ParticlesPerSegment.Num();

	if(AttachParticlesNum == 0) { return TArray<float>(); }

	if (AttachParticlesNum == CurrentSegments.Num())
	{
		CurrentSegments.RemoveAt(0); // remove this elements becouse when attach points equal segments the first segment is attachet only to one constraint. 
		InitSegments.RemoveAt(0);	// To property calculate stretch is required two constraints
	}

	if (InitSegments.Num() > 0 && CurrentSegments.Num() > 0 && ParticlesPerSegment.Num() > CurrentSegments.Num())
	{
		for (int i = 0; i < CurrentSegments.Num(); i++)
		{
			int PCountPerSegment = ParticlesPerSegment[FMath::Clamp<int>(i + 1,0, ParticlesPerSegment.Num()-1)] - ParticlesPerSegment[i];
			float Correct = 1;
			if(PCountPerSegment > 0) { Correct = PCountPerSegment* (CableLength / (float)NumSegments); }
			L = CurrentSegments[i] - InitSegments[i];
			L = L / Correct;
			OutputArray.Add(L);
		}
		return OutputArray;
	}
	return TArray<float>();
}

//______________________________________________________________________________________________________________________

// znajdz pozycje komponentu dla poczatkowej czastki liny
void UJakubCablePhysic::GetStartPosition(FVector& OutStartPosition)
{
	OutStartPosition = GetComponentLocation();
}

// Znajdz pozycje komponentu, do ktorego ma byc dolaczona lina
FVector UJakubCablePhysic::GetAttachPosition(FCableAttachPoint& in)
{
	USceneComponent* ComponentObj = Cast<USceneComponent>(in.ComponentRef.GetComponent(GetOwner()));
	FVector OutLoc = GetComponentLocation();
	if (ComponentObj == NULL)
	{ ComponentObj = this; }
	if (in.SocketName != NAME_None)
	{ OutLoc = ComponentObj->GetSocketTransform(in.SocketName).TransformPosition(FVector(0,0,0)); }
	else
	{ OutLoc = ComponentObj->GetComponentTransform().TransformPosition(FVector(0, 0, 0)); }
	return OutLoc;
}

// Update level of detail state (Not Replicated!!!) 
void UJakubCablePhysic::RefreshLodState()
{
	//Get Player Pawn from world
	AActor* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	//If Player Pawn is not valid dont run Lods update
	if (IsValid(PlayerPawn) == false)
	{ return; }
	float SingleStageRadius = 1200.0;
	SingleStageRadius = SingleStageRadius * LodsRadiusRangeMultipler;
	FVector RopeOrigin = Particles[0].Position + Particles[FMath::Clamp<int>(Particles.Num() - 1, 0, Particles.Num() - 1)].Position;
	RopeOrigin = RopeOrigin / 2;
	const float Distance = FVector::Distance(RopeOrigin, PlayerPawn->GetActorLocation());

	if (Distance > SingleStageRadius * 3 && LodCurrentState != 3)
	{
		LodCurrentState = 3;
		UpdateSubstepTime(FMath::Clamp<float>(DefSubstepTime * 6, 0.001, 0.12), true);
	}
	else if (Distance > SingleStageRadius * 2 && Distance <= SingleStageRadius * 3 && LodCurrentState != 2)
	{
		LodCurrentState = 2;
		UpdateSubstepTime(FMath::Clamp<float>(DefSubstepTime * 3, 0.001, 0.12), true);
	}
	else if (Distance > SingleStageRadius * 1 && Distance <= SingleStageRadius * 2 && LodCurrentState != 1)
	{
		LodCurrentState = 1;
		UpdateSubstepTime(FMath::Clamp<float>(DefSubstepTime * 1.5, 0.001, 0.12), true);
	}
	else if (Distance <= SingleStageRadius * 1 && LodCurrentState != 0)
	{
		LodCurrentState = 0;
		UpdateSubstepTime(DefSubstepTime, true);
	}
	return;
}

FExposedCableParticle UJakubCablePhysic::PrivateToExposedStructure(FCableSingleParticle& InStruct)
{
	FExposedCableParticle LocalStruct;
	LocalStruct.bIsFree = InStruct.bFree; LocalStruct.bIsColliding = InStruct.bIsColliding;
	LocalStruct.Position = InStruct.Position; LocalStruct.OldPosition = InStruct.OldPosition;
	return LocalStruct;
}

void UJakubCablePhysic::OnVisibilityChanged()
{
	Super::OnVisibilityChanged();

	// Does not interact well with any other states that would be blocking tick
	if (bSkipCableUpdateWhenNotVisible)
	{
		SetComponentTickEnabled(IsVisible());
	}
}

void UJakubCablePhysic::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bSkipCableUpdateWhenNotVisible && !IsVisible())
	{
		SetComponentTickEnabled(false);
		return;
	}

	AActor* Owner = GetOwner();
	if (bSkipCableUpdateWhenNotOwnerRecentlyRendered && Owner && !Owner->WasRecentlyRendered(2.0f))
	{ return; }

	// Get Gravity Force From World And Convert It To Vector
	const FVector Gravity = FVector(0, 0, GetWorld()->GetGravityZ()) * CableGravityScale;

	// Update end points
	FVector CableStart, CableEnd;
	GetStartPosition(CableStart);
	CableEnd = CableStart + EndLocation;

	// Apply Attach Contraint For Begin Of Rope (only If Enabled)
	FCableSingleParticle& StartParticle = Particles[0];
	if (bAttachStart)
	{ StartParticle.Position = StartParticle.OldPosition = CableStart; StartParticle.bFree = false; }
	else
	{ StartParticle.bFree = true; }

	// Apply Attach Constraint For Other Particles (Only For Selected Indices)
	if (AttachToArray.Num() > 0)
	{
		TArray<int> CheckedIndex;
		for (int i = 0; i < AttachToArray.Num(); i++)
		{
			int PIndex = FMath::Clamp<int>(AttachToArray[i].ParticleIndex, 0, Particles.Num() - 1);
			if (CheckedIndex.Find(PIndex) == -1 && Particles[PIndex].bFree == false)
			{
				Particles[PIndex].Position = Particles[PIndex].OldPosition = GetAttachPosition(AttachToArray[i]);
			}
			CheckedIndex.AddUnique(PIndex);
		}
	}

	// Ensure a non-zero substep
	float UseSubstep = FMath::Max(SubstepTime, 0.005f);
	
	// [][][][][][][][][][][][][][]   S O L V E   S I M U L A T I O N   [][][][][][][][][][][][][][]
	if (bSolveSimulationPerFrame == true)
	{
		PerformSubstep(DeltaTime, Gravity);
	}
	else
	{
		// Perform simulation substeps
		if (bLimitTimeTo60FPS == true)
		{
			TimeRemainder += FMath::Clamp<float>(DeltaTime, 0.0166, 1);
		} //dt for 60FPS is nearly equal to 0.01667
		else
		{
			TimeRemainder += DeltaTime;
		}

		while (TimeRemainder > UseSubstep)
		{
			PerformSubstep(bUseSubstepping ? UseSubstep : TimeRemainder, Gravity);
			if (bUseSubstepping)
			{
				TimeRemainder -= UseSubstep;
			}
			else
			{
				TimeRemainder = 0.0f;
			}
		}
	}
	// [][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][][]

	// -----> Refresh LODs States <-----
	if (bUseUpdatingLODs)
	{
		RefreshLodState();
	}

	// Need to send new data to render thread
	MarkRenderDynamicDataDirty();

	// Call this because bounds have changed
	UpdateComponentToWorld();
};

void UJakubCablePhysic::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	Super::CreateRenderState_Concurrent(Context);

	SendRenderDynamicData_Concurrent();
}

void UJakubCablePhysic::ApplyWorldOffset(const FVector& InOffset, bool bWorldShift)
{
	Super::ApplyWorldOffset(InOffset, bWorldShift);

	for (FCableSingleParticle& Particle : Particles)
	{
		Particle.Position += InOffset;
		Particle.OldPosition += InOffset;
	}
}

void UJakubCablePhysic::SendRenderDynamicData_Concurrent()
{
	if (SceneProxy)
	{
		// Allocate cable dynamic data
		FCableDynamicData* DynamicData = new FCableDynamicData;

		// Transform current positions from particles into component-space array
		const FTransform& ComponentTransform = GetComponentTransform();
		int32 NumPoints = NumSegments + 1;
		DynamicData->CablePoints.AddUninitialized(NumPoints);
		for (int32 PointIdx = 0; PointIdx < NumPoints; PointIdx++)
		{
			DynamicData->CablePoints[PointIdx] = ComponentTransform.InverseTransformPosition(Particles[PointIdx].Position);
		}

		// Enqueue command to send to render thread
		FJakubCableSceneProxy* CableSceneProxy = (FJakubCableSceneProxy*)SceneProxy;
		ENQUEUE_RENDER_COMMAND(FSendCableDynamicData)(
			[CableSceneProxy, DynamicData](FRHICommandListImmediate& RHICmdList)
			{
				CableSceneProxy->SetDynamicData_RenderThread(DynamicData, RHICmdList);
			});
	}
}

FBoxSphereBounds UJakubCablePhysic::CalcBounds(const FTransform& LocalToWorld) const
{
	// Calculate bounding box of cable points
	FBox CableBox(ForceInit);

	const FTransform& ComponentTransform = GetComponentTransform();

	for (int32 ParticleIdx = 0; ParticleIdx < Particles.Num(); ParticleIdx++)
	{
		const FCableSingleParticle& Particle = Particles[ParticleIdx];
		CableBox += ComponentTransform.InverseTransformPosition(Particle.Position);
	}

	// Expand by cable radius (half cable width)
	return FBoxSphereBounds(CableBox.ExpandBy(0.5f * CableWidth)).TransformBy(LocalToWorld);
}

void UJakubCablePhysic::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	OutSockets.Add(FComponentSocketDescription(CableStartSocketName, EComponentSocketType::Socket));
	for (int i = 0; i < AttachToArray.Num(); i++)
	{ OutSockets.Add(FComponentSocketDescription(AttachToArray[i].SocketName, EComponentSocketType::Socket)); }
}

FTransform UJakubCablePhysic::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	int32 NumParticles = Particles.Num();
	FName LocEndSocket = TEXT("None"); //Takie cos by moc usunac zmienna
	if ((InSocketName == LocEndSocket || InSocketName == CableStartSocketName) && NumParticles >= 2)
	{
		FVector ForwardDir, Pos;
		if (InSocketName == LocEndSocket)
		{
			FVector LastPos = Particles[NumParticles - 1].Position;
			FVector PreviousPos = Particles[NumParticles - 2].Position;

			ForwardDir = (LastPos - PreviousPos).GetSafeNormal();
			Pos = LastPos;
		}
		else
		{
			FVector FirstPos = Particles[0].Position;
			FVector NextPos = Particles[1].Position;

			ForwardDir = (NextPos - FirstPos).GetSafeNormal();
			Pos = FirstPos;
		}

		const FQuat RotQuat = FQuat::FindBetween(FVector(1, 0, 0), ForwardDir);
		FTransform WorldSocketTM = FTransform(RotQuat, Pos, FVector(1, 1, 1));

		switch (TransformSpace)
		{
		case RTS_World:
		{
			return WorldSocketTM;
		}
		case RTS_Actor:
		{
			if (const AActor* Actor = GetOwner())
			{
				return WorldSocketTM.GetRelativeTransform(GetOwner()->GetTransform());
			}
			break;
		}
		case RTS_Component:
		{
			return WorldSocketTM.GetRelativeTransform(GetComponentTransform());
		}
		}
	}

	return Super::GetSocketTransform(InSocketName, TransformSpace);
}

bool UJakubCablePhysic::HasAnySockets() const
{
	return (Particles.Num() >= 2);
}

bool UJakubCablePhysic::DoesSocketExist(FName InSocketName) const
{
	for (int i = 0; i < AttachToArray.Num(); i++)
	{
		if (AttachToArray[i].SocketName == InSocketName)
		{ return true; }
	}
	return (InSocketName == CableStartSocketName);
}
