[

	{
		header =>
		{
			className => 'HairWorksInfo',
			implementStorage => 1,
		
			# Version history
			# 1.0 Initial Version
			classVersion => '1.0',

			hints =>
			{
			},
		},

		structs =>
		[
		],
		
		parameters =>
		[
			{
				name => 'fileVersion',
				type => 'STRING',
				hints => { shortDescription => "file format version", },
			},
			{
				name => 'toolVersion',
				type => 'STRING',
				hints => { shortDescription => "authoring tool version", },
			},
			{
				name => 'sourcePath',
				type => 'STRING',
				hints => { shortDescription => "source asset path to generate this file", },
			},
			{
				name => 'authorName',
				type => 'STRING',
				hints => { shortDescription => "author name", },
			},
			{
				name => 'lastModified',
				type => 'STRING',
				hints => { shortDescription => "last modified date time yyyy-MM-DD HH:MM:SS format. ex) 2014-02-02 14:01:02", },
			},
		]
	},

	{
		header =>
		{
			className => 'HairSceneDescriptor',
			implementStorage => 1,
		
			# Version history
			# 1.0 Initial Version
			classVersion => '1.0',

			hints =>
			{
			},
		},

		structs =>
		[
		],

		parameters =>
		[
			{
				name => 'densityTexture',
				type => 'STRING',
				hints => { shortDescription => "density texture", }
			},
			{
				name => 'rootColorTexture',
				type => 'STRING',
				hints => { shortDescription => "root color texture", }
			},
			{
				name => 'tipColorTexture',
				type => 'STRING',
				hints => { shortDescription => "tip color texture", }
			},
			{
				name => 'rootWidthTexture',
				type => 'STRING',
				hints => { shortDescription => "root width texture", }
			},
			{
				name => 'tipWidthTexture',
				type => 'STRING',
				hints => { shortDescription => "top width texture", }
			},
			{
				name => 'stiffnessTexture',
				type => 'STRING',
				hints => { shortDescription => "stiffness texture", }
			},
			{
				name => 'rootStiffnessTexture',
				type => 'STRING',
				hints => { shortDescription => "root stiffness texture", }
			},
			{
				name => 'clumpScaleTexture',
				type => 'STRING',
				hints => { shortDescription => "clump scale texture", }
			},
			{
				name => 'clumpNoiseTexture',
				type => 'STRING',
				hints => { shortDescription => "clump noise texture", }
			},
			{
				name => 'waveScaletexture',
				type => 'STRING',
				hints => { shortDescription => "wave scale texture", }
			},
			{
				name => 'waveFreqTexture',
				type => 'STRING',
				hints => { shortDescription => "wave freq texture", }
			},
			{
				name => 'lengthTexture',
				type => 'STRING',
				hints => { shortDescription => "length texture", }
			},

		]
	},
	
	{
		header =>
		{
			className => 'HairAssetDescriptor',
			implementStorage => 1,
		
			# Version history
			# 1.0 Initial Version
			classVersion => '1.0',

			hints =>
			{
			},
		},
		
		parameters =>
		[
			{
				name => 'numGuideHairs',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "number of guide hairs",
					longDescription => "number of hair guide hair curves",
				},
			},
			{
				name => 'numVertices',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "number of vertices",
					longDescription => "number of total # of cvs in guide curves",
				},
			},
			{
				name => 'vertices',
				type => 'VEC3',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "vertices",
					longDescription => "all the cv data of guide curves",
				},
			},
			{
				name => 'endIndices',
				type => 'U32',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "end indices",
					longDescription => "last vertex index of each hair, size of this array should be 'numGuideHairs'",
				},
			},
			{
				name => 'numFaces',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "number of faces",
					longDescription => "number of hair triangles, we grow hairs for each mesh triangles",
				},
			},
			{
				name => 'faceIndices',
				type => 'U32',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "face indices",
					longDescription => "triangle indices for hair faces, size must be 3 * m_nbHairFaces",
				},
			},
			{
				name => 'faceUVs',
				type => 'VEC2',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "face UVs",
					longDescription => "uv values for hair faces, size must be 3 * m_nbHairFaces",
				},
			},
			{
				name => 'numBones',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "number of bones",
					longDescription => "number of bones",
				},
			},
			{
				name => 'boneIndices',
				type => 'VEC4',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "bone indices",
					longDescription => "size should be 'numGuideHairs'. each mesh vertex (hair root) can have up to 4 bone indices.",
				},
			},
			{
				name => 'boneWeights',
				type => 'VEC4',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "bone weights",
					longDescription => "size should be 'numGuideHairs'. each mesh vertex (hair root) can have up to 4 bone weights.",
				},
			},
			{
				name => 'boneNames',
				type => 'U8',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					shortDescription => "bone names",
					longDescription => "names for each bone used to check if bone names match. buffer size should be at least BONENAME_SIZE * 'numBones'.",
				},
			},
			{
				name => 'boneNameList',
				type => 'STRING',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					shortDescription => "bone name list",
					longDescription => "names for each bone used to check if bone names match. buffer size should be at least BONENAME_SIZE * 'numBones'.",
				},
			},
			{
				name => 'bindPoses',
				type => 'MAT44',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					shortDescription => "bind pose matrices",
					longDescription => "bind pose matrices for each bone. buffer size should be at least sizeof(gfsdk_float4x4) * m_NumBones."
				},
			},
			{
				name => 'boneParents',
				type => 'I32',
				isArray => 1,
				arraySize => '-1',
				hints =>
				{
					shortDescription => "bone parents",
					longDescription => "parent index for each bone.  if this is a root bone, the index will be -1. buffer size should be at least sizoef(gfsdk_S32) * m_NumBones."
				},
			},
			{
				name => 'sceneUnit',
				type => "F32",
				defaultValue => '1.0',
				hints => { shortDescription => "scene unit scale", },
			},
			{
				name => 'upAxis',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					min => '0',
					max => '2',
					shortDescription => "up axis",
					longDescription => "0: unknown, 1: y-up, 2: z-up",
				},
			},
			{
				name => 'handedness',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					min => '0',
					max => '2',
					shortDescription => "handedness",
					longDescription => "0: unknown, 1: right handed, 2: left handed",
				},
			},
		],
	},

	{
		header =>
		{
			className => 'HairInstanceDescriptor',
			implementStorage => 1,
		
			# Version history
			# 1.0 Initial Version
			classVersion => '1.0',

			hints =>
			{
			},
		},

		structs =>
		[
		],
		
		parameters =>
		[
			{
				name => 'width',
				type => 'F32',
				defaultValue => '2.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "width",
					longDescription => "hair width in millimeters",
				},
			},
			{
				name => 'widthNoise',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "width noise",
					longDescription => "noise to hair width ",
				},
			},
			{
				name => 'widthRootScale',
				type => 'F32',
				defaultValue => '1.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "width scale root",
					longDescription => "scale factor for top side of the strand",
				},
			},
			{
				name => 'widthTipScale',
				type => 'F32',
				defaultValue => '0.100000001',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "width scale tip",
					longDescription => "scale factor for bottom side of the strand",
				},
			},

			{
				name => 'clumpNoise',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "clump noise",
					longDescription => "probability of each hair gets clumped. (0 = all hairs get clumped, 1 = clump scale is randomly distributed from 0 to 1)",
				},
			},
			{
				name => 'clumpRoundness',
				type => 'F32',
				defaultValue => '1.000000000',
				hints =>
				{
					min => '0.200000003',
					max => '2.000000000',
					shortDescription => "clump roundness",
					longDescription => "exponential factor to control roundness of clump shape. (0 = linear cone, clump scale *= power(t, roundness), where t is normalized distance from the root)",
				},
			},
			{
				name => 'clumpScale',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "clump scale",
					longDescription => "how clumped each hair face is",
				},
			},
			{
				name => 'density',
				type => 'F32',
				defaultValue => '0.500000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "hair density ratio",
					longDescription => "ratio of number of interpolated hairs compared to maximum ",
				},
			},
			{
				name => 'usePixelDensity',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "use per pixel density ",
					longDescription => "use per pixel density for density map sampling",
				},
			},
			{
				name => 'lengthNoise',
				type => 'F32',
				defaultValue => '1.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "length noise",
					longDescription => "length variation noise",
				},
			},
			{
				name => 'lengthScale',
				type => 'F32',
				defaultValue => '1.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "length scale",
					longDescription => "length control for growing hair effect",
				},
			},
			{
				name => 'waveScale',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "wave scale",
					longDescription => "size of waves for hair waviness in centimeters",
				},
			},
			{
				name => 'waveScaleNoise',
				type => 'F32',
				defaultValue => '0.500000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "wave scale noise",
					longDescription => "noise factor for the wave scale",
				},
			},
			{
				name => 'waveFreq',
				type => 'F32',
				defaultValue => '3.000000000',
				hints =>
				{
					min => '1.000000000',
					max => '10.000000000',
					shortDescription => "wave freq",
					longDescription => "wave frequency (1.0 = one sine wave along hair length)",
				},
			},
			{
				name => 'waveFreqNoise',
				type => 'F32',
				defaultValue => '0.500000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "wave freq noise",
					longDescription => "noise factor for the wave frequency",
				},
			},
			{
				name => 'waveRootStraighten',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "wave root straighten",
					longDescription => "cutoff from root to make root not move by waviness ",
				},
			},
			{
				name => 'enableLOD',
				type => 'BOOL',
				defaultValue => 'true',
				hints =>
				{
					shortDescription => "Enable LOD",
					longDescription => "whether to enable lod",
				},
			},			
			{
				name => 'enableDistanceLOD',
				type => 'BOOL',
				defaultValue => 'true',
				hints =>
				{
					shortDescription => "Enable Distance LOD",
					longDescription => "whether to enable lod for far away object (distance LOD)",
				},
			},			
			{
				name => 'distanceLODStart',
				type => 'F32',
				defaultValue => '5.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "distance LOD start",
					longDescription => "distance (in meters) to camera where fur will start fading out (by reducing density)",
				},
			},
			{
				name => 'distanceLODEnd',
				type => 'F32',
				defaultValue => '10.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "LOD fade end",
					longDescription => "distance (in meters) to camera where fur will completely disappear (and stop simulating)",
				},
			},
			{
				name => 'distanceLODDensity',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "distance LOD density",
					longDescription => "distance (in meters) to camera where fur will start fading out (by reducing density)",
				},
			},
			{
				name => 'distanceLODWidth',
				type => 'F32',
				defaultValue => '2.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "LOD fade end",
					longDescription => "target hair width when distance LOD is triggered.",
				},
			},
			{
				name => 'enableDetailLOD',
				type => 'BOOL',
				defaultValue => 'true',
				hints =>
				{
					shortDescription => "Enable Detail LOD",
					longDescription => "whether to enable lod for close object (detail LOD)",
				},
			},			
			{
				name => 'detailLODStart',
				type => 'F32',
				defaultValue => '2.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "closeup start",
					longDescription => "[UNIT DEPENDENT] distance (in scene unit) to camera where fur will start getting denser toward closeup density",
				},
			},
			{
				name => 'detailLODEnd',
				type => 'F32',
				defaultValue => '1',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "closeup end",
					longDescription => "[UNIT DEPENDENT] distance (in scene unit) to camera where fur will get full closeup density value",
				},
			},
{
				name => 'detailLODDensity',
				type => 'F32',
				defaultValue => '1.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "detail LOD density",
					longDescription => "target hair density when detail LOD is in action",
				},
			},
			{
				name => 'detailLODWidth',
				type => 'F32',
				defaultValue => '1.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.000000000',
					shortDescription => "LOD fade end",
					longDescription => "target hair width when detail up density is triggered",
				},
			},
			{
				name => 'useViewfrustrumCulling',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "use view frustum culling",
					longDescription => "when this is on, density for hairs outside view are set to 0. Use this option when fur is in a closeup.",
				},
			},
			{
				name => 'useBackfaceCulling',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "use backface culling",
					longDescription => "when this is on, density for hairs growning from backfacing faces will be set to 0.",
				},
			},
			{
				name => 'backfaceCullingThreshold',
				type => 'F32',
				defaultValue => '-0.2',
				hints =>
				{
					min => '-1.000000000',
					max => '1.000000000',
					shortDescription => "backface culling threshold",
					longDescription => "threshold to determine backface, note that this value should be slightly smaller 0 to avoid hairs at the silhouette from disappearing",
				},
			},
			{
				name => 'shadowDensityScale',
				type => 'F32',
				defaultValue => '1.0',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "shadow density scale",
					longDescription => "density scale factor to reduce hair density for shadow map rendering",
				},
			},
			{
				name => 'ambientColor',
				type => 'VEC4',
				defaultValue => 'initVec4(0.0, 0.0, 0.0, 1.0)',
				hints =>
				{
					shortDescription => "ambient color",
					longDescription => "ambient color",
				},
			},
			{
				name => 'diffuseBlend',
				type => 'F32',
				defaultValue => '1.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "diffuse blend",
					longDescription => "blend factor between tangent based hair lighting vs normal based skin lighting (0 = all tangent, 1 = all normal)",
				},
			},
			{
				name => 'specularColor',
				type => 'VEC4',
				defaultValue => 'initVec4(0.8, 0.9, 1.0, 1.0)',
				hints =>
				{
					min => "0.0",
					max => '1.0',
					shortDescription => "specular color",
					longDescription => "specular lighing color (when specular textures are not used)",
				},
			},
			{
				name => 'specularPrimary',
				type => 'F32',
				defaultValue => '0.100000001',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "specular primary",
					longDescription => "primary specular factor",
				},
			},
			{
				name => 'specularSecondary',
				type => 'F32',
				defaultValue => '0.050000001',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "specular secondary",
					longDescription => "secondary specular factor",
				},
			},
			{
				name => 'specularSecondaryOffset',
				type => 'F32',
				defaultValue => '0.100000001',
				hints =>
				{
					min => "-1.000000000",
					max => '1.000000000',
					shortDescription => "spec secondary offset",
					longDescription => "secondary highlight shift offset along tangents",
				},
			},
			{
				name => 'specularPowerPrimary',
				type => 'F32',
				defaultValue => '100.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '100.000000000',
					shortDescription => "spec primary power",
					longDescription => "primary specular power exponent",
				},
			},
			{
				name => 'specularPowerSecondary',
				type => 'F32',
				defaultValue => '20.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '100.000000000',
					shortDescription => "spec secondary power",
					longDescription => "secondary specular power exponent",
				},
			},
			{
				name => 'rootColor',
				type => 'VEC4',
				defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000,1.000000000)',
				hints =>
				{
					min => "0.0",
					max => '1.0',
					shortDescription => "root color",
					longDescription => "hair color for root (when root color textures are not used)",
				},
			},
			{
				name => 'tipColor',
				type => 'VEC4',
				defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000,1.000000000)',
				hints =>
				{
					min => "0.0",
					max => '1.0',
					shortDescription => "tip color",
					longDescription => "hair color for tip (when tip color textures are not used)",
				},
			},
			{
				name => 'rootTipColorWeight',
				type => 'F32',
				defaultValue => '0.500000000',
				hints =>
				{
					shortDescription => "root tip color weight",
					longDescription => "bias factor for root/tip color blending ",
				},
			},
			{
				name => 'shadowSigma',
				type => 'F32',
				defaultValue => '1.00000000',
				hints =>
				{
					min => '0.000000000',
					max => '100.000000000',
					shortDescription => "shadow attenuation",
					longDescription => "distance through hair volume beyond which hairs get completely shadowed.",
				},
			},
			{
				name => 'useTextures',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "use textures",
					longDescription => "use textures ",
				},
			},
			{
				name => 'castShadows',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "cast shadows",
					longDescription => "this hair casts shadows on the scene",
				},
			},
			{
				name => 'receiveShadows',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "receive shadows",
					longDescription => "this hair receives shadows from the scene",
				},
			},
			{
				name => 'backStopRadius',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "backstop radius",
					longDescription => "[UNIT DEPENDENT] radius of backstop collision",
				},
			},
			{
				name => 'damping',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '0.100000001',
					shortDescription => "damping",
					longDescription => "damping to slow down hair motion",
				},
			},
			{
				name => 'massScale',
				type => 'F32',
				defaultValue => '10.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '1000.0000000',
					shortDescription => "massScale",
					longDescription => "mass to scale gravity strength",
				},
			},
			{
				name => 'gravityDir',
				type => 'VEC3',
				defaultValue => 'init(0.000000000,0.000000000,-1.000000000)',
				hints =>
				{
					min => "-1000.0",
					max => '1000.0',
					shortDescription => "gravityDir",
					longDescription => "gravity force vector direction",
				},
			},
			{
				name => 'rootStiffness',
				type => 'F32',
				defaultValue => '0.500000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "root stiffness",
					longDescription => "attenuation of stiffness away from the root (stiffer at root, weaker toward tip)",
				},
			},
			{
				name => 'simulate',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "simulate",
					longDescription => "whether to turn on/off simulation",
				},
			},
			{
				name => 'stiffness',
				type => 'F32',
				defaultValue => '0.500000000',
				hints =>
				{
					min => '0.000000000',
					max => '1.000000000',
					shortDescription => "stiffness",
					longDescription => "stiffness to restore to skinned rest shape for hairs",
				},
			},
			{
				name => 'wind',
				type => 'VEC3',
				defaultValue => 'init(0.000000000,0.000000000,0.000000000)',
				hints =>
				{
					min => "-1000.0",
					max => '1000.0',
					shortDescription => "wind",
					longDescription => "vector force for main wind direction",
				},
			},
			{
				name => 'windNoise',
				type => 'F32',
				defaultValue => '0.000000000',
				hints =>
				{
					min => '0.000000000',
					max => '10.000000000',
					shortDescription => "wind noise",
					longDescription => "strength of wind noise",
				},
			},
			{
				name => 'visualizeBones',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "visualize bones",
					longDescription => "draw bones ",
				},
			},
			{
				name => 'visualizeGrowthMesh',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "visualize growth mesh",
					longDescription => "draw growth mesh",
				},
			},
			{
				name => 'visualizeGuideHairs',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "visualize guide hairs",
					longDescription => "draw guide hairs",
				},
			},
			{
				name => 'visualizeControlVertices',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "visualize control points",
					longDescription => "visualize control points",
				},
			},
			{
				name => 'drawRenderHairs',
				type => 'BOOL',
				hints =>
				{
					shortDescription => "draw render hairs",
					longDescription => "draw render hair",
				},
			},
			{
				name => 'colorizeMode',
				type => 'U32',
				hints =>
				{
					shortDescription => "colirize LOD",
					longDescription => "show LOD factor in color (works only when non-custom pixel shader is used)",
				},
			},

		],
	},
]
