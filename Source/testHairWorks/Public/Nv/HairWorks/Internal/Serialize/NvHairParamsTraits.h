/* Copyright (c) 2016, NVIDIA CORPORATION.  All rights reserved.
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited. */

#include "NvParameterized.h"
#include "XmlSerializer.h"
#include "BinSerializer.h"
#include "NvFileBuffer.h"
#include "NvTraits.h"
#include "NvMemoryBuffer.h"
#include "NsAtomic.h"

#include <string>
#include <map>

// Copied from NvTraits.cpp. For now, NvTraits class is identical to the one in NvTraits.cpp

#define ALIGNED_ALLOC(n, align) nvidia::shdfnd::AlignedAllocator<align>().allocate(n, __FILE__, __LINE__)

// Does not depend on alignment in AlignedAllocator
#define ALIGNED_FREE(p) nvidia::shdfnd::AlignedAllocator<16>().deallocate(p)

class NvTraits : public NvParameterized::Traits, public nvidia::shdfnd::UserAllocated
{
public:
	NvTraits(void)
	{

	}

	virtual ~NvTraits(void)
	{
	}

	virtual void release(void)
	{
		delete this;
	}

	/**
	\brief Register NvParameterized class factory
	*/
	virtual void registerFactory( ::NvParameterized::Factory & factory ) 
	{
		bool ok = true;

		for (nvidia::NvU32 i=0; i<mFactories.size(); ++i)
		{
			NvParameterized::Factory *f = mFactories[i];
			if ( f == &factory )
			{
				ok = false;
				traitsWarn("Factory already registered.");
				break;
			}
			if ( strcmp(f->getClassName(),factory.getClassName()) == 0 && f->getVersion() == factory.getVersion() )
			{
				ok = false;
				traitsWarn("factory with this name and version already registered.");
				break;
			}
		}
		if ( ok )
		{
			mFactories.pushBack(&factory);
		}
	}

	/**
	\brief Remove NvParameterized class factory for current version of class
	\return Removed factory or NULL if it is not found
	*/
	virtual ::NvParameterized::Factory *removeFactory( const char * className ) 
	{
		NvParameterized::Factory *f = NULL;
		nvidia::NvU32 index=0;
		nvidia::NvU32 maxVersion = 0;
		for (nvidia::NvU32 i=0; i<mFactories.size(); i++)
		{
			if ( strcmp(mFactories[i]->getClassName(),className) == 0 )
			{
				if ( mFactories[i]->getVersion() >= maxVersion )
				{
					f = mFactories[i];
					maxVersion = f->getVersion();
					index = i;
				}
			}
		}
		if ( f )
		{
			mFactories.remove(index);
		}
		else
		{
			traitsWarn("Unable to remove factory.");
		}
		return f;
	}

	/**
	\brief Remove NvParameterized class factory for given version of class
	\return Removed factory or NULL if it is not found
	*/
	virtual ::NvParameterized::Factory *removeFactory( const char * className, nvidia::NvU32 version ) 
	{
		NvParameterized::Factory *f = NULL;
		for (nvidia::NvU32 i=0; i<mFactories.size(); ++i)
		{
			if ( strcmp(mFactories[i]->getClassName(),className) == 0 && mFactories[i]->getVersion() == version )
			{
				f = mFactories[i];
				mFactories.remove(i);
				break;
			}
		}
		if ( !f )
		{
			traitsWarn("Unable to remove factory, not found");
		}
		return f;
	}

	/**
	\brief Checks whether any class factory is registered
	*/
	virtual bool doesFactoryExist(const char* className) 
	{
		bool ret = false;
		for (nvidia::NvU32 i=0; i<mFactories.size(); ++i)
		{
			if ( strcmp(mFactories[i]->getClassName(),className) == 0 )
			{
				ret = true;
				break;
			}
		}
		return ret;
	}

	/**
	\brief Checks whether class factory for given version is registered
	*/
	virtual bool doesFactoryExist(const char* className, nvidia::NvU32 version) 
	{
		bool ret = false;
		for (nvidia::NvU32 i=0; i<mFactories.size(); ++i)
		{
			if ( strcmp(mFactories[i]->getClassName(),className) == 0 && mFactories[i]->getVersion() == version )
			{
				ret = true;
				break;
			}
		}
		return ret;
	}

	// Helper method, locate a factory of this name and exact version, of it specific version not being searched for, return the highest registered version number.
	NvParameterized::Factory * locateFactory(const char *className,nvidia::NvU32 version,bool useVersion) const
	{
		NvParameterized::Factory *f = NULL;
		nvidia::NvU32 maxVersion = 0;
		for (nvidia::NvU32 i=0; i<mFactories.size(); i++)
		{
			if ( strcmp(mFactories[i]->getClassName(),className) == 0 )
			{
				if ( useVersion )
				{
					if ( mFactories[i]->getVersion() == version )
					{
						f = mFactories[i];
					}
				}
				else if ( mFactories[i]->getVersion() >= maxVersion )
				{
					f = mFactories[i];
					maxVersion = f->getVersion();
				}
			}
		}
		return f;
	}

