/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_LINE_RENDERER_H
#define NV_HAIR_DX11_LINE_RENDERER_H

#include <dxgi.h>
#include <d3d11.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoWeakComPtr.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/HairWorks/Internal/Shader/NvHairInternalShaderTypes.h>
#include <Nv/HairWorks/Internal/NvHairLineRenderer.h>

#include "NvHairDx11DepthOpState.h"
#include "NvHairDx11ShaderPass.h"

namespace nvidia {
namespace HairWorks { 

class Dx11LineRenderer: public LineRenderer
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11LineRenderer, LineRenderer);

	typedef NvHair_VisualizeConstantBuffer ConstantBuffer;

	struct Vertex
	{
		gfsdk_float3 m_position;
		UInt32 m_color;
	};

	enum { MAX_VERTICES = 1024 };

	// Implements LineRenderer
	
	virtual Result start(const NvCo::ApiContext& context, const gfsdk_float4x4& projection) NV_OVERRIDE;
	virtual Void drawLine(const gfsdk_float3& start, const gfsdk_float3& end) NV_OVERRIDE;
	virtual Void drawSphere() NV_OVERRIDE;
	virtual Void end() NV_OVERRIDE;
	virtual Void onGpuWorkSubmitted(const NvCo::ApiHandle& handle) NV_OVERRIDE;

	
		/// Ctor
	Dx11LineRenderer(Dx11DepthOpState& depthOpState);
	/// Dtor
	virtual ~Dx11LineRenderer();

	/// Initialize
	Result init(const NvCo::ApiDevice& device, const NvCo::ApiContext& context);

protected:
	
	// Implements LineRenderer
	virtual Void onGlobalColorChange() NV_OVERRIDE;
	virtual Void onColorChange() NV_OVERRIDE;
	virtual Void onModelToWorldChange() NV_OVERRIDE;
	virtual Void onDepthOpChange() NV_OVERRIDE;

	NV_FORCE_INLINE Bool _needsFlush(Int numVerts) const { return m_buffer.getSize() + numVerts > MAX_VERTICES; }
	Result _flush();
	Result _syncConstantBuffer();

	static UInt32 _getPackedColor(const gfsdk_float3& color)
	{
		Int r = Int(color.x * 255.0f);
		Int g = Int(color.y * 255.0f);
		Int b = Int(color.z * 255.0f);
#ifdef NV_LITTLE_ENDIAN
		return UInt32(r) | UInt32(g << 8) | UInt32(b << 16) | UInt32(0xff000000);
#else
		return UInt32(r << 24) | UInt32(g << 16) | UInt32(b << 8) | UInt32(0x000000ff);
#endif
	}

	Dx11ShaderPass m_pass;

	UInt32 m_packedColor;

	Bool m_constantBufferChanged;
	ConstantBuffer m_cpuConstantBuffer;

	NvCo::ComPtr<ID3D11Buffer> m_constantBuffer;
	NvCo::ComPtr<ID3D11Buffer> m_vertexBuffer;	///< 
	NvCo::Array<Vertex> m_buffer;					///< Holds points, flushed to m_vertexBuffer when full

	NvCo::ComPtr<ID3D11Buffer> m_sphereVertexBuffer;
	Int m_numSphereLines;

	Dx11DepthOpState& m_depthOpState;		///< Depth op state

	ID3D11DeviceContext* m_context;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX11_LINE_RENDERER_H
