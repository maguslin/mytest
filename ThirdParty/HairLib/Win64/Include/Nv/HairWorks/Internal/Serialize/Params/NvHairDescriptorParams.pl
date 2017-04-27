[

	{
		header =>
		{
			className => 'HairWorksInfo',
			implementStorage => 1,
		
			# Version history
			# 1.0 Initial Version
			# 1.1 HairWorks 1.1 release
			classVersion => '1.1',

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
			# 1.1 HairWorks 1.1 release
			classVersion => '1.1',

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
				name => 'widthTexture',
				type => 'STRING',
				hints => { shortDescription => "root width texture", }
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
				name => 'clumpRoundnessTexture',
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
				name => 'strandTexture',
				type => 'STRING',
				hints => { shortDescription => "strand texture", }
			},
			{
				name => 'lengthTexture',
				type => 'STRING',
				hints => { shortDescription => "length texture", }
			},
			{
				name => 'specularTexture',
				type => 'STRING',
				hints => { shortDescription => "specular texture", }
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
			# 1.1 Added BoneSphere structure and new parameters: numBoneSpheres, boneSpheres, numBoneCapsules, and boneCapsuleIndices
			classVersion => '1.1',

			hints =>
			{
			},
		},
		
		structs =>
		[
			{
				name => 'BoneSphere',
				parameters =>
				[
					{ name => 'boneSphereIndex', type => 'I32',
					  hints => { shortDescription => "index for the bone where the collision sphere is attached to", },
					},
					{ name => 'boneSphereRadius', type => 'F32',
					  hints => { shortDescription => "radius for the collision sphere", },
					},
					{ name => 'boneSphereLocalPos', type => 'VEC3',
					  hints => { shortDescription => "offset value with regard to bind position of the bone", },
					},
				],
			},
			{
				name => 'Pin',
				parameters =>
				[
					{
						name => 'boneSphereIndex', type => 'I32',
						hints => { shortDescription => "index for the bone where the pin is attached to", },
					},
					{
						name => 'boneSphereRadius', type => 'F32',
						hints => { shortDescription => "radius for the sphere the pin influence on", },
					},
					{
						name => 'boneSphereLocalPos', type => 'VEC3',
						hints => { shortDescription => "offset value with regard to bind position of the bone", },
					},
					{
						name => 'pinStiffness',
						type => 'F32',
						defaultValue => '1.000000000',
						hints =>
						{
							min => '0.000000000',
							max => '1.000000000',
							shortDescription => "pin stiffness",
							longDescription => "stiffness for pin constraints",
						},
					},
					{
						name => 'influenceFallOff',
						type => 'F32',
						defaultValue => '1.000000000',
						hints =>
						{
							min => '0.000000000',
							max => '1.000000000',
							shortDescription => "influence fall off",
							longDescription => "how soft/hard the fall off of the hair pin zone of influence is",
						},
					},
					{
						name => 'useDynamicPin',
						type => 'BOOL',
						defaultValue => 'false',
						hints =>
						{
							shortDescription => "use dynamic pin",
							longDescription => "whether to use dynamic pin or bone based pin",
						},
					},
					{
						name => 'doLra',
						type => 'BOOL',
						defaultValue => 'false',
						hints =>
						{
							shortDescription => "enable tether pin",
							longDescription => "whether to enable tether pin, when enabled causes multiple dynamic pins to act like ball and socket joints tethered to a bone",
						},
					},
					{
						name => 'useStiffnessPin',
						type => 'BOOL',
						defaultValue => 'false',
						hints =>
						{
							shortDescription => "use stiffness pin",
							longDescription => "whether to use stiffness pin, if useStiffnessPin is true, it will ignore global stiffness",
						},
					},
					{
						name => 'influenceFallOffCurve',
						type => 'VEC4',
						defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000, 1.000000000)',
						hints =>
						{
							min => "0.0",
							max => '1.0',
							shortDescription => "curve for influence fall off",
							longDescription => "influence fall off curve",
						},
					},
				],
			},
		],

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
				name => 'numBoneSpheres',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					shortDescription => "number of bone spheres",
					longDescription => "collision spheres used for collision handling between body and hair"
				},
			},
			{
				name => 'boneSpheres',
				type => 'BoneSphere',
				isArray => '1',
				arraySize => '-1',
				hints =>
				{
					shortDescription => "bone spheres",
					longDescription => "collision sphere data, each sphere is attached to a bone. The size of this array should be numBoneSpheres"
				},
			},
			{
				name => 'numBoneCapsules',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					shortDescription => "number of bone capsules",
					longDescription => "capsules are defined by connecting a pair of collision spheres",
				},
			},
			{
				name => 'boneCapsuleIndices',
				type => 'U32',
				isArray => '1',
				arraySize => '-1',
				hints =>
				{
					shortDescription => "bone capsule indices",
					longDescription => "index to the bone spheres, size of this array must be 2 * numBoneCapsules",
				},
			},
			{
				name => 'numPinConstraints',
				type => 'U32',
				defaultValue => '0',
				hints =>
				{
					shortDescription => "number of constraint spheres",
					longDescription => "spheres used to apply soft constraint to pin hairs around the spheres."
				},
			},
			{
				name => 'pinConstraints',
				type => 'Pin',
				isArray => '1',
				arraySize => '-1',
				hints =>
				{
					shortDescription => "constraint spheres",
					longDescription => "constraint sphere data, each sphere is attached to a bone. "
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
			# 1.1 Added friction and useCollision
			classVersion => '1.1',

			hints =>
			{
			},
		},

		structs =>
		[
			{
				name => 'Material',

			parameters =>
			[
				{
					name => 'name',
					type => 'STRING',
					hints =>
					{
						shortDescription => "material name",
						longDescription => "material name",
					},
				},

				{
					name => 'densityTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{
					name => 'widthTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{
					name => 'rootWidthTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{
					name => 'tipWidthTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{
					name => 'clumpScaleTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
					
				{	
					name => 'clumpNoiseTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{	
					name => 'clumpRoundnessTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{
					name => 'waveScaleTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{
					name => 'waveFreqTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},
				{
					name => 'lengthTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},

				{
					name => 'stiffnessTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},

				{
					name => 'rootStiffnessTextureChan',
					type => 'U32',
					defaultValue => '0.0',
				},

				{
					name => 'splineMultiplier',
					type => 'U32',
					defaultValue => '4.0000000',
				},

				{
					name => 'assetType',
					type => 'U32',
					defaultValue => '0',
				},

				{
					name => 'assetPriority',
					type => 'U32',
					defaultValue => '0',
				},

				{
					name => 'assetGroup',
					type => 'U32',
					defaultValue => '0',
				},


				{
					name => 'width',
					type => 'F32',
					defaultValue => '2.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '10.000000000',
						shortDescription => "width",
						longDescription => "[UNIT DEPENDENT] hair width ",
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
					name => 'clumpNumSubclumps',
					type => 'U32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '8.000000000',
						shortDescription => "clump num sub clumps",
						longDescription => "number of clumps per triangle",
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
					name => 'clumpPerVertex',
					type => 'BOOL',
					defaultValue => 'true',
					hints =>
					{
						shortDescription => "use per vertex clump or per face clump",
						longDescription => "whether to use per vertex or per face clumping mode",
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
					name => 'waveScale',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "wave scale",
						longDescription => "[UNIT DEPENDENT] size of waves for hair waviness ",
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
					name => 'waveScaleStrand',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "wave scale",
						longDescription => "[UNIT DEPENDENT] size of waves for hair waviness ",
					},
				},
				{
					name => 'waveScaleClump',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "wave clump",
						longDescription => "[UNIT DEPENDENT] size of waves for hair waviness ",
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
						max => '10.000000000',
						shortDescription => "LOD fade start",
						longDescription => "[UNIT DEPENDENT] distance (in scene unit) to camera where fur will start fading out (by reducing density)",
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
						longDescription => "[UNIT DEPENDENT] distance (in scene unit) to camera where fur will completely disappear (and stop simulating)",
					},
				},
				{
					name => 'distanceLODFadeStart',
					type => 'F32',
					defaultValue => '1000.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '10.000000000',
						shortDescription => "LOD fade start",
						longDescription => "[UNIT DEPENDENT] distance (in scene unit) to camera where fur will start fading out (by reducing density)",
					},
				},
				{
					name => 'distanceLODDensity',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '10.000000000',
						shortDescription => "distance lod density",
						longDescription => "",
					},
				},
				{
					name => 'distanceLODWidth',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '1.000000000',
						max => '10.000000000',
						shortDescription => "distance lod width scale",
						longDescription => "[UNIT DEPENDENT] hair width that can change when lod is used",
					},
				},
				{
					name => 'enableDetailLOD',
					type => 'BOOL',
					defaultValue => 'true',
					hints =>
					{
						shortDescription => "Enable Closeup LOD",
						longDescription => "whether to enable lod for close object (closeup LOD)",
					},
				},			
				{
					name => 'detailLODStart',
					type => 'F32',
					defaultValue => '2.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '10.000000000',
						shortDescription => "closeup start",
						longDescription => "[UNIT DEPENDENT] distance (in scene unit) to camera where fur will start getting denser toward closeup density",
					},
				},
				{
					name => 'detailLODEnd',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '10.000000000',
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
						max => '10.000000000',
						shortDescription => "closeup density",
						longDescription => "ratio of number of interpolated hairs compared to maximum in closeup (can be larger than 1, 1 = 64 hairs per face)",
					},
				},
				{
					name => 'detailLODWidth',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '1.000000000',
						max => '10.000000000',
						shortDescription => "closeup width",
						longDescription => "[UNIT DEPENDENT] hair width that can change when close up density is triggered by closeup lod mechanism",
					},
				},

				{
					name => 'colorizeLODOption',
					type => 'U32',
					hints =>
					{
						shortDescription => "colirize LOD",
						longDescription => "show LOD factor in color (works only when non-custom pixel shader is used)",
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
					name => 'usePixelDensity',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "use per pixel density ",
						longDescription => "use per pixel density for density map sampling",
					},
				},
				{
					name => 'alpha',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "alpha",
						longDescription => "alpha value used for all hairs",
					},
				},
				{
					name => 'strandBlendScale',
					type => 'F32',
					defaultValue => '0.300000012',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "strand blend scale",
						longDescription => "scale for strand texture blending",
					},
				},
				{
					name => 'baseColor',
					type => 'VEC4',
					defaultValue => 'initVec4(0.5, 0.5, 0.5, 1.0)',
					hints =>
					{
						shortDescription => "base color",
						longDescription => "base color (when color textures are not used)",
					},
				},
				{
					name => 'diffuseBlend',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "diffuse blend",
						longDescription => "blend factor between tangent based hair lighting vs normal based skin lighting (0 = all tangent, 1 = all normal)",
					},
				},
				{
					name => 'diffuseScale',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "diffuse scale",
						longDescription => "scale term for diffuse lighting",
					},
				},
				{
					name => 'diffuseHairNormalWeight',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "diffuse hair normal weight",
						longDescription => "blend factor between mesh normal and long hair normal",
					},
				},
				{
					name => 'diffuseBoneIndex',
					type => 'U32',
					hints =>
					{
						shortDescription => "bone index for diffuse hair normal",
						longDescription => "bone index for diffuse hair normal",
					},
				},
				{
					name => 'diffuseBoneLocalPos',
					type => 'VEC3',
					defaultValue => 'init(0.000000000,0.000000000,0.000000000)',
					hints =>
					{
						min => "-1000.0",
						max => '1000.0',
						shortDescription => "diffuse local pos",
						longDescription => "[UNIT DEPENDENT] offset for diffuse bone center",
					},
				},
				{
					name => 'diffuseNoiseFreqU',
					type => 'F32',
					defaultValue => '64.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1024.000000000',
						shortDescription => "diffuse noise frequency along u",
						longDescription => "diffuse noise frequency along u",
					},
				},
				{
					name => 'diffuseNoiseFreqV',
					type => 'F32',
					defaultValue => '64.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1024.000000000',
						shortDescription => "diffuse noise frequency along v",
						longDescription => "diffuse noise frequency along v",
					},
				},
				{
					name => 'diffuseNoiseScale',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "diffuse noise scale",
						longDescription => "diffuse noise scale",
					},
				},
				{
					name => 'diffuseNoiseGain',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "diffuse noise gain",
						longDescription => "diffuse noise gain",
					},
				},

				{
					name => 'textureBrightness',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '2.000000000',
						shortDescription => "texture brightness",
						longDescription => "additional texture brightness control",
					},
				},
				{
					name => 'diffuseColor',
					type => 'VEC4',
					defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000,1.000000000)',
					hints =>
					{
						min => "0.0",
						max => '1.0',
						shortDescription => "diffuse color",
						longDescription => "diffuse lighting color (when specular textures are not used)",
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
					name => 'glintStrength',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						shortDescription => "glint strength",
						longDescription => "strength of glint effect",
					},
				},
				{
					name => 'glintCount',
					type => 'F32',
					defaultValue => '256.0',
					hints =>
					{
						shortDescription => "glint count",
						longDescription => "number of glint sparklets per hair",
					},
				},
				{
					name => 'glintExponent',
					type => 'F32',
					defaultValue => '2.000000000',
					hints =>
					{
						shortDescription => "glint exponent",
						longDescription => "power exponent of glint values",
					},
				},
				{
					name => 'rootAlphaFalloff',
					type => 'F32',
					defaultValue => '0.00000000',
					hints =>
					{
						shortDescription => "root alpha falloff",
						longDescription => "bias factor for root/tip alpha blending ",
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
					name => 'rootTipColorFalloff',
					type => 'F32',
					defaultValue => '1.0',
					hints =>
					{
						shortDescription => "root tip color falloff",
						longDescription => "falloff factor for root/tip color interpolation ",
					},
				},
				{
					name => 'shadowSigma',
					type => 'F32',
					defaultValue => '0.200000003',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "shadow attenuation",
						longDescription => "[UNIT DEPENDENT] shadow absorption factor",
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
					name => 'specularNoiseScale',
					type => 'F32',
					defaultValue => '0.100000001',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "specular noise",
						longDescription => "noise for primary specular factor",
					},
				},
				{
					name => 'specularEnvScale',
					type => 'F32',
					defaultValue => '0.100000001',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "specular env scale",
						longDescription => "scale factor for env specular",
					},
				},
				{
					name => 'specularPrimaryBreakup',
					type => 'F32',
					defaultValue => '0.0',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "specular primary breakup",
						longDescription => "noise factor to break up specular primary",
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
					name => 'strandBlendMode',
					type => 'U32',
					defaultValue => '0',
					hints =>
					{
						min => '0.000000000',
						max => '2.000000000',
						shortDescription => "blend mode for per strand texture",
						longDescription => "blend mode for per strand texture",
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
					name => 'useShadows',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "use shadows",
						longDescription => "turn on/off shadow",
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
						shortDescription => "specular env scale",
						longDescription => "scale factor for env specular",
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
					name => 'bendStiffness',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "bend stiffness",
						longDescription => "stiffness for bending, useful for long hair",
					},
				},			
				{
					name => 'interactionStiffness',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "interaction stiffness",
						longDescription => "stiffness for hair interaction, useful for long hair",
					},
				},			
				{
					name => 'pinStiffness',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "pin stiffness",
						longDescription => "stiffness for pin constraints",
					},
				},
				{
					name => 'collisionOffset',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						shortDescription => "collision offset",
						longDescription => "[UNUSED] additional body offset for hair/body collision",
					},
				},
				{
					name => 'useCollision',
					type => 'BOOL',
					defaultValue => 'false',
					hints =>
					{
						shortDescription => "use collision",
						longDescription => "whether to use the sphere/capsule collision or not for hair/body collision",
					},
				},
				{
					name => 'useDynamicPin',
					type => 'BOOL',
					defaultValue => 'false',
					hints =>
					{
						shortDescription => "use dynamic pin",
						longDescription => "whether to use dynamic pin or bone based pin",
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
					name => 'friction',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '10.0',
						shortDescription => "friction",
						longDescription => "friction to slow down hair motion",
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
					name => 'gravity',
					type => 'VEC3',
					defaultValue => 'init(0.000000000,0.000000000,0.000000000)',
					hints =>
					{
						min => "-1000.0",
						max => '1000.0',
						shortDescription => "gravity",
						longDescription => "[UNIT DEPENDENT] gravity force vector",
					},
				},
				{
					name => 'inertiaScale',
					type => 'F32',
					defaultValue => '1.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "inertia scale",
						longDescription => "Interia control. 0: no inertia, 1: full intertia",
					},
				},
				{
					name => 'inertiaLimit',
					type => 'F32',
					defaultValue => '1000.000000',
					hints =>
					{
						min => '0.000000000',
						max => '1000.000000000',
						shortDescription => "inertia limit",
						longDescription => "[UNIT DEPENDENT] Speed limit beyond which everything gets locked to skinned position (teleport, etc.)",
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
					name => 'tipStiffness',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "tip stiffness",
						longDescription => "attenuation of stiffness away from the tip (stiffer at tip, weaker toward root)",
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
					name => 'stiffnessStrength',
					type => 'F32',
					defaultValue => '1.00000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "stiffness strength",
						longDescription => "stiffness to restore to skinned rest shape for hairs",
					},
				},
				{
					name => 'stiffnessDamping',
					type => 'F32',
					defaultValue => '0.000000000',
					hints =>
					{
						min => '0.000000000',
						max => '1.000000000',
						shortDescription => "stiffness damping",
						longDescription => "stiffness to restore to skinned rest shape for hairs",
					},
				},
				{
					name => 'stiffnessCurve',
					type => 'VEC4',
					defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000, 1.0)',
					hints =>
					{
						min => "0.0",
						max => '1.0',
						shortDescription => "curve for stiffness",
						longDescription => "stiffness curve",
					},
				},
				{
					name => 'stiffnessStrengthCurve',
					type => 'VEC4',
					defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000, 1.0)',
					hints =>
					{
						min => "0.0",
						max => '1.0',
						shortDescription => "curve for stiffness",
						longDescription => "stiffness curve",
					},
				},
				{
					name => 'stiffnessDampingCurve',
					type => 'VEC4',
					defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000, 1.0)',
					hints =>
					{
						min => "0.0",
						max => '1.0',
						shortDescription => "curve for stiffness",
						longDescription => "stiffness curve",
					},
				},
				{
					name => 'bendStiffnessCurve',
					type => 'VEC4',
					defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000, 1.0)',
					hints =>
					{
						min => "0.0",
						max => '1.0',
						shortDescription => "curve for stiffness",
						longDescription => "stiffness curve",
					},
				},
				{
					name => 'interactionStiffnessCurve',
					type => 'VEC4',
					defaultValue => 'initVec4(1.000000000,1.000000000,1.000000000, 1.0)',
					hints =>
					{
						min => "0.0",
						max => '1.0',
						shortDescription => "curve for stiffness",
						longDescription => "stiffness curve",
					},
				},
				{
					name => 'wind',
					type => 'VEC3',
					defaultValue => 'init(0.000000000,0.000000000,0.000000000)',
					hints =>
					{
						min => "-10.0",
						max => '10.0',
						shortDescription => "wind",
						longDescription => "[UNIT DEPENDENT] vector force for main wind direction",
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
					name => 'visualizeBoundingBox',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize bounding box",
						longDescription => "draw bounding box ",
					},
				},
				{
					name => 'visualizeCapsules',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize capsules",
						longDescription => "draw collision capsules",
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
					name => 'visualizeCullSphere',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "draw cull sphere",
						longDescription => "draw cull sphere",
					},
				},
				{
					name => 'visualizeDiffuseBone',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize diffuse bone",
						longDescription => "visualize diffuse bone",
					},
				},
				{
					name => 'visualizeFrames',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize framess",
						longDescription => "draw frames ",
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
					name => 'visualizeHairInteractions',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize hair interaction connections",
						longDescription => "draw hair interaction",
					},
				},
				{
					name => 'visualizeHairSkips',
					type => 'U32',
					defaultValue => '0',
					hints =>
					{
						min => '0.000000000',
						max => '10000.000000000',
						shortDescription => "hair skips",
						longDescription => "how many hairs to skip in visualization",
					},
				},
				{
					name => 'visualizeLocalPos',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize local pos",
						longDescription => "draw local pos ",
					},
				},
				{
					name => 'visualizePinConstraints',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize pin constraints",
						longDescription => "draw pin constraints ",
					},
				},
				{
					name => 'visualizeShadingNormals',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize shading normals",
						longDescription => "visualize shading normals",
					},
				},

				{
					name => 'visualizeSkinnedGuideHairs',
					type => 'BOOL',
					hints =>
					{
						shortDescription => "visualize skinned pose for guide hairs",
						longDescription => "draw skinned guide hairs",
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
					name => 'enable',
					type => 'BOOL',
					defaultValue => 'true',
					hints =>
					{
						shortDescription => "enable",
						longDescription => "enable rendering and simulation",
					},
				},

			],
			}
		],
		
		parameters =>
		[
			{
				name => 'materials',
				type => 'Material',
				isArray => '1',
				arraySize => '-1',
				hints =>
				{
					shortDescription => "additional hair materials",
					longDescription => "additional hair materials"
				},
			},
		],
	},
]
