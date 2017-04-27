/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_API_INSTANCE_H
#define NV_HAIR_DX11_API_INSTANCE_H

#include <Nv/Common/NvCoComPtr.h>

#include <Nv/HairWorks/Internal/NvHairLodInfo.h>

#include <Nv/HairWorks/Internal/NvHairSliSystem.h>
#include <Nv/HairWorks/Internal/NvHairApiInstance.h>

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

#include "NvHairDx11ApiAsset.h"
#include "NvHairDx11ApiGlobal.h"

namespace nvidia {
namespace HairWorks { 

class Dx11ApiInstance: public ApiInstance
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11ApiInstance, ApiInstance);

	typedef SliSystem::Handle SliHandle;

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
	Result init(Dx11ApiGlobal* apiGlobal, Instance& inst);
	
	Result startEarlyPushSimulationBuffers(SliSystem* system);
	Result finishEarlyPushSimulationBuffers(SliSystem* system);
	Result setHintTessellationBuffers(SliSystem* system);

	ID3D11Buffer* nextTessellatedMasterStrandBuffer();
	ID3D11ShaderResourceView* getPreviousTessellatedMasterStrandSrv() const;
	ID3D11ShaderResourceView* getCurrentTessellatedMasterStrandSrv() const;

	/// Get the texture 
	NV_FORCE_INLINE ID3D11ShaderResourceView* getTexture(ETextureType type) const { return m_textures[IndexT(type)]; }
	NV_FORCE_INLINE Bool isTextureUsed(ETextureType type) { return m_textures[IndexT(type)].get() != NV_NULL; }

	Result _prepareTesselateConstantBuffer(ID3D11DeviceContext* context, Instance* inst, Float simulationInterp);

	Result _createComputeStatsBuffers(ID3D11Device* device, const Instance& inst);

	Result _createStreamOutBuffers(ID3D11Device* device, const Instance& ins);
	Result _createComputeShaderConstantBuffers(ID3D11Device* device);
	Result _createShadingConstantBuffers(ID3D11Device* device);

	Result _createGrowthMeshBuffers(ID3D11Device* device, const Instance& inst);

	Result _createMasterStrandVertexBuffers(ID3D11Device* device, const Instance& inst);
	Result _createMasterStrandTangentBuffers(ID3D11Device* device, const Instance& inst);

	Result _createMasterStrandFrames(ID3D11Device* device, const Instance& inst);
	Result _createMasterStrandNormals(ID3D11Device* device, const Instance& inst);
	Result _createMasterStrandLuminances(ID3D11Device* device, const Instance& inst);

	Result _createPinConstraintBuffers(ID3D11Device* device, const Instance& inst, const AssetDescriptor &hairDesc);

	Result _createDebugTessellatedIndexBuffer(ID3D11Device* device, const Instance& inst);
	Result _createDebugConstantBuffers(ID3D11Device* device);
	Result _computeStats();
	
		/// ComputeStats and Visualization rendering need buffers that aren't created by default
		/// _requireDebugResources should be called before such resources are needed, will create if not already created
	Result _requireDebugResources();
		/// Performs the actual creation of the debug resources
	Result _createDebugResources();

	ID3D11ShaderResourceView* _getResource(EShaderResourceType t) const;

		/// Ctor
	Dx11ApiInstance();
	
	const Dx11ApiAsset* m_apiAsset;	///< Pointer to api asset data
	Dx11ApiGlobal* m_apiGlobal;			///< global data
	Instance* m_instance;			///< The platform independent instance

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers modified in simulation shader (CS)
	
	// simulated master cvs
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterStrandUav;

	// Holds an exact copy of the last frames m_masterStrandBuffer positions. Needed for interp when using FIR. 
	// NOTE! m_masterStrandPrevBuffer does NOT hold exact copy of previous frames positions.
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandInterpBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandInterpSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterStrandInterpUav;

	// Master strand radius // NUTT
	//NvCo::ComPtr<ID3D11Buffer> m_masterStrandRadiusBuffer;
	//NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandRadiusSrv;
	//NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterStrandRadiusUav;

	// NUTT: Delta between interpolated position and skinned position, for interpolation
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandInterpolationDeltaBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView>  m_masterStrandInterpolationDeltaSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterStrandInterpolationDeltaUav;

	// simulated master previous cvs
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandPrevBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandPrevSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterStrandPrevUav;

	// skinned master cvs
	NvCo::ComPtr<ID3D11Buffer> m_skinnedMasterStrandBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_skinnedMasterStrandSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_skinnedMasterStrandUav;

	// simulated frames
	NvCo::ComPtr<ID3D11Buffer> m_masterFramesBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterFramesSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterFramesUav;

