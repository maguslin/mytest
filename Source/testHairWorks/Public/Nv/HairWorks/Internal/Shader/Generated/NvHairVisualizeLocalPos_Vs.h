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
// Resource bind info for g_masterStrandFrames
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
// g_masterStrandFrames              texture  struct         r/o             t1      1 
// g_masterLocalPosPrev              texture  float4         buf             t2      1 
// g_masterLocalPosNext              texture  float4         buf             t3      1 
// g_vertexToHair                    texture    sint         buf             t4      1 
// cbPerFrame                        cbuffer      NA          NA            cb0      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_VertexID              0   x           0   VERTID    uint   x   
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xyzw
// HairID                   0   x           1     NONE     int   x   
//
vs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[9], immediateIndexed
dcl_resource_structured t0, 16
dcl_resource_structured t1, 16
dcl_resource_buffer (float,float,float,float) t2
dcl_resource_buffer (float,float,float,float) t3
dcl_resource_buffer (sint,sint,sint,sint) t4
dcl_input_sgv v0.x, vertex_id
dcl_output_siv o0.xyzw, position
dcl_output o1.x
dcl_temps 6
udiv r0.x, r1.x, v0.x, l(3)
ld_structured_indexable(structured_buffer, stride=16)(mixed,mixed,mixed,mixed) r0.yzw, r0.x, l(0), t0.xxyz
switch r1.x
  case l(0)
  mov r1.xyz, l(0,0,0,0)
  break 
  case l(1)
  ld_indexable(buffer)(float,float,float,float) r1.xyz, r0.xxxx, t2.xyzw
  break 
  case l(2)
  ld_indexable(buffer)(float,float,float,float) r1.xyz, r0.xxxx, t3.xyzw
  break 
endswitch 
mov r1.xyzw, r1.yxzy
ld_structured_indexable(structured_buffer, stride=16)(mixed,mixed,mixed,mixed) r2.xyzw, r0.x, l(0), t1.xyzw
add r1.xyzw, r1.xyzw, r1.xyzw
mad r3.x, r2.w, r2.w, l(-0.500000)
mul r3.yz, r1.yywy, r2.xxyx
add r3.y, r3.z, r3.y
mad r3.y, r2.z, r1.z, r3.y
mul r3.xzw, r1.yywz, r3.xxxx
mul r4.xyz, r1.xzwx, r2.zxxz
mad r1.x, r2.y, r1.z, -r4.x
mad r1.x, r1.x, r2.w, r3.x
mad r5.x, r2.x, r3.y, r1.x
mad r1.x, r2.z, r1.y, -r4.y
mad r1.x, r1.x, r2.w, r3.z
mad r5.y, r2.y, r3.y, r1.x
mad r1.x, -r2.y, r1.y, r4.z
mad r1.x, r1.x, r2.w, r3.w
mad r5.z, r2.z, r3.y, r1.x
mad r0.yzw, cb0[8].xxxx, r5.xxyz, r0.yyzw
mul r1.xyz, r0.zzzz, cb0[5].xyzx
mad r1.xyz, r0.yyyy, cb0[4].xyzx, r1.xyzx
mad r0.yzw, r0.wwww, cb0[6].xxyz, r1.xxyz
add r0.yzw, r0.yyzw, cb0[7].xxyz
mul r1.xyzw, r0.zzzz, cb0[1].xyzw
mad r1.xyzw, r0.yyyy, cb0[0].xyzw, r1.xyzw
mad r1.xyzw, r0.wwww, cb0[2].xyzw, r1.xyzw
add o0.xyzw, r1.xyzw, cb0[3].xyzw
ld_indexable(buffer)(sint,sint,sint,sint) r0.x, r0.xxxx, t4.xyzw
mov o1.x, r0.x
ret 
// Approximately 43 instruction slots used
#endif

