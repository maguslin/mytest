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
// Resource bind info for g_masterStrandVertices
// {
//
//   float4 $Element;                   // Offset:    0 Size:    16
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// g_masterStrandVertices            texture  struct         r/o             t0      1 
// g_interactionIndices              texture    sint         buf             t1      1 
// g_interactionLength               texture   float         buf             t3      1 
// cbPerFrame                        cbuffer      NA          NA            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// VertexID                 0   x           0     NONE     int   x   
// START_VertexID           0    y          0     NONE     int    y  
// END_VertexID             0     z         0     NONE     int     z 
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xyzw
// RES                      0   x           1     NONE   float   x   
//
gs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[8], immediateIndexed
dcl_resource_structured t0, 16
dcl_resource_buffer (sint,sint,sint,sint) t1
dcl_resource_buffer (float,float,float,float) t3
dcl_input v[1][0].x
dcl_input v[1][0].y
dcl_input v[1][0].z
dcl_temps 5
dcl_inputprimitive point 
dcl_stream m0
dcl_outputtopology linestrip 
dcl_output_siv o0.xyzw, position
dcl_output o1.x
dcl_maxout 10
iadd r0.x, -v[0][0].y, v[0][0].z
ld_structured_indexable(structured_buffer, stride=16)(mixed,mixed,mixed,mixed) r0.yzw, v[0][0].x, l(0), t0.xxyz
mul r1.xyz, r0.zzzz, cb0[5].xyzx
mad r1.xyz, r0.yyyy, cb0[4].xyzx, r1.xyzx
mad r1.xyz, r0.wwww, cb0[6].xyzx, r1.xyzx
add r1.xyz, r1.xyzx, cb0[7].xyzx
mul r2.xyzw, r1.yyyy, cb0[1].xyzw
mad r2.xyzw, r1.xxxx, cb0[0].xyzw, r2.xyzw
mad r1.xyzw, r1.zzzz, cb0[2].xyzw, r2.xyzw
add r1.xyzw, r1.xyzw, cb0[3].xyzw
mov r2.x, l(0)
loop 
  ige r2.y, r2.x, r0.x
  breakc_nz r2.y
  iadd r2.y, r2.x, v[0][0].y
  ld_indexable(buffer)(sint,sint,sint,sint) r2.z, r2.yyyy, t1.yzxw
  ld_structured_indexable(structured_buffer, stride=16)(mixed,mixed,mixed,mixed) r3.xyz, r2.z, l(0), t0.xyzx
  ld_indexable(buffer)(float,float,float,float) r2.y, r2.yyyy, t3.yxzw
  add r4.xyz, r0.yzwy, -r3.xyzx
  dp3 r2.z, r4.xyzx, r4.xyzx
  sqrt r2.z, r2.z
  add r2.z, -r2.y, r2.z
  div r2.y, r2.z, r2.y
  mov o0.xyzw, r1.xyzw
  mov o1.x, r2.y
  emit_stream m0
  mul r4.xyz, r3.yyyy, cb0[5].xyzx
  mad r3.xyw, r3.xxxx, cb0[4].xyxz, r4.xyxz
  mad r3.xyz, r3.zzzz, cb0[6].xyzx, r3.xywx
  add r3.xyz, r3.xyzx, cb0[7].xyzx
  mul r4.xyzw, r3.yyyy, cb0[1].xyzw
  mad r4.xyzw, r3.xxxx, cb0[0].xyzw, r4.xyzw
  mad r3.xyzw, r3.zzzz, cb0[2].xyzw, r4.xyzw
  add r3.xyzw, r3.xyzw, cb0[3].xyzw
  mov o0.xyzw, r3.xyzw
  mov o1.x, r2.y
  emit_stream m0
  cut_stream m0
  iadd r2.x, r2.x, l(1)
endloop 
ret 
// Approximately 41 instruction slots used
#endif

