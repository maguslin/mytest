#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer cbPerFrame
// {
//
//   struct NvHair_VisualizeConstantBuffer
//   {
//       
//       row_major float4x4 viewProjection;// Offset:    0
//       row_major float4x4 modelToWorld;// Offset:   64
//       float4 color;                  // Offset:  128
//       int hairMin;                   // Offset:  144
//       int hairMax;                   // Offset:  148
//       int hairSkip;                  // Offset:  152
//       int hairDummy;                 // Offset:  156
//       float hairWidth;               // Offset:  160
//       float aspect;                  // Offset:  164
//       float scale;                   // Offset:  168
//       int pinId;                     // Offset:  172
//
//   } g_buffer;                        // Offset:    0 Size:   176
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// cbPerFrame                        cbuffer      NA          NA            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyz         0     NONE   float   xyz 
// COLOR                    0   xyzw        1     NONE   float   xyzw
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xyzw
// COLOR                    0   xyzw        1     NONE   float   xyzw
//
vs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[9], immediateIndexed
dcl_input v0.xyz
dcl_input v1.xyzw
dcl_output_siv o0.xyzw, position
dcl_output o1.xyzw
dcl_temps 2
mul r0.xyz, v0.yyyy, cb0[5].xyzx
mad r0.xyz, v0.xxxx, cb0[4].xyzx, r0.xyzx
mad r0.xyz, v0.zzzz, cb0[6].xyzx, r0.xyzx
add r0.xyz, r0.xyzx, cb0[7].xyzx
mul r1.xyzw, r0.yyyy, cb0[1].xyzw
mad r1.xyzw, r0.xxxx, cb0[0].xyzw, r1.xyzw
mad r0.xyzw, r0.zzzz, cb0[2].xyzw, r1.xyzw
add o0.xyzw, r0.xyzw, cb0[3].xyzw
mul o1.xyzw, v1.xyzw, cb0[8].xyzw
ret 
// Approximately 10 instruction slots used
#endif

