/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX11_MSAA_COMPOSER_H
#define NV_HAIR_DX11_MSAA_COMPOSER_H

#include <dxgi.h>
#include <d3d11.h>

#include <Nv/HairWorks/Internal/NvHairMsaaComposer.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include "NvHairDx11ShaderPass.h"

namespace nvidia {
namespace HairWorks {

class Dx11MsaaComposer: public MsaaComposer
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx11MsaaComposer, MsaaComposer);

	enum PassType
	{
		PASS_RESOLVE,
		PASS_POST,
		PASS_POST_COLOR,
		PASS_POST_DEPTH,
		PASS_DEPTH_COPY,
		PASS_UPSAMPLE,
		PASS_COUNT_OF,
	};

	struct ConstantBuffer
	{
		float m_msaaWidth;
		float m_msaaHeight;
		float m_msaaTopLeftX;
		float m_msaaTopLeftY;

		float m_userWidth;
		float m_userHeight;
		float m_userTopLeftX;
		float m_userTopLeftY;

		int m_sampleCountUser;
		int	m_sampleCountMsaa;
		int m_depthComparisonLess;
		int m_emitPartialFragment;
	};
	
	// MsaaComposer
	
	virtual Result prepare(const NvCo::ApiContext& contextIn, Int sampleCount, Bool depthComparisonLess, const NvCo::ConstApiPtr& other) NV_OVERRIDE;
	virtual Result startRendering() NV_OVERRIDE;
	virtual Result resolveDepth(const NvCo::ApiHandle& sourceSrv, const NvCo::ApiHandle& targetDsv) NV_OVERRIDE;
	virtual Result finishRendering() NV_OVERRIDE;
	virtual Result drawPostDepth(bool emitPartialFramgment) NV_OVERRIDE;
	virtual Result drawColor() NV_OVERRIDE;

	virtual Result upsampleDepthBuffer() NV_OVERRIDE;
	virtual Result bindConstantBuffer(bool emitPartialFramgment = true) NV_OVERRIDE;
	
	virtual void setBlendState() NV_OVERRIDE;
	virtual void restoreBlendState() NV_OVERRIDE;

	virtual void setSampleCount(Int sampleCount) NV_OVERRIDE;
	virtual Int getSampleCount() const NV_OVERRIDE;

	Result init(ID3D11Device*device, ID3D11DeviceContext* context);

		/// Ctor
	Dx11MsaaComposer();
		/// Dtor
	~Dx11MsaaComposer();

	// Public members
	Int m_sampleCountMsaa;  // MSAA sample count
	bool m_depthComparisonLess;
	D3D11_VIEWPORT	m_userViewport; // viewport for user render target

protected:
	static void _clearShaderResources(ID3D11DeviceContext* context);

	static Result _createDepthBufferSrv(ID3D11Device* device, ID3D11Texture2D* depthBuffer, ID3D11ShaderResourceView** srvOut);
	static Result _createDepthBufferSrv(ID3D11Device* device, ID3D11DepthStencilView* dsv, ID3D11ShaderResourceView** depthBufferSrvOut, UINT& sampleCountUser);
	static Result _createDepthBufferDsv(ID3D11Device* device, ID3D11Texture2D* depthBuffer, ID3D11DepthStencilView** dsvOut);

	static D3D11_TEXTURE2D_DESC _getDepthBufferDesc(UINT w, UINT h, DXGI_FORMAT format, UINT sampleCount);
	static D3D11_TEXTURE2D_DESC _getColorBufferDesc(UINT w, UINT h, UINT sampleCount);

	void _setRenderState(const Dx11ShaderPass& pass);

	NvCo::ComPtr<ID3D11Texture2D> m_colorBufferMsPost; // MSAA color buffer
	NvCo::ComPtr<ID3D11Texture2D> m_colorBufferPost; // non-MSAA color resolve buffer

	NvCo::ComPtr<ID3D11Texture2D> m_depthBufferMsPost; // MSAA depth buffer (post render)
	NvCo::ComPtr<ID3D11Texture2D> m_depthBufferPost; // non-MSAA post render depth resolve buffer

	NvCo::ComPtr<ID3D11RenderTargetView> m_colorBufferMsPostRtv; // MSAA render target
	NvCo::ComPtr<ID3D11RenderTargetView> m_colorBufferPostRtv; // MSAA render target

	NvCo::ComPtr<ID3D11DepthStencilView> m_depthBufferMsPostDsv; // MSAA depth buffer DSV
	NvCo::ComPtr<ID3D11DepthStencilView> m_depthBufferPostDsv;

	NvCo::ComPtr<ID3D11ShaderResourceView> m_colorBufferPostSrv;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_depthBufferMsPostSrv;
	NvCo::ComPtr<ID3D11ShaderResourceView> m_depthBufferPostSrv;

	NvCo::ComPtr<ID3D11Buffer> m_constantBuffer; // constant buffer for custom resolve

	NvCo::ComPtr<ID3D11RenderTargetView> m_userRtv; // user render target view
	NvCo::ComPtr<ID3D11DepthStencilView> m_userDsv; // user depth stencil view

	NvCo::ComPtr<ID3D11DepthStencilState>	m_depthStencilStateNone;
	NvCo::ComPtr<ID3D11DepthStencilState>	m_depthStencilStateWriteOnly;

	NvCo::ComPtr<ID3D11BlendState> m_blendStateNoBlending;
	NvCo::ComPtr<ID3D11BlendState> m_blendStateAlphaBlending;
	NvCo::ComPtr<ID3D11BlendState> m_blendStateAlphaBlendingDeferred;

	NvCo::ComPtr<ID3D11RasterizerState> m_rasterizerStateSolid;

	D3D11_VIEWPORT	m_viewportMsaa; // viewport for MSAA render target
	
	Dx11ShaderPass m_passes[PASS_COUNT_OF];

	UINT m_sampleCountUser; // user buffer sample count
	
	UINT m_numBlurs;

	UINT m_msaaBufferWidth;
	UINT m_msaaBufferHeight;

	NvCo::ComPtr<ID3D11BlendState> m_blendStatePrev;
	float m_blendFactorPrev[4];
	UINT m_sampleMaskPrev;

	ID3D11Device* m_device;
	ID3D11DeviceContext* m_context;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX11_MSAA_COMPOSER_H

