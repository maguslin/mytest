/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/Common/NvCoCommon.h>

#include "NvHairLineRenderer.h"

namespace nvidia {
namespace HairWorks {

LineRenderer::LineRenderer()
{
	m_globalColor = gfsdk_makeFloat4(1.0f, 1.0f, 1.0f, 1.0f);
	m_color = gfsdk_makeFloat3(1.0f, 1.0f, 1.0f);
	gfsdk_makeIdentity(m_modelToWorld);
	m_depthOp = DepthOp::WRITE_LESS;
	m_isStarted = false;

	m_stackSize = 0;
}

Void LineRenderer::pop()
{
	NV_CORE_ASSERT(m_stackSize > 0);
	const Entry& entry = m_stack[--m_stackSize];
	setColor(entry.m_color);
	setGlobalColor(entry.m_globalColor);
	setModelToWorldMatrix(entry.m_modelToWorld);
	setDepthOp(entry.m_depthOp);
}

void LineRenderer::push()
{
	NV_CORE_ASSERT(m_stackSize < STACK_SIZE);
	Entry& entry = m_stack[m_stackSize++];
	entry.m_color = m_color;
	entry.m_depthOp = m_depthOp;
	entry.m_globalColor = m_globalColor;
	entry.m_modelToWorld = m_modelToWorld;
}

Void LineRenderer::setColor(const gfsdk_float3& color)
{
	if (m_isStarted)
	{ 
		if (!gfsdk_equal(color, m_color))
		{
			m_color = color;
			onColorChange();
		}
	}
	else
	{
		m_color = color;
	}
}

Void LineRenderer::setGlobalColor(const gfsdk_float4& color)
{
	if (m_isStarted)
	{
		if (!gfsdk_equal(color, m_globalColor))
		{
			m_globalColor = color;
			onGlobalColorChange();
		}
	}
	else
	{
		m_globalColor = color;
	}
}

void LineRenderer::setModelToWorldMatrix(const gfsdk_float4x4& modelToWorld)
{
	if (m_isStarted)
	{ 
		if (!gfsdk_equal(modelToWorld, m_modelToWorld))
		{
			m_modelToWorld = modelToWorld;
			onModelToWorldChange();
		}
	}
	else
	{
		m_modelToWorld = modelToWorld;
	}
}

Void LineRenderer::setDepthOp(EDepthOp op)
{
	if (m_isStarted)
	{
		if (m_depthOp != op)
		{
			m_depthOp = op;
			onDepthOpChange();
		}
	}
	else
	{
		m_depthOp = op;
	}
}

Void LineRenderer::drawAxis()
{
	gfsdk_float3 color = getColor();

	setColor(gfsdk_makeFloat3(1, 0, 0));
	drawLine(gfsdk_makeFloat3(0, 0, 0), gfsdk_makeFloat3(1, 0, 0));
	setColor(gfsdk_makeFloat3(0, 1, 0));
	drawLine(gfsdk_makeFloat3(0, 0, 0), gfsdk_makeFloat3(0, 1, 0));
	setColor(gfsdk_makeFloat3(0, 0, 1));
	drawLine(gfsdk_makeFloat3(0, 0, 0), gfsdk_makeFloat3(0, 0, 1));
	
	setColor(color);
}

Void LineRenderer::drawAxis(const gfsdk_float4x4& transform, const gfsdk_float3& scale)
{
	gfsdk_float3 x = gfsdk_getRow(transform, 0) * scale.x;
	gfsdk_float3 y = gfsdk_getRow(transform, 1) * scale.y;
	gfsdk_float3 z = gfsdk_getRow(transform, 2) * scale.z;
	gfsdk_float3 pos = gfsdk_getRow(transform, 3);

	gfsdk_float3 color = getColor();

	setColor(gfsdk_makeFloat3(1, 0, 0));
	drawLine(pos, pos + x);
	setColor(gfsdk_makeFloat3(0, 1, 0));
	drawLine(pos, pos + y);
	setColor(gfsdk_makeFloat3(0, 0, 1));
	drawLine(pos, pos + z);

	setColor(color);
}

Void LineRenderer::drawBoundingBox(const gfsdk_float3& bbMin, const gfsdk_float3& bbMax)
{
	// x-axis lines
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMin.y, bbMin.z), gfsdk_makeFloat3(bbMax.x, bbMin.y, bbMin.z));
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMax.y, bbMin.z), gfsdk_makeFloat3(bbMax.x, bbMax.y, bbMin.z));
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMax.y, bbMax.z), gfsdk_makeFloat3(bbMax.x, bbMax.y, bbMax.z));
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMin.y, bbMax.z), gfsdk_makeFloat3(bbMax.x, bbMin.y, bbMax.z));	
	// y-axis lines
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMin.y, bbMin.z), gfsdk_makeFloat3(bbMin.x, bbMax.y, bbMin.z));
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMin.y, bbMax.z), gfsdk_makeFloat3(bbMin.x, bbMax.y, bbMax.z)); 
	drawLine(gfsdk_makeFloat3(bbMax.x, bbMin.y, bbMax.z), gfsdk_makeFloat3(bbMax.x, bbMax.y, bbMax.z));
	drawLine(gfsdk_makeFloat3(bbMax.x, bbMin.y, bbMin.z), gfsdk_makeFloat3(bbMax.x, bbMax.y, bbMin.z));
	// z-axis lines
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMin.y, bbMin.z), gfsdk_makeFloat3(bbMin.x, bbMin.y, bbMax.z));
	drawLine(gfsdk_makeFloat3(bbMin.x, bbMax.y, bbMin.z), gfsdk_makeFloat3(bbMin.x, bbMax.y, bbMax.z));
	drawLine(gfsdk_makeFloat3(bbMax.x, bbMax.y, bbMin.z), gfsdk_makeFloat3(bbMax.x, bbMax.y, bbMax.z)); 
	drawLine(gfsdk_makeFloat3(bbMax.x, bbMin.y, bbMin.z), gfsdk_makeFloat3(bbMax.x, bbMin.y, bbMax.z));
}

} // namespace HairWorks 
} // namespace nvidia 