const BYTE g_gs_main[] =
{
     68,  88,  66,  67, 225,  14, 
     35,  90,  22,  56, 169, 191, 
    218, 142, 167, 104, 105,  87, 
     66, 252,   1,   0,   0,   0, 
    196,  10,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    252,   3,   0,   0, 124,   4, 
      0,   0, 212,   4,   0,   0, 
     40,  10,   0,   0,  82,  68, 
     69,  70, 192,   3,   0,   0, 
      2,   0,   0,   0,   8,   1, 
      0,   0,   4,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
     83,  71,   0,   1,   0,   0, 
    152,   3,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
    188,   0,   0,   0,   5,   0, 
      0,   0,   6,   0,   0,   0, 
      1,   0,   0,   0,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 211,   0,   0,   0, 
      2,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
    255, 255, 255, 255,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 232,   0, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
      0,   0, 255, 255, 255, 255, 
      3,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    252,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 103,  95, 109,  97, 
    115, 116, 101, 114,  83, 116, 
    114,  97, 110, 100,  86, 101, 
    114, 116, 105,  99, 101, 115, 
      0, 103,  95, 105, 110, 116, 
    101, 114,  97,  99, 116, 105, 
    111, 110,  73, 110, 100, 105, 
     99, 101, 115,   0, 103,  95, 
    105, 110, 116, 101, 114,  97, 
     99, 116, 105, 111, 110,  76, 
    101, 110, 103, 116, 104,   0, 
     99,  98,  80, 101, 114,  70, 
    114,  97, 109, 101,   0, 171, 
    252,   0,   0,   0,   1,   0, 
      0,   0,  56,   1,   0,   0, 
    176,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    188,   0,   0,   0,   1,   0, 
      0,   0,  64,   3,   0,   0, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
     96,   1,   0,   0,   0,   0, 
      0,   0, 176,   0,   0,   0, 
      2,   0,   0,   0,  28,   3, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 103,  95, 
     98, 117, 102, 102, 101, 114, 
      0,  78, 118,  72,  97, 105, 
    114,  95,  86, 105, 115, 117, 
     97, 108, 105, 122, 101,  67, 
    111, 110, 115, 116,  97, 110, 
    116,  66, 117, 102, 102, 101, 
    114,   0, 118, 105, 101, 119, 
     80, 114, 111, 106, 101,  99, 
    116, 105, 111, 110,   0, 102, 
    108, 111,  97, 116,  52, 120, 
     52,   0,   2,   0,   3,   0, 
      4,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 151,   1, 
      0,   0, 109, 111, 100, 101, 
    108,  84, 111,  87, 111, 114, 
    108, 100,   0,  99, 111, 108, 
    111, 114,   0, 102, 108, 111, 
     97, 116,  52,   0, 171, 171, 
      1,   0,   3,   0,   1,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 215,   1,   0,   0, 
    104,  97, 105, 114,  77, 105, 
    110,   0, 105, 110, 116,   0, 
      0,   0,   2,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  12,   2,   0,   0, 
    104,  97, 105, 114,  77,  97, 
    120,   0, 104,  97, 105, 114, 
     83, 107, 105, 112,   0, 104, 
     97, 105, 114,  68, 117, 109, 
    109, 121,   0, 104,  97, 105, 
    114,  87, 105, 100, 116, 104, 
      0, 102, 108, 111,  97, 116, 
      0, 171,   0,   0,   3,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  89,   2, 
      0,   0,  97, 115, 112, 101, 
     99, 116,   0, 115,  99,  97, 
    108, 101,   0, 112, 105, 110, 
     73, 100,   0, 171, 136,   1, 
      0,   0, 160,   1,   0,   0, 
      0,   0,   0,   0, 196,   1, 
      0,   0, 160,   1,   0,   0, 
     64,   0,   0,   0, 209,   1, 
      0,   0, 224,   1,   0,   0, 
    128,   0,   0,   0,   4,   2, 
      0,   0,  16,   2,   0,   0, 
    144,   0,   0,   0,  52,   2, 
      0,   0,  16,   2,   0,   0, 
    148,   0,   0,   0,  60,   2, 
      0,   0,  16,   2,   0,   0, 
    152,   0,   0,   0,  69,   2, 
      0,   0,  16,   2,   0,   0, 
    156,   0,   0,   0,  79,   2, 
      0,   0,  96,   2,   0,   0, 
    160,   0,   0,   0, 132,   2, 
      0,   0,  96,   2,   0,   0, 
    164,   0,   0,   0, 139,   2, 
      0,   0,  96,   2,   0,   0, 
    168,   0,   0,   0, 145,   2, 
      0,   0,  16,   2,   0,   0, 
    172,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,  44,   0, 
      0,   0,  11,   0, 152,   2, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    105,   1,   0,   0, 104,   3, 
      0,   0,   0,   0,   0,   0, 
     16,   0,   0,   0,   2,   0, 
      0,   0, 116,   3,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  36,  69, 108, 101, 
    109, 101, 110, 116,   0, 171, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 215,   1, 
      0,   0,  77, 105,  99, 114, 
    111, 115, 111, 102, 116,  32, 
     40,  82,  41,  32,  72,  76, 
     83,  76,  32,  83, 104,  97, 
    100, 101, 114,  32,  67, 111, 
    109, 112, 105, 108, 101, 114, 
     32,  49,  48,  46,  49,   0, 
     73,  83,  71,  78, 120,   0, 
      0,   0,   3,   0,   0,   0, 
      8,   0,   0,   0,  80,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   1,   0,   0,  89,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      2,   2,   0,   0, 104,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   4,   0,   0,  86, 101, 
    114, 116, 101, 120,  73,  68, 
      0,  83,  84,  65,  82,  84, 
     95,  86, 101, 114, 116, 101, 
    120,  73,  68,   0,  69,  78, 
     68,  95,  86, 101, 114, 116, 
    101, 120,  73,  68,   0, 171, 
    171, 171,  79,  83,  71,  53, 
     80,   0,   0,   0,   2,   0, 
      0,   0,   8,   0,   0,   0, 
      0,   0,   0,   0,  64,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0,   0,   0, 
      0,   0,  76,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   1,  14, 
      0,   0,  83,  86,  95,  80, 
    111, 115, 105, 116, 105, 111, 
    110,   0,  82,  69,  83,   0, 
     83,  72,  69,  88,  76,   5, 
      0,   0,  80,   0,   2,   0, 
     83,   1,   0,   0, 106,   8, 
      0,   1,  89,   0,   0,   4, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
    162,   0,   0,   4,   0, 112, 
     16,   0,   0,   0,   0,   0, 
     16,   0,   0,   0,  88,   8, 
      0,   4,   0, 112,  16,   0, 
      1,   0,   0,   0,  51,  51, 
      0,   0,  88,   8,   0,   4, 
      0, 112,  16,   0,   3,   0, 
      0,   0,  85,  85,   0,   0, 
     95,   0,   0,   4,  18,  16, 
     32,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,  95,   0, 
      0,   4,  34,  16,  32,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,  95,   0,   0,   4, 
     66,  16,  32,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   5,   0, 
      0,   0,  93,   8,   0,   1, 
    143,   0,   0,   3,   0,   0, 
     17,   0,   0,   0,   0,   0, 
     92,  24,   0,   1, 103,   0, 
      0,   4, 242,  32,  16,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0, 101,   0,   0,   3, 
     18,  32,  16,   0,   1,   0, 
      0,   0,  94,   0,   0,   2, 
     10,   0,   0,   0,  30,   0, 
      0,  10,  18,   0,  16,   0, 
      0,   0,   0,   0,  26,  16, 
     32, 128,  65,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  42,  16,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 167,   0,   0, 140, 
      2, 131,   0, 128, 131, 153, 
     25,   0, 226,   0,  16,   0, 
      0,   0,   0,   0,  10,  16, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 121,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   8, 
    114,   0,  16,   0,   1,   0, 
      0,   0, 166,  10,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      5,   0,   0,   0,  50,   0, 
      0,  10, 114,   0,  16,   0, 
      1,   0,   0,   0,  86,   5, 
     16,   0,   0,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   4,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    114,   0,  16,   0,   1,   0, 
      0,   0, 246,  15,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   8, 114,   0, 
     16,   0,   1,   0,   0,   0, 
     70,   2,  16,   0,   1,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,  56,   0,   0,   8, 
    242,   0,  16,   0,   2,   0, 
      0,   0,  86,   5,  16,   0, 
      1,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 242,   0,  16,   0, 
      2,   0,   0,   0,   6,   0, 
     16,   0,   1,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   2,   0, 
      0,   0,  50,   0,   0,  10, 
    242,   0,  16,   0,   1,   0, 
      0,   0, 166,  10,  16,   0, 
      1,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
      0,   0,   0,   8, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  54,   0,   0,   5, 
     18,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  48,   0, 
      0,   1,  33,   0,   0,   7, 
     34,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      3,   0,   4,   3,  26,   0, 
     16,   0,   2,   0,   0,   0, 
     30,   0,   0,   8,  34,   0, 
     16,   0,   2,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0,  26,  16,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  45,   0,   0, 137, 
     66,   0,   0, 128, 195, 204, 
     12,   0,  66,   0,  16,   0, 
      2,   0,   0,   0,  86,   5, 
     16,   0,   2,   0,   0,   0, 
    150, 124,  16,   0,   1,   0, 
      0,   0, 167,   0,   0, 139, 
      2, 131,   0, 128, 131, 153, 
     25,   0, 114,   0,  16,   0, 
      3,   0,   0,   0,  42,   0, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  70, 114,  16,   0, 
      0,   0,   0,   0,  45,   0, 
      0, 137,  66,   0,   0, 128, 
     67,  85,  21,   0,  34,   0, 
     16,   0,   2,   0,   0,   0, 
     86,   5,  16,   0,   2,   0, 
      0,   0,  22, 126,  16,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   8, 114,   0,  16,   0, 
      4,   0,   0,   0, 150,   7, 
     16,   0,   0,   0,   0,   0, 
     70,   2,  16, 128,  65,   0, 
      0,   0,   3,   0,   0,   0, 
     16,   0,   0,   7,  66,   0, 
     16,   0,   2,   0,   0,   0, 
     70,   2,  16,   0,   4,   0, 
      0,   0,  70,   2,  16,   0, 
      4,   0,   0,   0,  75,   0, 
      0,   5,  66,   0,  16,   0, 
      2,   0,   0,   0,  42,   0, 
     16,   0,   2,   0,   0,   0, 
      0,   0,   0,   8,  66,   0, 
     16,   0,   2,   0,   0,   0, 
     26,   0,  16, 128,  65,   0, 
      0,   0,   2,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,  14,   0,   0,   7, 
     34,   0,  16,   0,   2,   0, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0,  26,   0, 
     16,   0,   2,   0,   0,   0, 
     54,   0,   0,   5, 242,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      2,   0,   0,   0, 117,   0, 
      0,   3,   0,   0,  17,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   8, 114,   0,  16,   0, 
      4,   0,   0,   0,  86,   5, 
     16,   0,   3,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   5,   0,   0,   0, 
     50,   0,   0,  10, 178,   0, 
     16,   0,   3,   0,   0,   0, 
      6,   0,  16,   0,   3,   0, 
      0,   0,  70, 136,  32,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,  70,   8,  16,   0, 
      4,   0,   0,   0,  50,   0, 
      0,  10, 114,   0,  16,   0, 
      3,   0,   0,   0, 166,  10, 
     16,   0,   3,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
     70,   3,  16,   0,   3,   0, 
      0,   0,   0,   0,   0,   8, 
    114,   0,  16,   0,   3,   0, 
      0,   0,  70,   2,  16,   0, 
      3,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      7,   0,   0,   0,  56,   0, 
      0,   8, 242,   0,  16,   0, 
      4,   0,   0,   0,  86,   5, 
     16,   0,   3,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 242,   0, 
     16,   0,   4,   0,   0,   0, 
      6,   0,  16,   0,   3,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      4,   0,   0,   0,  50,   0, 
      0,  10, 242,   0,  16,   0, 
      3,   0,   0,   0, 166,  10, 
     16,   0,   3,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
     70,  14,  16,   0,   4,   0, 
      0,   0,   0,   0,   0,   8, 
    242,   0,  16,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      3,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,  54,   0, 
      0,   5, 242,  32,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   3,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   1,   0,   0,   0, 
     26,   0,  16,   0,   2,   0, 
      0,   0, 117,   0,   0,   3, 
      0,   0,  17,   0,   0,   0, 
      0,   0, 118,   0,   0,   3, 
      0,   0,  17,   0,   0,   0, 
      0,   0,  30,   0,   0,   7, 
     18,   0,  16,   0,   2,   0, 
      0,   0,  10,   0,  16,   0, 
      2,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     22,   0,   0,   1,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    148,   0,   0,   0,  41,   0, 
      0,   0,   5,   0,   0,   0, 
      0,   0,   0,   0,   5,   0, 
      0,   0,  21,   0,   0,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,  10,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0
};