const BYTE g_vs_main[] =
{
     68,  88,  66,  67,  68, 246, 
     86,  42,  40, 239,  34,  73, 
    255, 124, 157,   0, 229, 197, 
     44, 145,   1,   0,   0,   0, 
     76,  11,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    160,   4,   0,   0, 212,   4, 
      0,   0,  40,   5,   0,   0, 
    176,  10,   0,   0,  82,  68, 
     69,  70, 100,   4,   0,   0, 
      3,   0,   0,   0, 108,   1, 
      0,   0,   6,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    254, 255,   0,   1,   0,   0, 
     60,   4,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
    252,   0,   0,   0,   5,   0, 
      0,   0,   6,   0,   0,   0, 
      1,   0,   0,   0,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  19,   1,   0,   0, 
      5,   0,   0,   0,   6,   0, 
      0,   0,   1,   0,   0,   0, 
     16,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  40,   1, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   1,   0, 
      0,   0, 255, 255, 255, 255, 
      2,   0,   0,   0,   1,   0, 
      0,   0,  13,   0,   0,   0, 
     61,   1,   0,   0,   2,   0, 
      0,   0,   5,   0,   0,   0, 
      1,   0,   0,   0, 255, 255, 
    255, 255,   3,   0,   0,   0, 
      1,   0,   0,   0,  13,   0, 
      0,   0,  82,   1,   0,   0, 
      2,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
    255, 255, 255, 255,   4,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  97,   1, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    103,  95, 109,  97, 115, 116, 
    101, 114,  83, 116, 114,  97, 
    110, 100,  86, 101, 114, 116, 
    105,  99, 101, 115,   0, 103, 
     95, 109,  97, 115, 116, 101, 
    114,  83, 116, 114,  97, 110, 
    100,  70, 114,  97, 109, 101, 
    115,   0, 103,  95, 109,  97, 
    115, 116, 101, 114,  76, 111, 
     99,  97, 108,  80, 111, 115, 
     80, 114, 101, 118,   0, 103, 
     95, 109,  97, 115, 116, 101, 
    114,  76, 111,  99,  97, 108, 
     80, 111, 115,  78, 101, 120, 
    116,   0, 103,  95, 118, 101, 
    114, 116, 101, 120,  84, 111, 
     72,  97, 105, 114,   0,  99, 
     98,  80, 101, 114,  70, 114, 
     97, 109, 101,   0,  97,   1, 
      0,   0,   1,   0,   0,   0, 
    180,   1,   0,   0, 176,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 252,   0, 
      0,   0,   1,   0,   0,   0, 
    188,   3,   0,   0,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,  19,   1, 
      0,   0,   1,   0,   0,   0, 
     20,   4,   0,   0,  16,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0, 220,   1, 
      0,   0,   0,   0,   0,   0, 
    176,   0,   0,   0,   2,   0, 
      0,   0, 152,   3,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 103,  95,  98, 117, 
    102, 102, 101, 114,   0,  78, 
    118,  72,  97, 105, 114,  95, 
     86, 105, 115, 117,  97, 108, 
    105, 122, 101,  67, 111, 110, 
    115, 116,  97, 110, 116,  66, 
    117, 102, 102, 101, 114,   0, 
    118, 105, 101, 119,  80, 114, 
    111, 106, 101,  99, 116, 105, 
    111, 110,   0, 102, 108, 111, 
     97, 116,  52, 120,  52,   0, 
      2,   0,   3,   0,   4,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  19,   2,   0,   0, 
    109, 111, 100, 101, 108,  84, 
    111,  87, 111, 114, 108, 100, 
      0,  99, 111, 108, 111, 114, 
      0, 102, 108, 111,  97, 116, 
     52,   0, 171, 171,   1,   0, 
      3,   0,   1,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     83,   2,   0,   0, 104,  97, 
    105, 114,  77, 105, 110,   0, 
    105, 110, 116,   0,   0,   0, 
      2,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    136,   2,   0,   0, 104,  97, 
    105, 114,  77,  97, 120,   0, 
    104,  97, 105, 114,  83, 107, 
    105, 112,   0, 104,  97, 105, 
    114,  68, 117, 109, 109, 121, 
      0, 104,  97, 105, 114,  87, 
    105, 100, 116, 104,   0, 102, 
    108, 111,  97, 116,   0, 171, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 213,   2,   0,   0, 
     97, 115, 112, 101,  99, 116, 
      0, 115,  99,  97, 108, 101, 
      0, 112, 105, 110,  73, 100, 
      0, 171,   4,   2,   0,   0, 
     28,   2,   0,   0,   0,   0, 
      0,   0,  64,   2,   0,   0, 
     28,   2,   0,   0,  64,   0, 
      0,   0,  77,   2,   0,   0, 
     92,   2,   0,   0, 128,   0, 
      0,   0, 128,   2,   0,   0, 
    140,   2,   0,   0, 144,   0, 
      0,   0, 176,   2,   0,   0, 
    140,   2,   0,   0, 148,   0, 
      0,   0, 184,   2,   0,   0, 
    140,   2,   0,   0, 152,   0, 
      0,   0, 193,   2,   0,   0, 
    140,   2,   0,   0, 156,   0, 
      0,   0, 203,   2,   0,   0, 
    220,   2,   0,   0, 160,   0, 
      0,   0,   0,   3,   0,   0, 
    220,   2,   0,   0, 164,   0, 
      0,   0,   7,   3,   0,   0, 
    220,   2,   0,   0, 168,   0, 
      0,   0,  13,   3,   0,   0, 
    140,   2,   0,   0, 172,   0, 
      0,   0,   5,   0,   0,   0, 
      1,   0,  44,   0,   0,   0, 
     11,   0,  20,   3,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 229,   1, 
      0,   0, 228,   3,   0,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   0,   2,   0,   0,   0, 
    240,   3,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     36,  69, 108, 101, 109, 101, 
    110, 116,   0, 171, 171, 171, 
      1,   0,   3,   0,   1,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  83,   2,   0,   0, 
    228,   3,   0,   0,   0,   0, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   0, 240,   3, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  77, 105, 
     99, 114, 111, 115, 111, 102, 
    116,  32,  40,  82,  41,  32, 
     72,  76,  83,  76,  32,  83, 
    104,  97, 100, 101, 114,  32, 
     67, 111, 109, 112, 105, 108, 
    101, 114,  32,  49,  48,  46, 
     49,   0,  73,  83,  71,  78, 
     44,   0,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   1,   1,   0,   0, 
     83,  86,  95,  86, 101, 114, 
    116, 101, 120,  73,  68,   0, 
     79,  83,  71,  78,  76,   0, 
      0,   0,   2,   0,   0,   0, 
      8,   0,   0,   0,  56,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   0,   0,   0,  68,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   1,   0,   0,   0, 
      1,  14,   0,   0,  83,  86, 
     95,  80, 111, 115, 105, 116, 
    105, 111, 110,   0,  72,  97, 
    105, 114,  73,  68,   0, 171, 
     83,  72,  69,  88, 128,   5, 
      0,   0,  80,   0,   1,   0, 
     96,   1,   0,   0, 106,   8, 
      0,   1,  89,   0,   0,   4, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   9,   0,   0,   0, 
    162,   0,   0,   4,   0, 112, 
     16,   0,   0,   0,   0,   0, 
     16,   0,   0,   0, 162,   0, 
      0,   4,   0, 112,  16,   0, 
      1,   0,   0,   0,  16,   0, 
      0,   0,  88,   8,   0,   4, 
      0, 112,  16,   0,   2,   0, 
      0,   0,  85,  85,   0,   0, 
     88,   8,   0,   4,   0, 112, 
     16,   0,   3,   0,   0,   0, 
     85,  85,   0,   0,  88,   8, 
      0,   4,   0, 112,  16,   0, 
      4,   0,   0,   0,  51,  51, 
      0,   0,  96,   0,   0,   4, 
     18,  16,  16,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
    103,   0,   0,   4, 242,  32, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   2,   6,   0,   0,   0, 
     78,   0,   0,   9,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,  16,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   3,   0,   0,   0, 
    167,   0,   0, 139,   2, 131, 
      0, 128, 131, 153,  25,   0, 
    226,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      6, 121,  16,   0,   0,   0, 
      0,   0,  76,   0,   0,   3, 
     10,   0,  16,   0,   1,   0, 
      0,   0,   6,   0,   0,   3, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   8, 
    114,   0,  16,   0,   1,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   1,   6,   0,   0,   3, 
      1,  64,   0,   0,   1,   0, 
      0,   0,  45,   0,   0, 137, 
     66,   0,   0, 128,  67,  85, 
     21,   0, 114,   0,  16,   0, 
      1,   0,   0,   0,   6,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 126,  16,   0,   2,   0, 
      0,   0,   2,   0,   0,   1, 
      6,   0,   0,   3,   1,  64, 
      0,   0,   2,   0,   0,   0, 
     45,   0,   0, 137,  66,   0, 
      0, 128,  67,  85,  21,   0, 
    114,   0,  16,   0,   1,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   3,   0,   0,   0, 
      2,   0,   0,   1,  23,   0, 
      0,   1,  54,   0,   0,   5, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  22,   6,  16,   0, 
      1,   0,   0,   0, 167,   0, 
      0, 139,   2, 131,   0, 128, 
    131, 153,  25,   0, 242,   0, 
     16,   0,   2,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   7, 242,   0, 
     16,   0,   1,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,   9,  18,   0,  16,   0, 
      3,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0, 191,  56,   0, 
      0,   7,  98,   0,  16,   0, 
      3,   0,   0,   0,  86,   7, 
     16,   0,   1,   0,   0,   0, 
      6,   1,  16,   0,   2,   0, 
      0,   0,   0,   0,   0,   7, 
     34,   0,  16,   0,   3,   0, 
      0,   0,  42,   0,  16,   0, 
      3,   0,   0,   0,  26,   0, 
     16,   0,   3,   0,   0,   0, 
     50,   0,   0,   9,  34,   0, 
     16,   0,   3,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,  42,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   3,   0,   0,   0, 
     56,   0,   0,   7, 210,   0, 
     16,   0,   3,   0,   0,   0, 
     86,  11,  16,   0,   1,   0, 
      0,   0,   6,   0,  16,   0, 
      3,   0,   0,   0,  56,   0, 
      0,   7, 114,   0,  16,   0, 
      4,   0,   0,   0, 134,   3, 
     16,   0,   1,   0,   0,   0, 
     38,   8,  16,   0,   2,   0, 
      0,   0,  50,   0,   0,  10, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      2,   0,   0,   0,  42,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16, 128,  65,   0, 
      0,   0,   4,   0,   0,   0, 
     50,   0,   0,   9,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  58,   0,  16,   0, 
      2,   0,   0,   0,  10,   0, 
     16,   0,   3,   0,   0,   0, 
     50,   0,   0,   9,  18,   0, 
     16,   0,   5,   0,   0,   0, 
     10,   0,  16,   0,   2,   0, 
      0,   0,  26,   0,  16,   0, 
      3,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   2,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16, 128,  65,   0,   0,   0, 
      4,   0,   0,   0,  50,   0, 
      0,   9,  18,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
     58,   0,  16,   0,   2,   0, 
      0,   0,  42,   0,  16,   0, 
      3,   0,   0,   0,  50,   0, 
      0,   9,  34,   0,  16,   0, 
      5,   0,   0,   0,  26,   0, 
     16,   0,   2,   0,   0,   0, 
     26,   0,  16,   0,   3,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10,  18,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16, 128,  65,   0,   0,   0, 
      2,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
     42,   0,  16,   0,   4,   0, 
      0,   0,  50,   0,   0,   9, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  58,   0, 
     16,   0,   2,   0,   0,   0, 
     58,   0,  16,   0,   3,   0, 
      0,   0,  50,   0,   0,   9, 
     66,   0,  16,   0,   5,   0, 
      0,   0,  42,   0,  16,   0, 
      2,   0,   0,   0,  26,   0, 
     16,   0,   3,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    226,   0,  16,   0,   0,   0, 
      0,   0,   6, 128,  32,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,   6,   9,  16,   0, 
      5,   0,   0,   0,  86,  14, 
     16,   0,   0,   0,   0,   0, 
     56,   0,   0,   8, 114,   0, 
     16,   0,   1,   0,   0,   0, 
    166,  10,  16,   0,   0,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,   5,   0, 
      0,   0,  50,   0,   0,  10, 
    114,   0,  16,   0,   1,   0, 
      0,   0,  86,   5,  16,   0, 
      0,   0,   0,   0,  70, 130, 
     32,   0,   0,   0,   0,   0, 
      4,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 226,   0, 
     16,   0,   0,   0,   0,   0, 
    246,  15,  16,   0,   0,   0, 
      0,   0,   6, 137,  32,   0, 
      0,   0,   0,   0,   6,   0, 
      0,   0,   6,   9,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   8, 226,   0,  16,   0, 
      0,   0,   0,   0,  86,  14, 
     16,   0,   0,   0,   0,   0, 
      6, 137,  32,   0,   0,   0, 
      0,   0,   7,   0,   0,   0, 
     56,   0,   0,   8, 242,   0, 
     16,   0,   1,   0,   0,   0, 
    166,  10,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  86,   5,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 242,   0, 
     16,   0,   1,   0,   0,   0, 
    246,  15,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   8, 242,  32,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
     45,   0,   0, 137,  66,   0, 
      0, 128, 195, 204,  12,   0, 
     18,   0,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   4,   0,   0,   0, 
     54,   0,   0,   5,  18,  32, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  62,   0,   0,   1, 
     83,  84,  65,  84, 148,   0, 
      0,   0,  43,   0,   0,   0, 
      6,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
     25,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   5,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
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
