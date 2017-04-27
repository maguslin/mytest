/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_API_ASSET_H
#define NV_HAIR_DX11_API_ASSET_H

#include <Nv/Common/NvCoComPtr.h>

#include <Nv/HairWorks/Internal/NvHairSliSystem.h>
#include <Nv/HairWorks/Internal/NvHairApiAsset.h>

#include "NvHairDx11ApiGlobal.h"

namespace nvidia {
namespace HairWorks { 

class Dx11ApiAsset: public ApiAsset
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11ApiAsset, ApiAsset);

	
		/// Must be called before used
	Result init(Dx11ApiGlobal* apiGlobal, Asset* asset);
		/// Ctor
	Dx11ApiAsset();

	virtual ~Dx11ApiAsset() {}

	protected:
	Result _createTessellatedFaceBuffers();
	Result _createFaceTexCoords();
	Result _createHairTexCoords();
	Result _createHairInteractionBuffers();

		/// Creates the bone weights/indexes read only buffers
	Result _createSkinningBuffers();
		/// Creates all the MasterStrand related buffers
	Result _createMasterStrandBuffers();

	Result _createGrowthMeshBuffers();
	Result _createDebugRenderBuffers();
	
	public:

	Dx11ApiGlobal* m_apiGlobal;		///< Api specific globals
	Asset* m_asset;				///< Platform independent asset description

	// counters computed during buffer creation time
	NvCo::PodBuffer<gfsdk_float3> m_faceIndices;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Master strand
	
	// distance spring length for simulation
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandLinearSpringLengthsBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandLinearSpringLengthsSrv;

	// bend spring length for simulation
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandBendSpringLengthsBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandBendSpringLengthsSrv;

	// root distance for simulation (LRA)
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandRootDistancesBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandRootDistancesSrv;

	// master length and tessellated length 
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandNormalizedLengthToRootBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandNormalizedLengthToRootSrv;

	// index buffers for master strand cv offset
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandOffsetBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandOffsetSrv;

	// original master vertices
	NvCo::ComPtr<ID3D11Buffer> m_origMasterStrandBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_origMasterStrandSrv;

	// original frames
	NvCo::ComPtr<ID3D11Buffer> m_origMasterFramesBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_origMasterFramesSrv;

	///////////////////////////////////////////////////////////////////////////////////////////
	// Position buffers

	// NUTT: Original pos
	NvCo::ComPtr<ID3D11Buffer> m_origMasterPosBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_origMasterPosSrv;

	// local pos for previous particle w.r.t frame
	NvCo::ComPtr<ID3D11Buffer> m_prevMasterLocalPosBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_prevMasterLocalPosSrv;

	// local pos for next particle w.r.t frame
	NvCo::ComPtr<ID3D11Buffer> m_nextMasterLocalPosBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_nextMasterLocalPosSrv;

	///////////////////////////////////////////////////////////////////////////////////////////
	// Interaction

	// hair interaction indices
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandInteractionIndicesBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandInteractionIndicesSrv;

	// hair interaction index offset
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandInteractionOffsetBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandInteractionOffsetSrv;

	// hair interaction rest length
	NvCo::ComPtr<ID3D11Buffer> m_masterStrandInteractionLengthBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_masterStrandInteractionLengthSrv;

	int	m_totalNumHairInteractions;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Read only buffers used in hair interpolation

	// indices for interpolated hairs per face
	NvCo::ComPtr<ID3D11Buffer> m_faceHairIndicesBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_faceHairIndicesSrv;

	// per face texture coordinate
	NvCo::ComPtr<ID3D11Buffer> m_faceTexCoordsBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_faceTexCoordsSrv;

	// per hair (face vertex) texture coordinates
	NvCo::ComPtr<ID3D11Buffer> m_hairTexCoordsBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_hairTexCoordsSrv;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Skinning

	NvCo::ComPtr<ID3D11Buffer> m_boneIndicesBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_boneIndicesSrv;
	NvCo::ComPtr<ID3D11Buffer> m_boneWeightsBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_boneWeightsSrv;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Growth mesh

	NvCo::ComPtr<ID3D11Buffer> m_growthMeshRestNormalBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_growthMeshRestNormalSrv;

	NvCo::ComPtr<ID3D11Buffer> m_growthMeshRestTangentBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_growthMeshRestTangentSrv;

	NvCo::ComPtr<ID3D11Buffer> m_growthMeshIndexBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// debug rendering

	NvCo::ComPtr<ID3D11Buffer> m_debugVertexToHairBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_debugVertexToHairSrv;

	NvCo::ComPtr<ID3D11Buffer> m_debugMasterIndexBuffer;

	NvCo::ComPtr<ID3D11Buffer> m_debugFrameIndexBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_debugLocalPosIndexBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_debugNormalIndexBuffer;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX11_API_ASSET_H
