/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvHairInternal.h"

#include "NvHairShaderCache.h"

#include "NvHairInstance.h"

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/NvCoMemory.h>

#include "NvHairInternal.h"

namespace nvidia {
namespace HairWorks {

/// Variable defines how cache settings are used with shader code. 
static const ShaderCache::Variable s_variables[] = 
{
	{ "_USE_MULTI_MATERIAL_",			ShaderCache::Variable::TYPE_SET_INDEX, 0},
	{ "_USE_PIXEL_DENSITY_",			ShaderCache::Variable::TYPE_FLAG, ShaderCacheSettings::FLAG_PIXEL_DENSITY },
	{ "_USE_CULL_SPHERE_",				ShaderCache::Variable::TYPE_FLAG, ShaderCacheSettings::FLAG_CULL_SPHERE },
	{ "_USE_CLUMPING_",					ShaderCache::Variable::TYPE_FLAG, ShaderCacheSettings::FLAG_CLUMPING },
	{ "_USE_WAVINESS_",					ShaderCache::Variable::TYPE_FLAG, ShaderCacheSettings::FLAG_WAVE_STRAND},
	{ "_USE_WAVE_CLUMP_",				ShaderCache::Variable::TYPE_FLAG, ShaderCacheSettings::FLAG_WAVE_CLUMP },

	{ "_USE_DENSITY_TEXTURE_",			ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::DENSITY)},
	{ "_SAMPLE_DENSITY_",				ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::DENSITY)},
	{ "_USE_WAVE_SCALE_TEXTURE_",		ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::WAVE_SCALE)},
	{ "_SAMPLE_WAVE_SCALE_",			ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::WAVE_SCALE)},
	{ "_USE_WAVE_FREQ_TEXTURE_",		ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::WAVE_FREQ)},
	{ "_SAMPLE_WAVE_FREQ_",				ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::WAVE_FREQ)},

	{ "_USE_CLUMP_SCALE_TEXTURE_",		ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::CLUMP_SCALE) },
	{ "_SAMPLE_CLUMP_SCALE_",			ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::CLUMP_SCALE)},

	{ "_USE_CLUMP_ROUNDNESS_TEXTURE_",	ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::CLUMP_ROUNDNESS)},
	{ "_SAMPLE_CLUMP_ROUNDNESS_",		ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::CLUMP_ROUNDNESS)},

	{ "_USE_WIDTH_TEXTURE_",			ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::WIDTH)},
	{ "_SAMPLE_WIDTH_",					ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::WIDTH)},

	{ "_USE_LENGTH_TEXTURE_",			ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::LENGTH)},
	{ "_SAMPLE_LENGTH_",				ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::LENGTH)},

	{ "_USE_WEIGHT_TEXTURE_",			ShaderCache::Variable::TYPE_TEXTURE_USED,	Int(TextureType::WEIGHTS)},
	{ "_SAMPLE_WEIGHT_",				ShaderCache::Variable::TYPE_SAMPLER,		Int(TextureType::WEIGHTS)},
};

static ShaderCacheSettings _calcSettingsMask()
{
	typedef ShaderCache::Variable Variable;
	typedef ShaderCacheSettings Settings;

	ShaderCacheSettings settings;
	settings.zero();

	// This is settable by default
	settings.m_useFlags |= Settings::FLAG_SINGLE_TARGET;

	for (IndexT i = 0; i < NV_COUNT_OF(s_variables); i++)
	{
		const Variable& var = s_variables[i];
		switch (var.m_type)
		{
			case Variable::TYPE_FLAG:			settings.setUsed(ShaderCacheSettings::Flag(var.m_index), true); break;
			case Variable::TYPE_SAMPLER:		settings.setChannel(var.m_index, ETextureChannel(3)); break;
			case Variable::TYPE_TEXTURE_USED:	settings.setTextureUsed(var.m_index, true); break;
			default: break;
		}
	}
	return settings;
}

/* static */const ShaderCacheSettings s_mask = _calcSettingsMask();

/* static */const char* ShaderCache::getChannelSampler(ETextureChannel channel)
{
	switch (channel)
	{
		default:
		case TextureChannel::RED:		return "SAMPLE_RED"; 
		case TextureChannel::GREEN:		return "SAMPLE_GREEN"; 
		case TextureChannel::BLUE:		return "SAMPLE_BLUE"; 
		case TextureChannel::ALPHA:		return "SAMPLE_ALPHA"; 
	}
}

