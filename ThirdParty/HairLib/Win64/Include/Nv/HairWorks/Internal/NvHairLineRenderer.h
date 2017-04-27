/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_LINE_RENDERER_H
#define NV_HAIR_LINE_RENDERER_H

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>
#include <Nv/Common/NvCoWeakComPtr.h>
#include <Nv/Common/NvCoApiHandle.h>

#include <Nv/HairWorks/NvHairSdk.h>

#include <Nv/HairWorks/Internal/Legacy/GfsdkMathUtil.h>

namespace nvidia {
namespace HairWorks { 

class LineRenderer
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(LineRenderer);

	enum {STACK_SIZE = 8 };
	
	struct Scope
	{
		Scope(LineRenderer* renderer): 
			m_renderer(renderer) 
		{ 
			renderer->push(); 
		}
		~Scope() { m_renderer->pop(); }
	protected:
		LineRenderer* m_renderer;
	};

		/// Called before any rendering. 
	virtual Result start(const NvCo::ApiContext& context, const gfsdk_float4x4& projection) = 0;
		
		/// Set the model to world matrix
	void setModelToWorldMatrix(const gfsdk_float4x4& modelToWorld);
		/// Get the model to world matrix
	NV_FORCE_INLINE const gfsdk_float4x4& getModelToWorldMatrix() const { return m_modelToWorld; }

		/// Set the overall color. The final color will be global color multiplied by per line color. 
	Void setGlobalColor(const gfsdk_float4& color);
		/// Get the current set global color
	const gfsdk_float4& getGlobalColor() const { return m_globalColor; }
		/// Set the per line color
	Void setColor(const gfsdk_float3& color);

		/// Get the current depth op
	EDepthOp getDepthOp() const { return m_depthOp; }
	Void setDepthOp(EDepthOp op);

		/// True if successfully started, such that drawing can occur. If false drawing cannot occur, and an implementation 
		/// may asset.
	NV_FORCE_INLINE Bool isStarted() const { return m_isStarted; }

	const gfsdk_float3& getColor() const { return m_color; }
		/// Draw a line from specified start and end points
	virtual Void drawLine(const gfsdk_float3& start, const gfsdk_float3& end) = 0;
		/// Draws a unit sphere with the model matrix, and the global color
	virtual Void drawSphere() = 0;
		/// End 
	virtual Void end() = 0;

		/// Called immediately after when work has been submitted to gpu
	virtual Void onGpuWorkSubmitted(const NvCo::ApiHandle& handle) = 0;

		/// Draw an axis (unit length with colored edges, r = x, g = y, b = z) placed on model origin.
	Void drawAxis();
		/// Draw axis of transform scaled by scale, placed at position defined in transform.
	Void drawAxis(const gfsdk_float4x4& transform, const gfsdk_float3& scale);
		/// Draw the bounding box 
	Void drawBoundingBox(const gfsdk_float3& bbMin, const gfsdk_float3& bbMax);

		/// Pop rendering settings.
	Void pop();
		/// Push rendering settings.
	Void push();

		/// 
	virtual ~LineRenderer() {}

protected:
	LineRenderer();

		/// Called when the global color changes (when started)
		/// Ie the code to handle device specific 
	virtual Void onGlobalColorChange() = 0;
		/// Called when the per line color changes
	virtual Void onColorChange() = 0;
		/// Called when model to world Matrix changes
	virtual Void onModelToWorldChange() = 0;
		/// Called when the depth op changes
	virtual Void onDepthOpChange() = 0;

	EDepthOp m_depthOp;
	gfsdk_float4x4 m_modelToWorld;
	gfsdk_float4 m_globalColor;			///< The overall color
	gfsdk_float3 m_color;				///< The line color	

	Bool m_isStarted;					///< The state is set

	struct Entry
	{
		EDepthOp m_depthOp;
		gfsdk_float4x4 m_modelToWorld;
		gfsdk_float4 m_globalColor;			///< The overall color
		gfsdk_float3 m_color;				///< The line color	
	};

	Entry m_stack[STACK_SIZE];
	Int m_stackSize;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_LINE_RENDERER_H