	// simulated normal
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandNormalBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandNormalSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterStrandNormalUav;

	// simulated luminance
	NvCo::ComPtr<ID3D11Buffer>				m_masterStrandLuminanceBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView>	m_masterStrandLuminanceSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView>	m_masterStrandLuminanceUav;

	// simulated tangent
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandTangentBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandTangentSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_masterStrandTangentUav;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers modified in spline tessellation shader (GS)
	
	// double buffering status for master strand tessellation
	// 0: first time, 1: first time process done, 2: first buffer, 3: second buffer
	int	m_tessellatedMasterStrandDoubleBufferStatus;

	// tessellated master strand vertices (spline interpolated)
	NvCo::ComPtr<ID3D11Buffer> m_tessellatedMasterStrandBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_tessellatedMasterStrandSrv;

	// tessellated master strand prev vertices (spline interpolated)
	NvCo::ComPtr<ID3D11Buffer> m_tessellatedMasterStrandPrevBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_tessellatedMasterStrandPrevSrv;

	// tessellated master strand tangents (spline interpolated)
	NvCo::ComPtr<ID3D11Buffer> m_tessellatedMasterStrandTangentsBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_tessellatedMasterStrandTangentsSrv;

	// tessellated master strand normals (spline interpolated)
	NvCo::ComPtr<ID3D11Buffer> m_tessellatedMasterStrandNormalsBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_tessellatedMasterStrandNormalsSrv;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers used for GPU skinning
	
	// GPU Skinning and Compute Data (DX Path)
	NvCo::ComPtr<ID3D11Buffer> m_csConstantBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Buffers used for pin constraints
	
	NvCo::PodBuffer<NvHair_Pin> m_pins;

	NvCo::ComPtr<ID3D11Buffer> m_pinsBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_pinsStaging;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_pinsSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_pinsUav;

	NvCo::ComPtr<ID3D11Buffer> m_pinScratchBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_pinScratchStagingBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_pinScratchSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_pinScratchUav;

	////////////////////////////////////////////////////////////////////////////////////////////
	// GPU buffers for growth mesh skinning
	
	NvCo::ComPtr<ID3D11Buffer> m_growthMeshVertexBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_growthMeshVertexSrv;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_growthMeshVertexUav;

	NvCo::ComPtr<ID3D11InputLayout> m_growthMeshLayout;

	////////////////////////////////////////////////////////////////////////////////////////////
	// [optional] buffers for stats and profiling

	NvCo::PodBuffer<gfsdk_float4> m_stats;		///< Holds stats per face

	NvCo::ComPtr<ID3D11Buffer> m_statsBuffer;
	NvCo::ComPtr<ID3D11UnorderedAccessView> m_statsUav;
	NvCo::ComPtr<ID3D11Buffer> m_statsStagingBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_statsConstantBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// debug rendering
	
	NvCo::ComPtr<ID3D11Buffer> m_debugTessellatedIndexBuffer;

	NvCo::ComPtr<ID3D11Buffer> m_debugVsConstantBuffer;

	// const buffer to store all the shading variable
	NvCo::ComPtr<ID3D11Buffer> m_hairSplineConstantBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_hairTessellationConstantBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_hairPixelShaderConstantBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// CPU Buffers
	
	// copied from the descriptor
	Int m_numMasterStrands;
	Int m_numMasterStrandControlVertices;
	Int m_numFaces;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Textures

	NvCo::ComPtr<ID3D11ShaderResourceView> m_textures[TextureType::COUNT_OF];

	////////////////////////////////////////////////////////////////////////////////////////////
	// for SLI
	SliHandle m_growthMeshVertexSliHandle;

	SliHandle m_statsSliHandle;

	SliHandle m_skinnedMasterStrandSliHandle;

	SliHandle m_masterStrandSliHandle;
	SliHandle m_masterStrandPrevSliHandle;

	SliHandle m_masterFramesSliHandle;
	SliHandle m_masterStrandTangentSliHandle;
	SliHandle m_masterStrandNormalSliHandle;
	SliHandle m_masterStrandLuminanceSliHandle;
	SliHandle m_pinConstraintsSliHandle;
	SliHandle m_scratchSliHandle;

	SliHandle m_tessellatedMasterStrandSliHandle;
	SliHandle m_tessellatedMasterStrandPrevSliHandle;
	SliHandle m_tessellatedMasterStrandTangentSliHandle;
	SliHandle m_tessellatedMasterStrandNormalSliHandle;

	Result m_debugResourcesResult;						///< Either NV_E_UNINITIALIZED, or the result of the call to _createDebugResources
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX11_API_INSTANCE_H
