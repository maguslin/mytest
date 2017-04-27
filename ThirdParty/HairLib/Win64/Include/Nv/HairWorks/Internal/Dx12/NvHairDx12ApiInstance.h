/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_API_INSTANCE_H
#define NV_HAIR_DX12_API_INSTANCE_H

#include <Nv/Common/NvCoComPtr.h>

#include <Nv/HairWorks/Internal/NvHairLodInfo.h>

#include <Nv/HairWorks/Internal/NvHairSliSystem.h>
#include <Nv/HairWorks/Internal/NvHairApiInstance.h>

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include "NvHairDx12ApiAsset.h"
#include "NvHairDx12ApiGlobal.h"
#include "NvHairDx12AsyncTypes.h"

namespace nvidia {
namespace HairWorks { 

class Dx12ApiInstance: public ApiInstance
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx12ApiInstance, ApiInstance);

	typedef SliSystem::Handle SliHandle;
	typedef NvCo::Dx12DescriptorSet DescriptorSet;

	enum 
	{
		MAX_NUM_TESS_PASSES = 32,
	};

	// ApiHairInstance
	virtual Result computeStats(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Stats& statsOut) NV_OVERRIDE;
	virtual Result stepSimulation(float timeStep, const gfsdk_float4x4* worldReference) NV_OVERRIDE;
	virtual Result renderHairShading(const RenderViewInfo& frameViewInfo, ShaderSettings& shaderSettings, const NvCo::ConstApiPtr& other) NV_OVERRIDE;
	virtual Void calcPixelConstantBuffer(const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& constantBufferOut) NV_OVERRIDE;
	virtual Bool isApiTextureUsed(ETextureType type) NV_OVERRIDE;
	virtual Result getApiTextures(const ETextureType* types, Int numTextures, const NvCo::ApiPtr& textureOut) NV_OVERRIDE;
	virtual Result setApiTexture(ETextureType type, const NvCo::ApiHandle& texture) NV_OVERRIDE;
	virtual Result getApiResources(const EShaderResourceType* types, Int numResources, const NvCo::ApiPtr& textureOut) NV_OVERRIDE;
	virtual Result debugDraw(EDebugDraw drawType, const ViewInfo& viewInfo) NV_OVERRIDE;
	virtual Result getPinMatrix(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int pinId, gfsdk_float4x4& matrixOut) NV_OVERRIDE;
	virtual Result getPinMatrices(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int startPinIndex, Int numPins, gfsdk_float4x4* matricesOut) NV_OVERRIDE;
	virtual Result preRender(Float simulationInterp) NV_OVERRIDE;
	virtual Result updatePinConstraintBuffer() NV_OVERRIDE;
	virtual Result onNumVertsPerSegmentChanged() NV_OVERRIDE;

	/// Initialize
	Result init(Dx12ApiGlobal* apiGlobal, Instance& inst);
	
	Result startEarlyPushSimulationBuffers(SliSystem* system);
	Result finishEarlyPushSimulationBuffers(SliSystem* system);
	Result setHintTessellationBuffers(SliSystem* system);

	Dx12StreamOutBuffer& nextTessellatedMasterStrandBuffer();

	Dx12StreamOutBuffer& getPreviousTessellatedMasterStrand();
	Dx12StreamOutBuffer& getCurrentTessellatedMasterStrand();

		/// Implement const versions using non-const versions
	NV_FORCE_INLINE const Dx12StreamOutBuffer& getPreviousTessellatedMasterStrand() const { return const_cast<ThisType*>(this)->getPreviousTessellatedMasterStrand(); }
	NV_FORCE_INLINE const Dx12StreamOutBuffer& getCurrentTessellatedMasterStrand() const { return const_cast<ThisType*>(this)->getCurrentTessellatedMasterStrand(); }

	/// Get the texture 
	NV_FORCE_INLINE D3D12_CPU_DESCRIPTOR_HANDLE getTextureSrv(ETextureType type) const 
	{
		const Dx12Texture& tex = m_textures[IndexT(type)];
		return tex.isSet() ? tex.getSrvCpuHandle() : m_nullTextureSrvCpuHandle;
	}
	NV_FORCE_INLINE Bool isTextureUsed(ETextureType type) { return m_textures[IndexT(type)].m_texture.get() != NV_NULL; }

		/// Get all the srvs
	Void getTextureSrvs(const ETextureType* types, Int numTypes, D3D12_CPU_DESCRIPTOR_HANDLE* dst);

	Result _createComputeStatsBuffers(const Dx12Info& info);

	Result _createStreamOutBuffers(const Dx12Info& info);

	Result _createGrowthMeshBuffers(const Dx12Info& info);

	Result _createMasterStrandVertexBuffers(const Dx12Info& info);
	Result _createMasterStrandTangentBuffers(const Dx12Info& info);

	Result _createMasterStrandFrames(const Dx12Info& info);
	Result _createMasterStrandNormals(const Dx12Info& info);
	
