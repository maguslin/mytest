/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

//#define DEBUG_HAIR_SKIP 20

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx11DebugRender.h"

#include <Nv/HairWorks/Internal/Dx11/NvHairDx11Util.h>
#include <Nv/HairWorks/Internal/Dx11/NvHairDx11ApiInstance.h>

#include <Nv/HairWorks/Internal/NvHairInstance.h>

// constant buffer definition shared with shaders
#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>

namespace nvidia { 
namespace HairWorks { 

static Result _bindConstantBuffer(Dx11ApiGlobal* glob, ID3D11Buffer* constantBuffer, NvHair_VisualizeConstantBuffer& buffer, bool setGS = false)
{
	ID3D11DeviceContext* context = glob->getContext();

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// set CB_CS_PER_FRAME constant data
	{
		NV_RETURN_ON_FAIL(context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		NvHair_VisualizeConstantBuffer* perFrame = ( NvHair_VisualizeConstantBuffer* )mappedResource.pData;
		memcpy(perFrame, &buffer, sizeof(NvHair_VisualizeConstantBuffer));
		context->Unmap(constantBuffer, 0);
	}
	context->VSSetConstantBuffers(0, 1, &constantBuffer);
	context->PSSetConstantBuffers(0, 1, &constantBuffer);
	if (setGS)
		context->GSSetConstantBuffers(0, 1, &constantBuffer);
	return NV_OK;
}

static Void _setMatrices(const Instance* inst, const ViewInfo& viewInfo, NvHair_VisualizeConstantBuffer& dst)
{
	dst.modelToWorld = inst->m_modelToWorld;
	dst.viewProjection = viewInfo.m_viewProjectionMatrix;
}

static void _setHairMinMax(Dx11ApiInstance* apiInst, NvHair_VisualizeConstantBuffer& dst)
{
	const InstanceDescriptor& params = apiInst->m_instance->getDefaultMaterial();
	dst.hairMin = 0;
	dst.hairMax = apiInst->m_numMasterStrands;
	dst.hairSkip = params.m_visualizeHairSkips;
}

static void _setColor(float r, float g, float b, float a, NvHair_VisualizeConstantBuffer& dst)
{
	dst.color = gfsdk_makeFloat4(r, g, b, a);
}

static void _drawSphere(Dx11ApiGlobal* glob)
{
	ID3D11DeviceContext* context = glob->getContext();

	// draw spheres
	UINT stride = UINT(sizeof(float4));
	UINT offset = 0;
	{
		context->IASetVertexBuffers(0, 1, glob->m_sphereVertexBuffer.readRef(), &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->Draw(glob->m_numSphereLines * 2, 0);
	}
}

/* static */Result Dx11DebugRender::drawGuideHairs(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;

	// set shader resource
	{
		ID3D11ShaderResourceView* srvs[] =
		{ 
			apiInst->m_masterStrandSrv,		
			apiAsset->m_debugVertexToHairSrv
		};
		context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	}
	// set constant buffer
	{
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 0, 0, 1, dst);
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
	}
	// draw lines
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_DEBUG_GUIDE], context);
		// we use vertex data from master strand SRV directly coming from the compute shader
	    unsigned int stride = 0, offset = 0;
		ID3D11Buffer* buffer[] = { NV_NULL };
		context->IASetVertexBuffers(0, 1, buffer, &stride, &offset);
		context->IASetIndexBuffer(apiAsset->m_debugMasterIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
	    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

		int lineCount = inst->m_asset->m_totalNumMasterSegments;
		context->DrawIndexed( lineCount * 2, 0, 0 );
	}

	return NV_OK;
}

/* static */Result Dx11DebugRender::drawSkinnedGuideHairs(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	// set shader resource
	{
		ID3D11ShaderResourceView* srvs[] = 
		{ 
			apiInst->m_skinnedMasterStrandSrv,		
			apiAsset->m_debugVertexToHairSrv
		};
		context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	}
	// set constant buffer
	{
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 1, 0, 1, dst);
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
	}
	// draw lines
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_DEBUG_GUIDE], context);

		// we use vertex data from master strand SRV directly coming from the compute shader
	    unsigned int stride = 0;
		unsigned int offset = 0;
		ID3D11Buffer* buffer[] = { NV_NULL };
		
		context->IASetVertexBuffers(0, 1, buffer, &stride, &offset);
		context->IASetIndexBuffer(apiAsset->m_debugMasterIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

		context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

		int cvCount = inst->m_asset->m_numMasterStrandControlVertices;
		context->DrawIndexed( cvCount*2, 0, 0 );
	}

	return NV_OK;
}