	/**
	\brief Create object of NvParameterized class
	
	Most probably this just calls Factory::create on appropriate factory.
	*/
	virtual ::NvParameterized::Interface * createNvParameterized( const char * name ) 
	{
		NvParameterized::Interface *ret = NULL;
		NvParameterized::Factory *f = locateFactory(name,0,false);
		if ( f )
		{
			ret = f->create(this);
		}
		return ret;
	}

	/**
	\brief Create object of NvParameterized class
	
	Most probably this just calls Factory::create on appropriate factory.
	*/
	virtual ::NvParameterized::Interface * createNvParameterized( const char * name, nvidia::NvU32 ver )
	{
		NvParameterized::Interface *ret = NULL;
		NvParameterized::Factory *f = locateFactory(name,ver,true);
		if ( f )
		{
			ret = f->create(this);
		}
		return ret;
	}

	/**
	\brief Finish construction of inplace object of NvParameterized class
	
	Most probably this just calls Factory::finish using appropriate factory.
	*/
	virtual ::NvParameterized::Interface * finishNvParameterized( const char * name, void *obj, void *buf, nvidia::NvI32 *refCount ) 
	{
		NvParameterized::Factory *f = locateFactory(name,0,false);
		return f ? f->finish(this,obj,buf,refCount) : NULL;
	}

	/**
	\brief Finish construction of inplace object of NvParameterized class
	
	Most probably this just calls Factory::finish using appropriate factory.
	*/
	virtual ::NvParameterized::Interface * finishNvParameterized( const char * name, nvidia::NvU32 ver, void *obj, void *buf, nvidia::NvI32 *refCount ) 
	{
		NvParameterized::Factory *f = locateFactory(name,ver,true);
		return f ? f->finish(this,obj,buf,refCount) : NULL;
	}

	/**
	\brief Get version of class which is currently used
	*/
	virtual nvidia::NvU32 getCurrentVersion(const char *className) const 
	{
		NvParameterized::Factory *f = locateFactory(className,0,false);
		return f ? f->getVersion() : 0;
	}

	/**
	\brief Get memory alignment required for objects of class
	*/
	virtual nvidia::NvU32 getAlignment(const char *className, nvidia::NvU32 classVersion) const 
	{
		NvParameterized::Factory *f = locateFactory(className,classVersion,true);
		return f ? f->getAlignment() : 16;
	}

	/**
	\brief Register converter for legacy version of class
	*/
	virtual void registerConversion(const char * /*className*/, nvidia::NvU32 /*from*/, nvidia::NvU32 /*to*/, NvParameterized::Conversion & /*conv*/) 
	{
		NV_ALWAYS_ASSERT(); // TODO : Not yet implemented
	}

	/**
	\brief Remove converter for legacy version of class
	*/
	virtual ::NvParameterized::Conversion *removeConversion(const char * /*className*/, nvidia::NvU32 /*from*/, nvidia::NvU32 /*to*/) 
	{ 
		NV_ALWAYS_ASSERT(); // TODO : Not yet implemented
		return 0; 
	}

	/**
	\brief Update legacy object (most probably using appropriate registered converter)
	\param [in] legacyObj legacy object to be converted
	\param [in] obj destination object
	\return True if conversion was successful, false otherwise
	\warning Note that update is intrusive - legacyObj may be modified as a result of update
	*/
	virtual bool updateLegacyNvParameterized(::NvParameterized::Interface &legacyObj, ::NvParameterized::Interface &obj)
	{
		NV_ALWAYS_ASSERT(); // TODO : Not yet implemented
		NV_UNUSED(&legacyObj);
		NV_UNUSED(&obj);

		return false;
	}

	/**
	\brief Get a list of the NvParameterized class type names

	\param [in] names buffer for names
	\param [out] outCount minimal required length of buffer
	\param [in] inCount length of buffer
	\return False if 'inCount' is not large enough to contain all of the names, true otherwise
	
	\warning The memory for the strings returned is owned by the traits class
	         and should only be read, not written or freed.
	*/
	virtual bool getNvParameterizedNames( const char ** names, nvidia::NvU32 &outCount, nvidia::NvU32 inCount) const 
	{
		bool ret = true;

		outCount = 0;
		for (nvidia::NvU32 i=0; i<mFactories.size(); i++)
		{
			NvParameterized::Factory *f = mFactories[i];
			const char *name = f->getClassName();
			for (nvidia::NvU32 j=0; j<outCount; j++)
			{
				if ( strcmp(name,names[j]) == 0 )
				{
					name = NULL;
					break;
				}
			}
			if ( name )
			{
				if ( outCount == inCount )
				{
					ret = false;
					break;
				}
				else
				{
					names[outCount] = name;
					outCount++;
				}
			}
		}
		return ret;
	}

