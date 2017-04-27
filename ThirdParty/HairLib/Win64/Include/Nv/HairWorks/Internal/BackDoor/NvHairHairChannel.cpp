/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include <Nv/Common/NvCoCommon.h>

#include "NvHairHairChannel.h"

// Yuk - needed for var args
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace nvidia {
namespace HairWorks {

//#pragma warning(disable:4996)

HairChannel::HairChannel()
{
	m_logFile = NV_NULL;

	m_renderCallCount = 0;
	m_renderCount = 0;
	m_lastRenderCallCount = 0;
	m_lastRenderCount = 0;
	
	const char* env = ::getenv("NVHAIR_BACKDOOR_LOG");
	if ( env && _stricmp(env,"true") == 0 )
	{
		m_logFile = fopen("NvHairBackDoorLog.txt", "w");
	}
}

HairChannel::~HairChannel(void)
{
	if ( m_logFile )
	{
		fclose(m_logFile);
	}
}

static bool getTrueFalse(const char *v)
{
	return ( _stricmp(v,"true") == 0 || strcmp(v,"1") == 0  || _stricmp(v,"t") == 0);
}

Result HairChannel::init()
{
	// initialize the system..
	m_channel = new BackDoorChannel;	
	Result res = m_channel->init("BACKDOOR_CLIENT","BACKDOOR_SERVER");
	if (NV_FAILED(res)) return res;

	m_channel->send("Welcome! HairWorks DLL connected!");
	
	const Item items[] = 
	{
		/// hair geometry (density/width/length/clump) controls
		Item("width", m_override.m_width),
		Item("widthNoise",m_override.m_widthNoise),
		Item("widthRootScale",m_override.m_widthRootScale),
		Item("widthTipScale",m_override.m_widthTipScale),

		Item("clumpNoise",m_override.m_clumpNoise),	
		Item("clumpRoundness",m_override.m_clumpRoundness), 
		Item("clumpScale",m_override.m_clumpScale), 

		Item("density",m_override.m_density), 
		Item("usePixelDensity",m_override.m_usePixelDensity),

		Item("lengthNoise",m_override.m_lengthNoise),
		Item("lengthScale",m_override.m_lengthScale),

		Item("waveScale",m_override.m_waveScale), 
		Item("waveScaleNoise",m_override.m_waveScaleNoise),
		Item("waveScaleClump",	m_override.m_waveScaleClump),
		Item("waveStrand",	m_override.m_waveScaleStrand),
		Item("waveRootStraighten",	m_override.m_waveRootStraighten),
		Item("waveFreq",m_override.m_waveFreq),
		Item("waveFreqNoise",m_override.m_waveFreqNoise),		
		
		/// shading controls
		Item("diffuseBlend",m_override.m_diffuseBlend),

		Item("hairNormalWeight",m_override.m_hairNormalWeight),
		Item("hairNormalBoneIndex",m_override.m_hairNormalBoneIndex),

		Item("glintStrength",m_override.m_glintStrength),
		Item("glintCount",m_override.m_glintCount),
		Item("glintExponent",m_override.m_glintExponent),

		Item("specularColor",m_override.m_specularColor),
		Item("specularNoiseScale",m_override.m_specularNoiseScale),
		Item("specularEnvScale",m_override.m_specularEnvScale),

		Item("specularPrimary",m_override.m_specularPrimary),
		Item("specularPowerPrimary",m_override.m_specularPowerPrimary),
		Item("specularPrimaryBreakup",m_override.m_specularPrimaryBreakup),
		Item("specularSecondary",m_override.m_specularSecondary),
		Item("specularSecondaryOffset",m_override.m_specularSecondaryOffset),
		Item("specularPowerSecondary",m_override.m_specularPowerSecondary),
	
		Item("rootColor",m_override.m_rootColor),
		Item("tipColor",m_override.m_tipColor),
		Item("rootAlphaFalloff",m_override.m_rootAlphaFalloff),
		Item("rootTipColorWeight",m_override.m_rootTipColorWeight),
		Item("rootTipColorFalloff",m_override.m_rootTipColorFalloff),

		Item("castShadows",m_override.m_castShadows),
		Item("receiveShadows",m_override.m_receiveShadows),
		Item("shadowSigma",m_override.m_shadowSigma),

		Item("strandBlendMode",m_override.m_strandBlendMode),
		Item("strandBlendScale",m_override.m_strandBlendScale),
		
		// simulation control
		Item("backStopRadius",m_override.m_backStopRadius),
		Item("bendStiffness",m_override.m_bendStiffness),
		Item("gravityDir",m_override.m_gravityDir),
		Item("friction",m_override.m_friction),
		Item("massScale",m_override.m_massScale),
		Item("inertiaScale",m_override.m_inertiaScale),
		Item("inertiaLimit",m_override.m_inertiaLimit),
		Item("interactionStiffness",m_override.m_interactionStiffness),
		Item("rootStiffness",m_override.m_rootStiffness),
		Item("pinStiffness",m_override.m_pinStiffness),
		Item("simulate",m_override.m_simulate),
		Item("stiffness",m_override.m_stiffness),
		Item("stiffnessStrength",m_override.m_stiffnessStrength),
		Item("stiffnessDamping",m_override.m_stiffnessDamping),
		Item("tipStiffness",m_override.m_tipStiffness),

		Item("stiffnessCurve", m_override.m_stiffnessCurve),
		Item("stiffnessStrengthCurve", m_override.m_stiffnessStrengthCurve),
		Item("stiffnessDampingCurve", m_override.m_stiffnessDampingCurve),
		Item("bendStiffnessCurve", m_override.m_bendStiffnessCurve),

		Item("useCollision",m_override.m_useCollision),
		Item("useDynamicPin",m_override.m_useDynamicPin),
		Item("windNoise",m_override.m_windNoise),
		Item("wind",m_override.m_wind),

		/// LOD controls
		Item("enableLOD",m_override.m_enableLod),

		Item("enableDistanceLOD",m_override.m_enableDistanceLod),
		Item("distanceLODStart",m_override.m_distanceLodStart),
		Item("distanceLODEnd",m_override.m_distanceLodEnd),
		Item("distanceLODFadeStart",m_override.m_distanceLodFadeStart),
		Item("distanceLODDensity",m_override.m_distanceLodDensity),
		Item("distanceLODWidth",m_override.m_distanceLodWidth),

		Item("enableDetailLOD",m_override.m_enableDetailLod),
		Item("detailLODStart",m_override.m_detailLodStart),
		Item("detailLODEnd",m_override.m_detailLodEnd),
		Item("detailLODDensity",m_override.m_detailLodDensity),
		Item("detailLODWidth",m_override.m_detailLodWidth),

		Item("shadowDensityScale",m_override.m_shadowDensityScale),

		Item("useViewfrustrumCulling",m_override.m_useViewfrustrumCulling),
		Item("useBackfaceCulling",m_override.m_useBackfaceCulling),
		Item("backfaceCullingThreshold",m_override.m_backfaceCullingThreshold),

		// drawing option
		Item("drawRenderHairs",m_override.m_drawRenderHairs),
		Item("visualizeBones",m_override.m_visualizeBones),
		Item("visualizeBoundingBox",m_override.m_visualizeBoundingBox),
		Item("visualizeCapsules",m_override.m_visualizeCapsules),
		Item("visualizeControlVertices",m_override.m_visualizeControlVertices),
		Item("visualizeCullSphere",m_override.m_visualizeCullSphere),
		Item("visualizeDiffuseBone",m_override.m_visualizeShadingNormalBone),
		Item("visualizeFrames",m_override.m_visualizeFrames),
		Item("visualizeGrowthMesh",m_override.m_visualizeGrowthMesh),
		Item("visualizeGuideHairs",m_override.m_visualizeGuideHairs),
		Item("visualizeHairInteractions",m_override.m_visualizeHairInteractions),
		Item("visualizeHairSkips",m_override.m_visualizeHairSkips),
		Item("visualizePinConstraints",m_override.m_visualizePinConstraints),
		Item("visualizeShadingNormals",m_override.m_visualizeShadingNormals),
		Item("visualizeSkinnedGuideHairs",m_override.m_visualizeSkinnedGuideHairs),

		Item("colorizeMode",m_override.m_colorizeMode),
	};
	m_items.pushBack(items, NV_COUNT_OF(items));

	return NV_OK;
}

void HairChannel::_updateItem(Int argc, const char*const* argv, Item& item)
{
	if ( argc == 1 )
	{
		if ( item.m_type == Item::TYPE_BOOL )
		{
			item.m_override = true;
			*item.m_state = !*item.m_state;
		}
		else
		{
			item.m_override = !item.m_override;
		}
	}
	else if ( argc == 2 )
	{
		const char* value = argv[1];
		switch (item.m_type)
		{
			case Item::TYPE_FLOAT:
			{
				Float32 v = (Float32)atof(value);
				*item.m_float = v;
				item.m_override = true;
				break;
			}
			case Item::TYPE_INT32:
			case Item::TYPE_UINT32:
			{
				UInt32 v = (UInt32)atoi(value);
				*item.m_uint = v;
				item.m_override = true;
				break;
			}
			case Item::TYPE_FLOAT4:
			{
				Float32 v = (Float32)atof(value);
				gfsdk_float4& dst = *item.m_float4;
				dst.x = v;
				dst.y = v;
				dst.z = v;
				dst.w = v;
				item.m_override = true;
				break;
			}
			case Item::TYPE_FLOAT3:
			{
				Float32 v = (Float32)atof(value);
				gfsdk_float3& dst = *item.m_float3;
				dst.x = v;
				dst.y = v;
				dst.z = v;
				item.m_override = true;
				break;
			}
			case Item::TYPE_BOOL:
			{
				*item.m_state = getTrueFalse(value);
				item.m_override = true;
				break;
			}
			default: break;
		}
	}
	else if ( argc == 4 )
	{
		if ( item.m_type == Item::TYPE_FLOAT3 )
		{
			gfsdk_float3& dst = *item.m_float3;
			dst.x = (Float32)atof(argv[1]);
			dst.y = (Float32)atof(argv[2]);
			dst.z = (Float32)atof(argv[3]);
			item.m_override = true;
		}
	}
	else if ( argc == 5 )
	{
		if ( item.m_type == Item::TYPE_FLOAT4 )
		{
			gfsdk_float4& dst = *item.m_float4;
			dst.x = (Float32)atof(argv[1]);
			dst.y = (Float32)atof(argv[2]);
			dst.z = (Float32)atof(argv[3]);
			dst.w = (Float32)atof(argv[4]);
			item.m_override = true;
		}
	}
}

IndexT HairChannel::_indexOfItem(const char* name) const
{
	const IndexT numItems = m_items.getSize();
	for (IndexT i = 0; i < numItems; i++)
	{
		const Item& item = m_items[i];
		if (_stricmp(item.m_command, name) == 0)
		{
			return i;
		}
	}
	return -1;
}

void HairChannel::_updateDescriptor(const Item& item, InstanceDescriptor& descriptor, Bool verbose)
{
	const char* keyword = item.m_command;

	char* dst;
	if ( item.m_isDesc )
	{
		PtrDiffT offset = (char *)item.m_float - (char *)&m_override;
		dst = offset + (char *)&descriptor;
	}
	else
	{
		dst = (char*)item.m_float;
	}
	switch ( item.m_type)
	{
		case Item::TYPE_BOOL:
		{
			if (verbose)
				m_channel->send("%s=%s ", keyword, *item.m_state ? "enabled" : "disabled");
			break;
		}
		case Item::TYPE_FLOAT:
		{
			float* value = item.m_float;
			if (!item.m_override)
			{
				value = (float *)dst;
			}

			if (verbose)
				m_channel->send("%s=%0.4f is %s ", keyword, *value, item.m_override ? "enabled" : "disabled");
			break;
		}
		case Item::TYPE_INT32:
		case Item::TYPE_UINT32:
		{
			UInt32* value = item.m_uint;
			if ( !item.m_override )
			{
				value = (UInt32*)dst;
			}
			if (verbose)
				m_channel->send("%s=%d is %s ", keyword, *value, item.m_override ? "enabled" : "disabled");
			break;
		}
		case Item::TYPE_FLOAT4:
		{
			gfsdk_float4* value = item.m_float4;
			if (!item.m_override)
			{
				value = (gfsdk_float4 *)dst;
			}
			if (verbose)
				m_channel->send("%s=%0.4f,%0.4f,%0.4f,%0.4f is %s ", keyword, value->x, value->y, value->z, value->w, item.m_override ? "enabled" : "disabled");
			break;
		}
		case Item::TYPE_FLOAT3:
		{
			gfsdk_float3* value = item.m_float3;
			if ( !item.m_override )
			{
				value = (gfsdk_float3 *)dst;
			}
			if (verbose)
				m_channel->send("%s=%0.4f,%0.4f,%0.4f is %s ", keyword, value->x, value->y, value->z, item.m_override ? "enabled" : "disabled");
			break;
		}
	}
}

void HairChannel::updateDescriptor(InstanceDescriptor& descriptor, Bool verbose)
{
	if ( !m_channel ) 
	{
		return;
	}
	Int argc;
	const char*const*argv = m_channel->getInput(argc);

	if ( argv )
	{
		const char* keyword = argv[0];
		if ( strcmp(keyword,"stats") == 0 )
		{
			m_channel->send("RenderCallCount=%d RenderCount=%d", m_lastRenderCallCount, m_lastRenderCount );
		}	
		else
		{
			const IndexT itemIndex = _indexOfItem(keyword);
			if (itemIndex >= 0)
			{
				Item& item = m_items[itemIndex];
				_updateItem(argc, argv, item);
				_updateDescriptor(item, descriptor, verbose);
			}
		}
	}

	// write actual values to the descriptor
	{
		const IndexT numItems = m_items.getSize();
		for (IndexT i = 0; i < numItems; i++)
		{
			Item& item = m_items[i];
			if ( item.m_override )
			{
				if ( item.m_isDesc )
				{
					PtrDiffT offset = (char *)item.m_float - (char *)&m_override;
					char* dst = offset + (char *)&descriptor;
					switch ( item.m_type)
					{
						case Item::TYPE_BOOL:
						{
							*(bool*)dst = *item.m_state;
							break;
						}
						case Item::TYPE_FLOAT:
						{
							*(Float32*)dst = *item.m_float;
							break;
						}
						case Item::TYPE_INT32:
						{
							*(Int32*)dst = *item.m_int;
							break;
						}
						case Item::TYPE_UINT32:
						{
							*(UInt32*)dst = *item.m_uint;
							break;
						}
						case Item::TYPE_FLOAT4:
						{
							*(gfsdk_float4*)dst = *item.m_float4;
							break;
						}
						case Item::TYPE_FLOAT3:
						{
							*(gfsdk_float3*)dst = *item.m_float3;
							break;
						}
						default: break;
					}
				}
			}
		}
	}
}

void HairChannel::resetStats()
{
	m_lastRenderCount = m_renderCount;
	m_lastRenderCallCount = m_renderCallCount;
	m_renderCount = 0;
	m_renderCallCount = 0;
}

void HairChannel::logMessage(const char* fmt,...)
{
	if ( m_logFile )
	{
		char buf[512];
		buf[511] = 0;

		va_list argptr;
		va_start(argptr, fmt);
		_vsnprintf(buf, NV_COUNT_OF(buf), fmt, argptr);
		va_end(argptr);

		fprintf(m_logFile,"%s", buf);
		fflush(m_logFile);
	}
}

} // namespace HairWorks 
} // namespace nvidia 