/* static */Result Dx11DebugRender::drawGuideHairControlVertices(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	//context->RSSetState(glob->m_rasterizerStateSolid);
	Instance* inst = apiInst->m_instance;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;
	// set shader resource
	{
		ID3D11ShaderResourceView* srvs[] = 
		{ 
			apiInst->m_masterStrandSrv,		
			apiInst->m_masterStrandLuminanceSrv,
			apiAsset->m_debugVertexToHairSrv
		};
		context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	}
	float sceneUnit = asset->getUnitInCentimeters();
	// set constant buffer
	{
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(0.1, 0.1, 1, 1, dst);
		dst.hairWidth = 0.5f / sceneUnit;
		dst.aspect = viewInfo.m_aspect;
		
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst, true));
	}

	// draw cvs
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_DEBUG_CV], context);
		
		// we use vertex data from master strand SRV directly coming from the compute shader
	    unsigned int stride = 0;
		unsigned int offset = 0;
		ID3D11Buffer* buffer[] = { NV_NULL };
		
		context->IASetVertexBuffers(0, NV_COUNT_OF(buffer), buffer, &stride, &offset);
		context->IASetIndexBuffer(apiAsset->m_debugMasterIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

		context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

		int cvCount = asset->m_numMasterStrandControlVertices;
		context->Draw(cvCount + 1, 0 );
	}
	//context->RSSetState(glob->m_rasterizerStateWireFrame);
	return NV_OK;
}

/* static */Result Dx11DebugRender::drawFrames(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	const Asset* asset = inst->m_asset;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;

	// set shader resource
	ID3D11ShaderResourceView* srvs[] = 
	{ 
		apiInst->m_masterStrandSrv,		
		apiInst->m_masterFramesSrv,
		apiAsset->m_debugVertexToHairSrv
	};
	context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);

	// set constant buffer
	{
		float sceneUnit = asset->getUnitInCentimeters();
		float boneDisplayScale = 1.0f / sceneUnit;
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 0, 0, 1, dst);
		dst.scale = boneDisplayScale;
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
	}

	// draw frames
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_VISUALIZE_FRAME], context);
		
		// we use vertex data from master strand SRV directly coming from the compute shader
	    unsigned int stride = 0;
		unsigned int offset = 0;
		ID3D11Buffer* const nullBuffers[] = { NV_NULL };
		context->IASetVertexBuffers(0, NV_COUNT_OF(nullBuffers), nullBuffers, &stride, &offset);
		context->IASetIndexBuffer(apiAsset->m_debugFrameIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
		context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
		int cvCount = asset->m_numMasterStrandControlVertices;
		context->DrawIndexed( cvCount*6, 0, 0 );
	}

	{
		ID3D11ShaderResourceView* const nullSrvs[NV_COUNT_OF(srvs)] = { NV_NULL };
		context->VSSetShaderResources( 0, NV_COUNT_OF(nullSrvs), nullSrvs);
	}
	return NV_OK;
}

/* static */Result Dx11DebugRender::drawLocalPos(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;

	ID3D11ShaderResourceView* const srvs[] =
	{ 
		apiInst->m_masterStrandSrv,		
		apiInst->m_masterFramesSrv,
		apiAsset->m_prevMasterLocalPosSrv,
		apiAsset->m_nextMasterLocalPosSrv,
		apiAsset->m_debugVertexToHairSrv
	};
	// set shader resource
	context->VSSetShaderResources( 0, NV_COUNT_OF(srvs),  srvs);
	// set constant buffer
	{
		float sceneUnit = asset->getUnitInCentimeters();
		float boneDisplayScale = 1.0f / sceneUnit;

		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 0, 0, 1, dst);
		dst.scale = boneDisplayScale;
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
	}
	// draw local pos
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_VISUALIZE_LOCAL_POS], context);
		// we use vertex data from master strand SRV directly coming from the compute shader
	    unsigned int stride = 0;
		unsigned int offset = 0;
		ID3D11Buffer* nullBuffers[] = { NV_NULL };
		context->IASetVertexBuffers(0, NV_COUNT_OF(nullBuffers), nullBuffers, &stride, &offset);
		context->IASetIndexBuffer(apiAsset->m_debugLocalPosIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

		int cvCount = asset->m_numMasterStrandControlVertices;
		context->DrawIndexed( cvCount*4, 0, 0 );
	}
	{
		ID3D11ShaderResourceView* nullSrvs[NV_COUNT_OF(srvs)] = {NV_NULL};
		context->VSSetShaderResources( 0, NV_COUNT_OF(nullSrvs), nullSrvs);
	}
	return NV_OK;
}