	/**
	\brief Get a list of versions of particular NvParameterized class

	\param [in] className Name of the class
	\param [in] versions buffer for versions
	\param [out] outCount minimal required length of buffer
	\param [in] inCount length of buffer
	\return False if 'inCount' is not large enough to contain all of version names, true otherwise
	
	\warning The memory for the strings returned is owned by the traits class
	         and should only be read, not written or freed.
	*/
	virtual bool getNvParameterizedVersions(const char* className, nvidia::NvU32* versions, nvidia::NvU32 &outCount, nvidia::NvU32 inCount) const 
	{
		bool ret = true;

		outCount = 0;
		for (nvidia::NvU32 i=0; i<mFactories.size(); i++)
		{
			NvParameterized::Factory *f = mFactories[i];
			const char *name = f->getClassName();
			if ( strcmp(name,className) == 0 )
			{
				if ( outCount == inCount )
				{
					ret = false;
					break;
				}
				else
				{
					versions[outCount] = f->getVersion();
					outCount++;
				}
			}
		}
		return ret;
	}

	/**
	\brief Called when inplace object is destroyed
	*/
	virtual void onInplaceObjectDestroyed(void * /*buf*/, ::NvParameterized::Interface * /*obj*/) 
	{

	}

	/**
	\brief Called when all inplace objects are destroyed
	*/
	virtual void onAllInplaceObjectsDestroyed(void *buf) 
	{ 
		free(buf); 
	}

	void* alloc(nvidia::NvU32 nbytes)
	{
		return alloc(nbytes, 16);
	}

	void* alloc(nvidia::NvU32 nbytes, nvidia::NvU32 align)
	{
		if (align <= 16)
		{
			return ALIGNED_ALLOC(nbytes, 16);
		}
		else switch (align)
		{
		case 32:
			return ALIGNED_ALLOC(nbytes, 32);
		case 64:
			return ALIGNED_ALLOC(nbytes, 64);
		case 128:
			return ALIGNED_ALLOC(nbytes, 128);
		}

		// Do not support larger alignments

		return 0;
	}

	void free(void* buf)
	{
		ALIGNED_FREE(buf);
	}

	nvidia::NvI32 incRefCount(nvidia::NvI32* refCount)
	{
		return nvidia::shdfnd::atomicIncrement(refCount);
	}

	virtual nvidia::NvI32 decRefCount(nvidia::NvI32* refCount)
	{
		return nvidia::shdfnd::atomicDecrement(refCount);
	}


 	/**
	\brief Warns user
	*/
	virtual void traitsWarn(const char * msg) const 
	{
		nvidia::shdfnd::getFoundation().error(NV_WARN, "NvParameterized::Traits::Warning(%s)", msg );
	}

	nvidia::shdfnd::Array< NvParameterized::Factory * > mFactories;
};

class GFSDK_HairParamsTraits : public NvTraits
{
	typedef std::pair<nvidia::NvU32, nvidia::NvU32> VersionPair;
	typedef std::map<VersionPair, NvParameterized::Conversion*> VersionMap;
	typedef std::map<std::string, VersionMap> MapType;
	MapType m_map;

public:
	GFSDK_HairParamsTraits()
	{
	}

	/**
	\brief Register converter for legacy version of class
	*/
	virtual void registerConversion(const char *className, nvidia::NvU32 from, nvidia::NvU32 to, NvParameterized::Conversion &conv) 
	{
		m_map[ std::string(className) ][ VersionPair(from, to) ] = &conv;
	}

	/**
	\brief Remove converter for legacy version of class
	*/
	virtual ::NvParameterized::Conversion *removeConversion(const char * /*className*/, nvidia::NvU32 /*from*/, nvidia::NvU32 /*to*/) 
	{ 
		
		return 0; 
	}

	/**
	\brief Update legacy object (most probably using appropriate registered converter)
	\param [in] legacyObj legacy object to be converted
	\param [in] obj destination object
	\return True if conversion was successful, false otherwise
	\warning Note that update is intrusive - legacyObj may be modified as a result of update
	*/
	virtual bool updateLegacyNvParameterized(::NvParameterized::Interface &legacyObj, ::NvParameterized::Interface &obj)
	{
		MapType::iterator found = m_map.find(std::string(legacyObj.className()));
		if (found == m_map.end()) return false;
		VersionMap::iterator foundConversion = found->second.find(VersionPair(legacyObj.version(), obj.version()));
		if (foundConversion == found->second.end()) return false;
		NvParameterized::Conversion* conversion = foundConversion->second;
		if (conversion)
		{
			return (*conversion)(legacyObj, obj);
		}
		return false;
	}
};