void ShaderCache::calcSettings(Instance* inst, ShaderCacheSettings& settingsOut)
{
	ApiInstance* apiInst = inst->m_apiInstance;

	const InstanceDescriptor& defaultMaterial = inst->getDefaultMaterial();
	typedef ShaderCacheSettings Settings;

	settingsOut.setUsed(Settings::FLAG_PIXEL_DENSITY, defaultMaterial.m_usePixelDensity);
	settingsOut.setUsed(Settings::FLAG_CULL_SPHERE, defaultMaterial.m_useCullSphere);

	settingsOut.setUsed(Settings::FLAG_CLUMPING, defaultMaterial.m_clumpScale > 0.0f);
	settingsOut.setUsed(Settings::FLAG_WAVE_STRAND, (defaultMaterial.m_waveScale > 0.0f) && (defaultMaterial.m_waveScaleStrand > 0.0f));
	settingsOut.setUsed(Settings::FLAG_WAVE_CLUMP,  (defaultMaterial.m_waveScale > 0.0f) && (defaultMaterial.m_waveScaleClump > 0.0f));

	for (Int i = 0; i < Int(TextureType::COUNT_OF); i++)
	{
		settingsOut.setTextureUsed(i, apiInst->isApiTextureUsed((ETextureType)i));
		settingsOut.setChannel(i, (ETextureChannel)inst->getDefaultTextureChannel((ETextureType)i));
	}
}

/* static */const ShaderCache::Variable* ShaderCache::findVariable(const Char* text, Int& lenOut)
{
	for (Int i = 0; i < NV_COUNT_OF(s_variables); i++)
	{
		const Variable& var = s_variables[i];
		const Char* name = var.m_name;
		Int pos;
		for (pos = 0; name[pos] == text[pos] && name[pos]; pos++);
		// If made to the end it is a match
		if (name[pos] == 0)
		{
			lenOut = pos;
			return &var;
		}
	}
	return NV_NULL;
}

/* static */void ShaderCache::createShaderSource(const Settings& settings, const Char* sourceCode, NvCo::Array<Char>& targetCodeOut)
{
	targetCodeOut.clear();

	const Char* start = sourceCode;
	const Char* cur = start;
	while (*start)
	{
		// Find either the end or first _
		while (*cur && *cur != '_') cur++;
		// If we've hit the end then we are done
		if (*cur == 0)
		{
			break;
		}
		Int varLen;
		// See if we can match one of the variables
		const Variable* match = findVariable(cur, varLen);
		if (match)
		{
			// Flush anything so far
			targetCodeOut.pushBack(start, IndexT(PtrDiffT(cur - start)));

			// Apply the replacement based on type
			switch (match->m_type)
			{
				case Variable::TYPE_FLAG:
				{
					const Char c = settings.isUsed(Settings::Flag(match->m_index)) ? '1' : '0';
					targetCodeOut.pushBack(c);
					break;
				}
				case Variable::TYPE_SAMPLER:
				{
					ETextureChannel texChan = settings.getChannel(match->m_index);
					const Char* chanText = getChannelSampler(texChan);
					targetCodeOut.pushBack(chanText, strlen(chanText));
					break;
				}
				case Variable::TYPE_TEXTURE_USED:
				{
					const Char c = settings.isTextureUsed(match->m_index) ? '1' : '0';
					targetCodeOut.pushBack(c);
					break;
				}
				case Variable::TYPE_SET_INDEX:
				{
					char buf[16];
					sprintf(buf, "%d", match->m_index);
					targetCodeOut.pushBack(buf, strlen(buf));
					break;
				}
			}
			// Skip the whole symbol
			start = cur + varLen;
			// Cur is at start again (as we flushed contents)
			cur = start;
			continue;
		}
		// Skip the _ then
		cur++;
	}

	// Flush any remaining. Add 1 so we write terminating 0
	targetCodeOut.pushBack(start, IndexT(PtrDiffT(cur + 1 - start)));
}

void ShaderCache::clear()
{
	for (IndexT i = 0; i < m_entries.getSize(); ++i)
	{
		Entry* entry = m_entries[i];
		if (entry)
		{
			delete entry;
		}
	}
	m_entries.clear();
}