	Result _createPinConstraintBuffers(const Dx12Info& info, const AssetDescriptor &hairDesc);

	Result _createDebugTessellatedIndexBuffer(const Dx12Info& info);

	Result _createMasterStrandLuminancesBuffer(const Dx12Info& info);

	Result _computeStats(NvCo::Dx12CircularResourceHeap::Cursor& cursorOut);

	Void _completeComputeStats(Dx12ComputeStatsAsync& async);
	Void _completeGetPinMatrix(Dx12GetPinMatrixAsync& async); 
	
		/// ComputeStats and Visualization rendering need buffers that aren't created by default
		/// _requireDebugResources should be called before such resources are needed, will create if not already created
	Result _requireDebugResources();
		/// Performs the actual creation of the debug resources
	Result _createDebugResources();

	ID3D12Resource* _getResource(EShaderResourceType t) const;
	D3D12_CPU_DESCRIPTOR_HANDLE _getDescriptorHandle(EShaderResourceType t) const;

		/// Ctor
	Dx12ApiInstance();
		/// Dtor
	~Dx12ApiInstance();
	
	D3D12_CPU_DESCRIPTOR_HANDLE m_nullTextureSrvCpuHandle;

	const Dx12ApiAsset* m_apiAsset;	///< Pointer to api asset data
	Dx12ApiGlobal* m_apiGlobal;		///< global data
	Instance* m_instance;			///< The platform independent instance

	NvCo::Dx12DescriptorHeap m_viewHeap;	///< Cbv, Srv, Uav 

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers modified in simulation shader (CS)

	// simulated master cvs
	Dx12StructuredBuffer m_masterStrandBuffer;
	// Holds an exact copy of the last frames m_masterStrandBuffer positions. Needed for interp when using FIR. 
	// NOTE! m_masterStrandPrevBuffer does NOT hold exact copy of previous frames positions.
	Dx12StructuredBuffer m_masterStrandInterpBuffer;

	// Master strand radius // NUTT
	//Dx12StructuredBuffer m_masterStrandRadiusBuffer;

	// simulated master previous cvs
	Dx12StructuredBuffer m_masterStrandPrevBuffer;
	// skinned master cvs
	Dx12StructuredBuffer m_skinnedMasterStrandBuffer;
	// simulated frames
	Dx12StructuredBuffer m_masterFramesBuffer;
	// simulated normal
	Dx12StructuredBuffer m_masterStrandNormalBuffer;

	// simulated luminance
	Dx12StructuredBuffer m_masterStrandLuminanceBuffer;
	
	// simulated tangent
	Dx12StructuredBuffer m_masterStrandTangentBuffer;

	// NUTT: Delta between interpolated position and skinned position, for interpolation
	Dx12StructuredBuffer m_masterStrandInterpolationDeltaBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers modified in spline tessellation shader (GS)
	
	// double buffering status for master strand tessellation
	// 0: first time, 1: first time process done, 2: first buffer, 3: second buffer
	int	m_tessellatedMasterStrandDoubleBufferStatus;

	// tessellated master strand vertices (spline interpolated)
	Dx12StreamOutBuffer m_tessellatedMasterStrandBuffer;
	// tessellated master strand prev vertices (spline interpolated)
	Dx12StreamOutBuffer m_tessellatedMasterStrandPrevBuffer;
	// tessellated master strand tangents (spline interpolated)
	Dx12StreamOutBuffer m_tessellatedMasterStrandTangentsBuffer;
	// tessellated master strand normals (spline interpolated)
	Dx12StreamOutBuffer m_tessellatedMasterStrandNormalsBuffer;
	
	Dx12Resource m_streamOutPositionBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS m_streamOutPositionGpuAddr; 

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers used for GPU skinning
	
	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers used for pin constraints
	
	NvCo::PodBuffer<NvHair_Pin> m_pins;
	Dx12StructuredBuffer m_pinsBuffer;
	Dx12StructuredBuffer m_pinScratchBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// GPU buffers for growth mesh skinning
	
	Dx12StructuredBuffer m_growthMeshVertexBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// debug rendering

	// This will change if the m_numVertsPerSegment on the instance changes
	Dx12IndexBuffer m_debugTessellatedIndexBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// [optional] buffers for stats and profiling

	NvCo::PodBuffer<gfsdk_float4> m_stats;		///< Holds stats per face
	Dx12StructuredBuffer m_statsBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// CPU Buffers
	
	// copied from the descriptor
	Int m_numMasterStrands;
	Int m_numMasterStrandControlVertices;
	Int m_numFaces;
	Int32 m_simulationStep;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Textures

	Dx12Texture m_textures[TextureType::COUNT_OF];

	enum BundleType
	{
		BUNDLE_TESSELATE, 
		BUNDLE_SIMULATE,
		BUNDLE_INTERACTION,
		BUNDLE_RENDER,
		BUNDLE_COUNT_OF,
	};

	Dx12Bundle m_bundles[BUNDLE_COUNT_OF];
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX12_API_INSTANCE_H