/* static */Result Dx11DebugRender::drawNormals(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	const Asset* asset = inst->m_asset;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;

	// set shader resource
	ID3D11ShaderResourceView* const srvs[] = 
	{ 
		apiInst->m_masterStrandSrv,		
		apiInst->m_masterStrandNormalSrv,
		apiAsset->m_debugVertexToHairSrv
	};
	context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);

	// set constant buffer
	{
		float sceneUnit = asset->getUnitInCentimeters();
		float boneDisplayScale = 1.0f / sceneUnit;

		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 0, 0, 1, dst);
		dst.scale = boneDisplayScale;
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
	}
	// draw normals
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_VISUALIZE_NORMAL], context);
		// we use vertex data from master strand SRV directly coming from the compute shader
	    unsigned int stride = 0;
		unsigned int offset = 0;
		ID3D11Buffer* nullBuffers[] = { NV_NULL };
		context->IASetVertexBuffers(0, NV_COUNT_OF(nullBuffers), nullBuffers, &stride, &offset);
		context->IASetIndexBuffer(apiAsset->m_debugNormalIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

	    context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );
		int numCvs = asset->m_numMasterStrandControlVertices;
		context->DrawIndexed( numCvs * 2, 0, 0 );
	}
	{
		ID3D11ShaderResourceView* nullSrvs[NV_COUNT_OF(srvs)] = { NV_NULL };
		context->VSSetShaderResources(0, NV_COUNT_OF(nullSrvs), nullSrvs);
	}
	return NV_OK;
}

/* static */Result Dx11DebugRender::drawHairInteraction(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;

	ID3D11ShaderResourceView* const srvs[] = 
	{ 
		apiInst->m_masterStrandSrv,
		apiAsset->m_masterStrandInteractionIndicesSrv,
		apiAsset->m_masterStrandInteractionOffsetSrv,
		apiAsset->m_masterStrandInteractionLengthSrv,
	};

	// set shader resource
	context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	context->GSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	// set constant buffer
	{
		const float sceneUnit = asset->getUnitInCentimeters();

		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setHairMinMax(apiInst, dst);
		_setColor(1, 1, 0, 1, dst);
		dst.hairWidth = 0.5f / sceneUnit;
		dst.aspect = viewInfo.m_aspect;
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst, true));
	}

	// draw interaction lines
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_DEBUG_INTERACTION], context);
		
		// we use vertex data from master strand SRV directly coming from the compute shader
	    unsigned int stride = 0;
		unsigned int offset = 0;
		ID3D11Buffer* nullBuffers[] = { NV_NULL };
		
		context->IASetVertexBuffers(0, NV_COUNT_OF(nullBuffers), nullBuffers, &stride, &offset);
		context->IASetIndexBuffer( NV_NULL, DXGI_FORMAT_R32_UINT, 0 );
		context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
		context->Draw(asset->m_numMasterStrandControlVertices, 0 );
	}
	return NV_OK;
}

/* static */Result Dx11DebugRender::drawGrowthMesh(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	const Dx11ApiAsset* apiAsset = apiInst->m_apiAsset;
	const Asset* asset = inst->m_asset;

	// set constant buffer
	{
		NvHair_VisualizeConstantBuffer dst;
		_setMatrices(inst, viewInfo, dst);
		_setColor(1, 1, 0, 1, dst);
		NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
	}

	{
		ID3D11ShaderResourceView* const srvs[] =
		{
			apiInst->m_growthMeshVertexSrv,
		};
		context->VSSetShaderResources(0, NV_COUNT_OF(srvs), srvs);
	}

	// draw growth mesh
	{
		Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_BODY_DEBUG], context);
		{
			unsigned int stride = 0;
			unsigned int offset = 0;
			ID3D11Buffer* nullBuffers[] = { NV_NULL };

			context->IASetVertexBuffers( 0, NV_COUNT_OF(nullBuffers), nullBuffers, &stride, &offset );
			context->IASetIndexBuffer( apiAsset->m_growthMeshIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
			context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			context->DrawIndexed( asset->m_numFaces * 3, 0, 0 );
		}
	}
	return NV_OK;
}