ShaderCacheEntry* ShaderCache::findOrCreate(const Settings& settings)
{
	// Make the settings unique
	Entry* entry  = find(settings);
	if (entry)
		return entry;
	entry = m_factory->createEntry();
	if (!entry)
	{
		return false;
	}
	if (NV_FAILED(entry->compile(settings)))
	{
		delete entry;
		return NV_NULL;
	}
	const IndexT slot = m_entries.indexOf(NV_NULL);
	if (slot < 0)
	{
		m_entries.pushBack(entry);
	}
	else
	{
		m_entries[slot] = entry;
	}
	return entry;
}

ShaderCache::Entry* ShaderCache::find(const Settings& settings)
{
	for (Int i = 0; i < m_entries.getSize(); i++)
	{
		Entry* entry = m_entries[i];
		if (entry->getSettings() == settings)
		{
			return entry;
		}
	}
	return NV_NULL;
}

Result ShaderCache::write(NvCo::WriteStream* stream)
{
	// Header
	SizeT totalSize = sizeof(Header);
	// Followed by entry sizes
	totalSize += sizeof(UInt32) * m_entries.getSize();

	NvCo::Array<UInt32> sizes;
	sizes.setSize(m_entries.getSize());
	for (IndexT i = 0; i < m_entries.getSize(); i++)
	{
		const Entry* entry = m_entries[i];
		UInt32 entrySize = entry ? UInt32(entry->calcSize()) : 0;
		sizes[i] =  entrySize;
		totalSize += entrySize;
	}

	{
		Header header;
		// Set the header
		header.m_totalSize = UInt32(totalSize);
		header.m_numEntries = UInt32(m_entries.getSize()); // actual caches
		stream->write(&header, sizeof(header));
	}

	// Save the sizes
	stream->write(sizes.begin(), sizes.getSize() * sizeof(UInt32));

	// Write out each entry
	for (Int i = 0; i < sizes.getSize(); i++)
	{
		UInt32 size = sizes[i];
		const Entry* entry = m_entries[i];
		if (size)
		{
			NV_RETURN_ON_FAIL(entry->write(stream));
		}
	}
	return NV_OK;
}

Result ShaderCache::read(NvCo::ReadStream* stream)
{
	// Have to clear out whats there already
	clear();

	// Read the header
	Header header;
	if (stream->read(&header, sizeof(header)) != sizeof(Header))
	{
		NV_CO_LOG_ERROR("Unable to read header");
		return NV_FAIL;
	}

	// Set up the entries
	m_entries.setSizeWithDefault(header.m_numEntries, NV_NULL);
	
	// Read the sizes
	NvCo::Array<UInt32> sizes;
	{
		sizes.setSize(header.m_numEntries);
		const SizeT readSize = sizeof(UInt32) * header.m_numEntries;
		if (SizeT(stream->read(sizes.begin(), readSize)) != readSize)
		{
			NV_CO_LOG_ERROR("Unable shader cache sizes");
			return NV_FAIL;
		}
	}

	for (Int i = 0; i < Int(header.m_numEntries); i++)
	{
		const UInt32 size = sizes[i];
		if (size)
		{
			Entry* entry = m_factory->createEntry();
			m_entries[i] = entry;
			NV_RETURN_ON_FAIL(entry->read(stream));
		}
	}
	return NV_OK;
}

Result ShaderCache::createShaders(const NvCo::ApiDevice& device)
{
	for (IndexT i = 0; i < m_entries.getSize(); i++)
	{
		Entry* entry = m_entries[i];
		if (entry)
		{
			NV_RETURN_ON_FAIL(entry->createShaders(device));
		}
	}
	return NV_OK;
}

/* static */const ShaderCacheSettings& ShaderCache::getDefaultSettingsMask()
{
	return s_mask;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  ShaderCacheFactory !!!!!!!!!!!!!!!!!!!!!!!!

ShaderCacheSettings ShaderCacheFactory::calcUniqueSettings(const ShaderCacheSettings& settingsIn)
{
	ShaderCacheSettings settings(settingsIn);
	settings.applyMask(s_mask);
	return settings;
}

} // namespace HairWorks
} // namespace nvidia