/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_MSAA_COMPOSER_H
#define NV_HAIR_MSAA_COMPOSER_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/Common/NvCoApiHandle.h>

namespace nvidia {
namespace HairWorks {

class MsaaComposer
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(MsaaComposer);

	virtual Result prepare(const NvCo::ApiContext& contextIn, Int sampleCount, Bool depthComparisonLess, const NvCo::ConstApiPtr& other) = 0;

	virtual Result startRendering() = 0;
	virtual Result resolveDepth(const NvCo::ApiHandle& srcSrv, const NvCo::ApiHandle& targetDsv) = 0;
	virtual Result finishRendering() = 0;
	virtual Result drawPostDepth(bool emitPartialFramgment) = 0;
	virtual Result drawColor() = 0;

	virtual Result upsampleDepthBuffer() = 0;
	virtual Result bindConstantBuffer(bool emitPartialFramgment = true) = 0;
	
	virtual void setBlendState() = 0;
	virtual void restoreBlendState() = 0;

	virtual void setSampleCount(Int sampleCount) = 0;
	virtual Int getSampleCount() const = 0;

	virtual ~MsaaComposer() {}
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_MSAA_COMPOSER_H

