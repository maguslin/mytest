/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_API_ASSET_H
#define NV_HAIR_DX12_API_ASSET_H

#include <Nv/Common/NvCoComPtr.h>

#include <Nv/HairWorks/Internal/NvHairSliSystem.h>
#include <Nv/HairWorks/Internal/NvHairApiAsset.h>

#include "NvHairDx12ApiGlobal.h"
#include "NvHairDx12Buffer.h"

namespace nvidia {
namespace HairWorks { 

class Dx12ApiAsset: public ApiAsset
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx12ApiAsset, ApiAsset);

	
		/// Must be called before used
	Result init(Dx12ApiGlobal* apiGlobal, Asset* asset);
		/// Ctor
	Dx12ApiAsset();
		/// Dtor
	~Dx12ApiAsset();

	protected:
	Result _createTessellatedFaceBuffers(const Dx12Info& info);
	Result _createFaceTexCoords(const Dx12Info& info);
	Result _createHairTexCoords(const Dx12Info& info);
	Result _createHairInteractionBuffers(const Dx12Info& info);

		/// Creates the bone weights/indexes read only buffers
	Result _createSkinningBuffers(const Dx12Info& info);
		/// Creates all the MasterStrand related buffers
	Result _createMasterStrandBuffers(const Dx12Info& info);

	Result _createGrowthMeshBuffers(const Dx12Info& info);
	Result _createDebugRenderBuffers(const Dx12Info& info);
	
	public:

	Dx12ApiGlobal* m_apiGlobal;		///< Api specific globals
	Asset* m_asset;					///< Platform independent asset description


	NvCo::Dx12DescriptorHeap m_viewHeap;		///< Cbv, Srv, Uav 

	////////////////////////////////////////////////////////////////////////////////////////////
	// Master strand
	
	// distance spring length for simulation
	Dx12ReadOnlyBuffer m_masterStrandLinearSpringLengthsBuffer;
	// bend spring length for simulation
	Dx12ReadOnlyBuffer m_masterStrandBendSpringLengthsBuffer;
	// root distance for simulation (LRA)
	Dx12ReadOnlyBuffer m_masterStrandRootDistancesBuffer;
	// master length and tessellated length 
	Dx12ReadOnlyBuffer m_masterStrandNormalizedLengthToRootBuffer;
	// index buffers for master strand cv offset
	Dx12ReadOnlyBuffer m_masterStrandOffsetBuffer;
	// original master vertices
	Dx12ReadOnlyBuffer m_origMasterStrandBuffer;
	// original frames
	Dx12ReadOnlyBuffer m_origMasterFramesBuffer;
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// Position buffers

	// NUTT: Original pos
	//ComPtr<ID3D11Buffer> m_origMasterPosBuffer;
	//ComPtr<ID3D11ShaderResourceView> m_origMasterPosSrv;

	// local pos for previous particle w.r.t frame
	Dx12ReadOnlyBuffer m_prevMasterLocalPosBuffer;
	// local pos for next particle w.r.t frame
	Dx12ReadOnlyBuffer m_nextMasterLocalPosBuffer;
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// Interaction

	// hair interaction indices
	Dx12ReadOnlyBuffer m_masterStrandInteractionIndicesBuffer;
	// hair interaction index offset
	Dx12ReadOnlyBuffer m_masterStrandInteractionOffsetBuffer;
	// hair interaction rest length
	Dx12ReadOnlyBuffer m_masterStrandInteractionLengthBuffer;
	
	int	m_totalNumHairInteractions;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Read only buffers used in hair interpolation

	// indices for interpolated hairs per face
	Dx12ReadOnlyBuffer m_faceHairIndicesBuffer;
	// per face texture coordinate
	Dx12ReadOnlyBuffer m_faceTexCoordsBuffer;
	// per hair (face vertex) texture coordinates
	Dx12ReadOnlyBuffer m_hairTexCoordsBuffer;
	
	////////////////////////////////////////////////////////////////////////////////////////////
	// Skinning

	Dx12ReadOnlyBuffer m_boneIndicesBuffer;
	Dx12ReadOnlyBuffer m_boneWeightsBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// Growth mesh

	Dx12ReadOnlyBuffer m_growthMeshRestNormalBuffer;
	Dx12ReadOnlyBuffer m_growthMeshRestTangentBuffer;
	Dx12IndexBuffer m_growthMeshIndexBuffer;

	////////////////////////////////////////////////////////////////////////////////////////////
	// debug rendering

	Dx12ReadOnlyBuffer m_debugVertexToHairBuffer;

	Dx12IndexBuffer m_debugMasterIndexBuffer;

	Dx12IndexBuffer m_debugFrameIndexBuffer;
	Dx12IndexBuffer m_debugLocalPosIndexBuffer;
	Dx12IndexBuffer m_debugNormalIndexBuffer;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX11_API_ASSET_H
