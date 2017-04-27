/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_API_INSTANCE_H
#define NV_HAIR_API_INSTANCE_H

#include "NvHairApiGlobal.h"
#include <Nv/Common/Container/NvCoHandleMap.h>

namespace nvidia {
namespace HairWorks { 

struct RenderViewInfo;

class ApiInstance
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(ApiInstance);

	/*! Defines the available debug draw types */
	class DebugDraw { DebugDraw(); public: enum Enum 
	{
		GUIDE_HAIRS,
		SKINNED_GUIDE_HAIRS,
		FRAMES,
		NORMALS,
		LOCAL_POS,
		HAIR_INTERACTION,
		GUIDE_HAIR_CONTROL_VERTICES,
		GROWTH_MESH,
		PIN_CONSTRAINTS,
		COUNT_OF,
	}; };
	typedef DebugDraw::Enum EDebugDraw;

		/// Compute stats
	virtual Result computeStats(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Stats& statsOut) = 0;
	virtual Result stepSimulation(float timeStep, const gfsdk_float4x4* worldReference) = 0;
	virtual Result renderHairShading(const RenderViewInfo& frameViewInfo, ShaderSettings& shaderSettings, const NvCo::ConstApiPtr& other) = 0;
	virtual Void calcPixelConstantBuffer(const RenderViewInfo& frameViewInfo, ShaderConstantBuffer& constantBufferOut) = 0;

		/// True if the texture is set/used
	virtual Bool isApiTextureUsed(ETextureType type) = 0;
	
	virtual Result setApiTexture(ETextureType type, const NvCo::ApiHandle& texture) = 0;
	virtual Result getApiTextures(const ETextureType* types, Int numTextures, const NvCo::ApiPtr& textureOut) = 0;

	virtual Result getApiResources(const EShaderResourceType* types, Int numResources, const NvCo::ApiPtr& resourceOut) = 0;

	virtual Result getPinMatrix(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int pinId, gfsdk_float4x4& matrixOut) = 0;

	virtual Result getPinMatrices(NvCo::HandleMapHandle* asyncInOut, Bool asyncRepeat, Int startPinIndex, Int numPins, gfsdk_float4x4* matricesOut) = 0;

		/// Draw the specified debug info
	virtual Result debugDraw(EDebugDraw drawType, const ViewInfo& viewInfo) = 0;

	virtual Result preRender(Float simulationInterp) = 0;

	virtual Result updatePinConstraintBuffer() = 0;

		/// Called if the number of vertices per segment changed
	virtual Result onNumVertsPerSegmentChanged() = 0;

	virtual ~ApiInstance() {}

	static const char* getText(EDebugDraw drawType);
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_API_INSTANCE_H
