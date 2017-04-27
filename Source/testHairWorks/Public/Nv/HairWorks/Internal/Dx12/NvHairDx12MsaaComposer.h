/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_DX12_MSAA_COMPOSER_H
#define NV_HAIR_DX12_MSAA_COMPOSER_H

#include <dxgi.h>
#include <d3d12.h>

#include <Nv/HairWorks/Internal/NvHairMsaaComposer.h>

#include <Nv/HairWorks/Internal/NvHairInternal.h>

#include <Nv/HairWorks/Internal/Dx12/NvHairDx12ApiGlobal.h>

#include "NvHairDx12ShaderPass.h"

namespace nvidia {
namespace HairWorks {

class Dx12MsaaComposer: public MsaaComposer
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS(Dx12MsaaComposer, MsaaComposer);

	typedef NvCo::Dx12DescriptorSet DescriptorSet;

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

	Result init(ID3D12Device* device, Dx12ApiGlobal* glob);

		/// Ctor
	Dx12MsaaComposer();
		/// Dtor
	~Dx12MsaaComposer();

	// Public members
	Int m_sampleCountMsaa;  // MSAA sample count
	bool m_depthComparisonLess;
	D3D12_VIEWPORT	m_userViewport; // viewport for user render target

protected:
	Dx12RenderTargetView m_colorMsPost;
	Dx12RenderTargetView m_colorPost;

	Dx12DepthStencilView m_depthMsPost;
	Dx12DepthStencilView m_depthPost;

	Dx12DepthStencilView m_dsvUser;

	const Dx12MsaaInfo* m_msaaInfo;

	NvCo::Dx12CircularResourceHeap::Cursor m_cursorOut;

	D3D12_VIEWPORT	m_viewportMsaa; // viewport for MSAA render target
	
	Dx12ShaderPass m_passes[PASS_COUNT_OF];

	Int m_sampleCountUser; // user buffer sample count
	
	Int m_numBlurs;

	Int m_msaaBufferWidth;
	Int m_msaaBufferHeight;

	Dx12ApiGlobal* m_apiGlobal;

	ID3D12Device* m_device;
	ID3D12GraphicsCommandList* m_commandList;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_DX12_MSAA_COMPOSER_H
