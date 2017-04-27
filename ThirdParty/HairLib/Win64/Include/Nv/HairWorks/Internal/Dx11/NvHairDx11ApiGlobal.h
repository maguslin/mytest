/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_API_GLOBAL_H
#define NV_HAIR_DX11_API_GLOBAL_H

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoPodBuffer.h>

#include <Nv/HairWorks/Internal/NvHairSliSystem.h>
#include <Nv/HairWorks/Internal/NvHairApiGlobal.h>
#include <Nv/HairWorks/Internal/NvHairLineRenderer.h>

#include "NvHairDx11ShaderPass.h"
#include "NvHairDx11RenderState.h"
#include "NvHairDx11ShaderCacheEntry.h"
#include "NvHairDx11DepthOpState.h"

namespace nvidia {
namespace HairWorks { 

class Dx11ApiGlobal: public ApiGlobal
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11ApiGlobal, ApiGlobal);

	enum PassType
	{
		PASS_SPLINE,

		PASS_INTERPOLATE,
		PASS_CUBE_MAP_INTERPOLATE,

		PASS_DEBUG_GUIDE,
		PASS_DEBUG_CV,
		PASS_DEBUG_INTERACTION,
		PASS_BODY_DEBUG,

		PASS_VISUALIZE_FRAME,
		PASS_VISUALIZE_LOCAL_POS,
		PASS_VISUALIZE_NORMAL,
		PASS_VISUALIZE_PIN,
		//PASS_HAIR_PARTICLE,  // NUTT

		PASS_COUNT_OF,
	};

		/// ApiGlobal
	virtual Result bindAsset(Asset& asset) NV_OVERRIDE;
	virtual Result bindInstance(Instance& inst) NV_OVERRIDE;
	virtual Result setContext(const NvCo::ApiContext& context) NV_OVERRIDE;	
	virtual MsaaComposer* createMsaaComposer() NV_OVERRIDE;
	virtual Result init(const NvCo::ApiDevice& device, const NvCo::ApiContext& context, const NvCo::ConstApiPtr& other) NV_OVERRIDE;
	virtual Void saveRenderState() NV_OVERRIDE;
	virtual Void restoreRenderState() NV_OVERRIDE;
	virtual Void setDepthOp(EDepthOp depthOp) NV_OVERRIDE;
	virtual Void setRasterizerMode(RasterizerMode mode) NV_OVERRIDE;
	virtual Void applySamplers(bool useVs, bool useHs, bool useDs, bool useGs, bool usePs) NV_OVERRIDE;
	virtual Bool canRender() NV_OVERRIDE;
	virtual Result calcViewInfo(const Viewport& viewport, const gfsdk_float4x4& view, const gfsdk_float4x4& projection, float fov, ViewInfo& viewInfoOut) NV_OVERRIDE;
	virtual Bool areEqual(const NvCo::ApiHandle& a, const NvCo::ApiHandle& b) NV_OVERRIDE;
	virtual Void onGpuWorkSubmitted(const NvCo::ApiHandle& handle) NV_OVERRIDE;
	virtual Result setPixelShader(Int shaderIndex, const NvCo::ConstApiPtr& pixelShaderInfo) NV_OVERRIDE;

	/// Enable a specific pass
	void enable(PassType passType, ID3D11DeviceContext* context) { m_passes[passType].enable(context); }

		/// Get the device
	NV_FORCE_INLINE ID3D11Device* getDevice() const { return m_device; }
		/// Get the context
	NV_FORCE_INLINE ID3D11DeviceContext* getContext() const { return m_context; }

		/// Ctor
	Dx11ApiGlobal();
		/// Dtor
	virtual ~Dx11ApiGlobal();

protected:

	Result _createShaders();
	Result _createVisualizationVertexBuffers();
	Result _createScalarNoiseLut();
	Result _createVectorNoiseLut();
	Result _createStrandBarycentricCoordinates();

	ID3D11Device* m_device;
	ID3D11DeviceContext* m_context;

public:
	Dx11ShaderPass m_passes[PASS_COUNT_OF];

	NvCo::ComPtr<ID3D11ComputeShader> m_hairPrepareInterpolate;
	NvCo::ComPtr<ID3D11PixelShader> m_hairSimpleShader;
	NvCo::ComPtr<ID3D11PixelShader> m_hairShadowShader;

	NvCo::ComPtr<ID3D11ComputeShader> m_hairSimulate;
	NvCo::ComPtr<ID3D11ComputeShader> m_hairSimulateInteraction;
	NvCo::ComPtr<ID3D11ComputeShader> m_hairSimulatePinPass;
	NvCo::ComPtr<ID3D11ComputeShader>	m_hairSimulatePinComPass;
	NvCo::ComPtr<ID3D11ComputeShader> m_hairSimulatePinComGatherPass;

	NvCo::ComPtr<ID3D11ComputeShader> m_computeStats;

	NvCo::ComPtr<ID3D11DepthStencilState> m_depthStencilStateLess;
	NvCo::ComPtr<ID3D11DepthStencilState> m_depthStencilStateGreater;
	NvCo::ComPtr<ID3D11DepthStencilState> m_depthStencilStateNone;
	NvCo::ComPtr<ID3D11DepthStencilState> m_depthStencilStateTestOnlyLess;
	NvCo::ComPtr<ID3D11DepthStencilState> m_depthStencilStateTestOnlyGreater;

	NvCo::ComPtr<ID3D11RasterizerState> m_rasterizerStateSolid;
	NvCo::ComPtr<ID3D11RasterizerState> m_rasterizerStateWireFrame;

	NvCo::ComPtr<ID3D11SamplerState> m_linearSampler;
	NvCo::ComPtr<ID3D11SamplerState> m_comparisonSampler;
	NvCo::ComPtr<ID3D11SamplerState> m_pointClampSampler;

	Bool m_hasRenderState;			///< True if has stored render state
	Dx11RenderState m_renderState;	///< For storing render state

	Dx11DepthOpState m_depthOpState;	///< Used for setting depth op state

	// Shader Cache factory
	NvCo::UniquePtr<Dx11ShaderCacheFactory> m_shaderCacheFactory;

	// For debug rendering

	// bone visualization data
	NvCo::ComPtr<ID3D11Buffer> m_sphereVertexBuffer;		//< The vertex buffer holding a unit sphere (float4s)
	Int m_numSphereLines;							//< Two vertices for every line

	// general noise table (1D)
	NvCo::ComPtr<ID3D11Buffer> m_scalarNoiseBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_scalarNoiseSrv;

	// barycentric coordinate LUT (2D noise)
	NvCo::ComPtr<ID3D11Buffer> m_strandCoordinatesBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_strandCoordinatesSrv;

	// wind noise table (3D vector noise)
	NvCo::ComPtr<ID3D11Buffer> m_vectorNoiseBuffer;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_vectorNoiseSrv;
};

} // namespace HairWorks
} // namespace nvidia

#endif // NV_HAIR_DX11_API_GLOBAL_H
