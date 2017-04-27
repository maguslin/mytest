/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#ifndef NV_HAIR_SHADER_CACHE_H
#define NV_HAIR_SHADER_CACHE_H

//#include <dxgi.h>
//#include <d3d11.h>

#include <Nv/Common/NvCoCommon.h>
#include <Nv/Common/NvCoTypeMacros.h>

#include <Nv/Common/NvCoComPtr.h>
#include <Nv/Common/Container/NvCoArray.h>

#include <Nv/Common/NvCoStream.h>

#include <Nv/Common/NvCoApiHandle.h>

#include "NvHairInternal.h"

namespace nvidia {
namespace HairWorks {

class Instance;

/*! Cache entry. Really should be a child class of ShaderCache, but includes
means it can't be. Can be accessed as Shader::Cache once the header is included */
class ShaderCacheEntry
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(ShaderCacheEntry);

	typedef ShaderCacheSettings Settings;

	struct Header
	{
		UInt32 m_totalLen;
		UInt32 m_hullShaderLen;
		UInt32 m_domainShaderLen;
		UInt32 m_geometryShaderLen;
	};

		/// Compile using the specified settings 
	virtual Result compile(const Settings& settings) = 0;

		/// True if has blobs and valueString set
	virtual Bool isValid() const = 0;
		/// Total size of entry
	virtual SizeT calcSize() const = 0;
		/// True if has all shaders
	virtual Bool hasShaders() const = 0;

		/// Write out 
	virtual Result write(NvCo::WriteStream* stream) const = 0;
		/// Read from previous written data 
	virtual Result read(NvCo::ReadStream* stream) = 0;

		/// Create the shaders on the device
	virtual Result createShaders(const NvCo::ApiDevice& device) = 0;
		/// Apply the shaders to the context
	virtual Bool applyShaders(const NvCo::ApiDevice& device, const NvCo::ApiContext& context) = 0;

	/// Get the settings
	NV_FORCE_INLINE const Settings& getSettings() const { return m_settings; }

		// Virtual dtor
	virtual ~ShaderCacheEntry() {}

protected:
	Settings m_settings;
};

class ShaderCacheFactory
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(ShaderCacheFactory);

		/// Modify settings taking into account what are unique settings
	virtual ShaderCacheSettings calcUniqueSettings(const ShaderCacheSettings& settings);
		/// Create an empty entry that can be read into, or compiled into 
	virtual ShaderCacheEntry* createEntry() = 0;
};

class ShaderCache
{
	NV_CO_DECLARE_POLYMORPHIC_CLASS_BASE(ShaderCache);

	typedef ShaderCacheSettings Settings;
	typedef ShaderCacheEntry Entry;

	struct Header
	{
		UInt32 m_totalSize;
		UInt32 m_numEntries;
	};

		/*!  Variable identifies something in the shader source text that will be changed */
	struct Variable
	{
		enum Type
		{
			TYPE_SET_INDEX,		///< Use the index value
			TYPE_SAMPLER,		///< Channel (Text for texture channel used)
			TYPE_TEXTURE_USED,	///< 1 or 0 based on if texture is used
			TYPE_FLAG,			///< 1 or 0 based on flag setting
		};
		const Char* m_name;		///< The name
		Type m_type;
		Int m_index;			///< Identifies the index associated with the value. 	
	};

		/// Clear the cache
	void clear();
	
		/// Create the shaders on the device
	Result createShaders(const NvCo::ApiDevice& device);

		/// Write the cache
	Result write(NvCo::WriteStream* stream);
		/// Load the cache from the buffer
	Result read(NvCo::ReadStream* stream);

		/// Find an entry based on settings
	Entry* find(const Settings& settings);
		/// Find or create based on settings. If not possible to create will return NV_NULL.
	Entry* findOrCreate(const Settings& settings);

		/// Get the list of active entries
	NV_FORCE_INLINE const NvCo::Array<Entry*>& getEntries() const { return m_entries; }

		/// Calculate unique settings
	NV_FORCE_INLINE Settings calcUniqueSettings(const Settings& in) { return m_factory->calcUniqueSettings(in); }

		/// Create the shader source
	static void createShaderSource(const Settings& settings, const Char* sourceCode, NvCo::Array<Char>& targetCodeOut);
		/// Returns the variable that matched the text, or NV_NULL if not found
	static const Variable* findVariable(const Char* text, Int& lenOut);

		/// Convert the resources definition into settings
	static void calcSettings(Instance* resources, Settings& cacheSettings);
		/// Get a channel number as text
	static const char* getChannelSampler(ETextureChannel channel);
	
		/// Calculate the mask based on variables used in shader generation 
		/// The settings mask is such that the logical AND of it with Settings will remove 
		/// any features not used in the shader. If the features aren't used, they don't need to 
		/// produce different shader compilations.
	static const Settings& getDefaultSettingsMask();

		/// Ctor
	ShaderCache(ShaderCacheFactory* factory):
		m_factory(factory)
	{}

		/// Dtor
	~ShaderCache() { clear(); }

protected:
	ShaderCacheFactory* m_factory;
	NvCo::Array<Entry*> m_entries;
};

} // namespace HairWorks
} // namespace nvidia


#endif // NV_HAIR_SHADER_CACHE_H