/* static */Result Dx11DebugRender::drawPinConstraints(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	
	const gfsdk_float4x4& modelToWorld = inst->m_modelToWorld;
	// set shader resource
	{
		ID3D11ShaderResourceView* srvs[] = 
		{ 
			apiInst->m_pinScratchSrv,
			apiInst->m_pinsSrv,
		};
		context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	}

	Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_VISUALIZE_PIN], context);
	const float colors[12][3] = 
	{
		{1.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{0.0f,0.0f,1.0f},
		{1.0f,1.0f,0.0f},
		{1.0f,0.0f,1.0f},
		{0.0f,1.0f,1.0f},
		{0.5f,0.0f,0.0f},
		{0.0f,0.5f,0.0f},
		{0.0f,0.0f,0.5f},
		{1.0f,0.5f,0.0f},
		{1.0f,0.0f,0.5f},
		{0.0f,1.0f,0.5f}
	};

	for (IndexT i = 0; i < apiInst->m_pins.getSize(); i++)
	{
		// set constant buffer
		{
			NvHair_VisualizeConstantBuffer dst;
			dst.modelToWorld = modelToWorld;
			dst.viewProjection = viewInfo.m_viewProjectionMatrix;

			//GFSDK_HAIR_SetVisualzeColor(cbuffer, 1, 0, 1, 1);
			_setColor(colors[i%12][0], colors[i%12][1], colors[i%12][2], 1, dst);
			dst.pinId = int(i);
			dst.scale = apiInst->m_pins[i].radius;
			NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
		}
		_drawSphere(glob);
	}
	return NV_OK;
}

#if 0
/* static */Result Dx11DebugRender::drawHairParticles(Dx11ApiInstance* apiInst, const ViewInfo& viewInfo)
{
	// NUT
	Dx11ApiGlobal* glob = apiInst->m_apiGlobal;
	ID3D11DeviceContext* context = glob->getContext();
	Instance* inst = apiInst->m_instance;
	
	const gfsdk_float4x4& modelToWorld = inst->m_modelToWorld;

	// set shader resource
	{
		ID3D11ShaderResourceView* srvs[] = 
		{ 
			apiInst->m_pinScratchSrv,
			apiInst->m_masterStrandSrv,
			apiInst->m_masterStrandRadiusSrv
		};
		context->VSSetShaderResources( 0, NV_COUNT_OF(srvs), srvs);
	}
	Dx11ScopeShaderPass scope(glob->m_passes[Dx11ApiGlobal::PASS_HAIR_PARTICLE], context);
	const float colors[12][3] = 
	{
		{1.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{0.0f,0.0f,1.0f},
		{1.0f,1.0f,0.0f},
		{1.0f,0.0f,1.0f},
		{0.0f,1.0f,1.0f},
		{0.5f,0.0f,0.0f},
		{0.0f,0.5f,0.0f},
		{0.0f,0.0f,0.5f},
		{1.0f,0.5f,0.0f},
		{1.0f,0.0f,0.5f},
		{0.0f,1.0f,0.5f}
	};

	for (Int i = 0; i < apiInst->m_numMasterStrandControlVertices; i++)
	{
		float radius		= 1.0f;
		gfsdk_float4x4 scale, model;

		gfsdk_makeScale(scale, gfsdk_makeFloat3(radius, radius, radius));
		model = scale * modelToWorld;

		// set constant buffer
		{
			NvHair_VisualizeConstantBuffer dst;
			dst.modelToWorld = model;
			dst.viewProjection = viewInfo.m_viewProjectionMatrix;
			//GFSDK_HAIR_SetVisualzeColor(cbuffer, 1, 0, 1, 1);
			_setColor(colors[i%12][0], colors[i%12][1], colors[i%12][2], 1, dst);
			dst.pinId = i;
			NV_RETURN_ON_FAIL(_bindConstantBuffer(glob, apiInst->m_debugVsConstantBuffer, dst));
		}
		_drawSphere(glob);
	}
	return NV_OK;
}
#endif

} // namespace HairWorks 
} // namespace nvidia 