const BYTE g_vs_main[] =
{
     68,  88,  66,  67,  46, 248, 
    139, 223, 159, 227,  37, 171, 
    233, 139, 143, 255, 107,  70, 
    155,  17,   1,   0,   0,   0, 
    208,   5,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    236,   2,   0,   0,  60,   3, 
      0,   0, 144,   3,   0,   0, 
     52,   5,   0,   0,  82,  68, 
     69,  70, 176,   2,   0,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   0,   1,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    254, 255,   0,   1,   0,   0, 
    136,   2,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
     92,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  99,  98,  80, 101, 
    114,  70, 114,  97, 109, 101, 
      0, 171,  92,   0,   0,   0, 
      1,   0,   0,   0, 128,   0, 
      0,   0, 176,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 168,   0,   0,   0, 
      0,   0,   0,   0, 176,   0, 
      0,   0,   2,   0,   0,   0, 
    100,   2,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    103,  95,  98, 117, 102, 102, 
    101, 114,   0,  78, 118,  72, 
     97, 105, 114,  95,  86, 105, 
    115, 117,  97, 108, 105, 122, 
    101,  67, 111, 110, 115, 116, 
     97, 110, 116,  66, 117, 102, 
    102, 101, 114,   0, 118, 105, 
    101, 119,  80, 114, 111, 106, 
    101,  99, 116, 105, 111, 110, 
      0, 102, 108, 111,  97, 116, 
     52, 120,  52,   0,   2,   0, 
      3,   0,   4,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    223,   0,   0,   0, 109, 111, 
    100, 101, 108,  84, 111,  87, 
    111, 114, 108, 100,   0,  99, 
    111, 108, 111, 114,   0, 102, 
    108, 111,  97, 116,  52,   0, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  31,   1, 
      0,   0, 104,  97, 105, 114, 
     77, 105, 110,   0, 105, 110, 
    116,   0,   0,   0,   2,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  84,   1, 
      0,   0, 104,  97, 105, 114, 
     77,  97, 120,   0, 104,  97, 
    105, 114,  83, 107, 105, 112, 
      0, 104,  97, 105, 114,  68, 
    117, 109, 109, 121,   0, 104, 
     97, 105, 114,  87, 105, 100, 
    116, 104,   0, 102, 108, 111, 
     97, 116,   0, 171,   0,   0, 
      3,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    161,   1,   0,   0,  97, 115, 
    112, 101,  99, 116,   0, 115, 
     99,  97, 108, 101,   0, 112, 
    105, 110,  73, 100,   0, 171, 
    208,   0,   0,   0, 232,   0, 
      0,   0,   0,   0,   0,   0, 
     12,   1,   0,   0, 232,   0, 
      0,   0,  64,   0,   0,   0, 
     25,   1,   0,   0,  40,   1, 
      0,   0, 128,   0,   0,   0, 
     76,   1,   0,   0,  88,   1, 
      0,   0, 144,   0,   0,   0, 
    124,   1,   0,   0,  88,   1, 
      0,   0, 148,   0,   0,   0, 
    132,   1,   0,   0,  88,   1, 
      0,   0, 152,   0,   0,   0, 
    141,   1,   0,   0,  88,   1, 
      0,   0, 156,   0,   0,   0, 
    151,   1,   0,   0, 168,   1, 
      0,   0, 160,   0,   0,   0, 
    204,   1,   0,   0, 168,   1, 
      0,   0, 164,   0,   0,   0, 
    211,   1,   0,   0, 168,   1, 
      0,   0, 168,   0,   0,   0, 
    217,   1,   0,   0,  88,   1, 
      0,   0, 172,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
     44,   0,   0,   0,  11,   0, 
    224,   1,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 177,   0,   0,   0, 
     77, 105,  99, 114, 111, 115, 
    111, 102, 116,  32,  40,  82, 
     41,  32,  72,  76,  83,  76, 
     32,  83, 104,  97, 100, 101, 
    114,  32,  67, 111, 109, 112, 
    105, 108, 101, 114,  32,  49, 
     48,  46,  49,   0,  73,  83, 
     71,  78,  72,   0,   0,   0, 
      2,   0,   0,   0,   8,   0, 
      0,   0,  56,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   7,   7, 
      0,   0,  65,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,  15,  15, 
      0,   0,  80,  79,  83,  73, 
     84,  73,  79,  78,   0,  67, 
     79,  76,  79,  82,   0, 171, 
     79,  83,  71,  78,  76,   0, 
      0,   0,   2,   0,   0,   0, 
      8,   0,   0,   0,  56,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0,  68,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     15,   0,   0,   0,  83,  86, 
     95,  80, 111, 115, 105, 116, 
    105, 111, 110,   0,  67,  79, 
     76,  79,  82,   0, 171, 171, 
     83,  72,  69,  88, 156,   1, 
      0,   0,  80,   0,   1,   0, 
    103,   0,   0,   0, 106,   8, 
      0,   1,  89,   0,   0,   4, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   9,   0,   0,   0, 
     95,   0,   0,   3, 114,  16, 
     16,   0,   0,   0,   0,   0, 
     95,   0,   0,   3, 242,  16, 
     16,   0,   1,   0,   0,   0, 
    103,   0,   0,   4, 242,  32, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   2,   2,   0,   0,   0, 
     56,   0,   0,   8, 114,   0, 
     16,   0,   0,   0,   0,   0, 
     86,  21,  16,   0,   0,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,   5,   0, 
      0,   0,  50,   0,   0,  10, 
    114,   0,  16,   0,   0,   0, 
      0,   0,   6,  16,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     50,   0,   0,  10, 114,   0, 
     16,   0,   0,   0,   0,   0, 
    166,  26,  16,   0,   0,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   8, 114,   0,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   0,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   7,   0,   0,   0, 
     56,   0,   0,   8, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     86,   5,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    242,   0,  16,   0,   1,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 242,   0, 
     16,   0,   0,   0,   0,   0, 
    166,  10,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   8, 242,  32,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
     56,   0,   0,   8, 242,  32, 
     16,   0,   1,   0,   0,   0, 
     70,  30,  16,   0,   1,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  62,   0,   0,   1, 
     83,  84,  65,  84, 148,   0, 
      0,   0,  10,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   4,   0,   0,   0, 
      9,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0
};
