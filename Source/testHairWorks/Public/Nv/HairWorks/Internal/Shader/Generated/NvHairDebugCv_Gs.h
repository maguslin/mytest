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
// SV_Position              0   xyzw        0      POS   float   xyzw
// Luminance                0   x           1     NONE   float   x   
// HairID                   0   x           2     NONE     int   x   
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xyzw
// Luminance                0   x           1     NONE   float   x   
// HairID                   0   x           2     NONE     int   x   
//
gs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[11], immediateIndexed
dcl_input_siv v[1][0].xyzw, position
dcl_input v[1][1].x
dcl_input v[1][2].x
dcl_temps 2
dcl_inputprimitive point 
dcl_stream m0
dcl_outputtopology trianglestrip 
dcl_output_siv o0.xyzw, position
dcl_output o1.x
dcl_output o2.x
dcl_maxout 4
div r0.x, cb0[10].x, cb0[10].y
mul r1.xyzw, cb0[10].xxxx, l(0.000000, 1.000000, 0.000000, -1.000000)
mad r0.yzw, r0.xxxx, l(0.000000, -1.000000, 0.000000, 0.000000), r1.xxyx
add r0.yzw, r0.yyzw, v[0][0].xxyz
mov o0.xyz, r0.yzwy
mov o0.w, v[0][0].w
mov o1.x, v[0][1].x
mov o2.x, v[0][2].x
emit_stream m0
mad r0.yzw, r0.xxxx, l(0.000000, 1.000000, 0.000000, 0.000000), r1.xxyx
add r0.yzw, r0.yyzw, v[0][0].xxyz
mov o0.xyz, r0.yzwy
mov o0.w, v[0][0].w
mov o1.x, v[0][1].x
mov o2.x, v[0][2].x
emit_stream m0
mad r0.yzw, r0.xxxx, l(0.000000, -1.000000, 0.000000, 0.000000), r1.zzwz
mad r1.xyz, r0.xxxx, l(1.000000, 0.000000, 0.000000, 0.000000), r1.zwzz
add r1.xyz, r1.xyzx, v[0][0].xyzx
add r0.xyz, r0.yzwy, v[0][0].xyzx
mov o0.xyz, r0.xyzx
mov o0.w, v[0][0].w
mov o1.x, v[0][1].x
mov o2.x, v[0][2].x
emit_stream m0
mov o0.xyz, r1.xyzx
mov o0.w, v[0][0].w
mov o1.x, v[0][1].x
mov o2.x, v[0][2].x
emit_stream m0
cut_stream m0
ret 
// Approximately 32 instruction slots used
#endif

const BYTE g_gs_main[] =
{
     68,  88,  66,  67, 205, 251, 
    190, 152, 248,  80, 163,  48, 
    133, 202, 115, 125,  93,  39, 
    154, 163,   1,   0,   0,   0, 
    104,   8,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    236,   2,   0,   0, 100,   3, 
      0,   0, 232,   3,   0,   0, 
    204,   7,   0,   0,  82,  68, 
     69,  70, 176,   2,   0,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   0,   1,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
     83,  71,   0,   1,   0,   0, 
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
     71,  78, 112,   0,   0,   0, 
      3,   0,   0,   0,   8,   0, 
      0,   0,  80,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,  15, 
      0,   0,  92,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   1,   1, 
      0,   0, 102,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   1,   1, 
      0,   0,  83,  86,  95,  80, 
    111, 115, 105, 116, 105, 111, 
    110,   0,  76, 117, 109, 105, 
    110,  97, 110,  99, 101,   0, 
     72,  97, 105, 114,  73,  68, 
      0, 171, 171, 171,  79,  83, 
     71,  53, 124,   0,   0,   0, 
      3,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
     92,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
      0,   0,   0,   0, 104,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
      1,  14,   0,   0,   0,   0, 
      0,   0, 114,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      2,   0,   0,   0,   1,  14, 
      0,   0,  83,  86,  95,  80, 
    111, 115, 105, 116, 105, 111, 
    110,   0,  76, 117, 109, 105, 
    110,  97, 110,  99, 101,   0, 
     72,  97, 105, 114,  73,  68, 
      0, 171, 171, 171,  83,  72, 
     69,  88, 220,   3,   0,   0, 
     80,   0,   2,   0, 247,   0, 
      0,   0, 106,   8,   0,   1, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   0,   0,   0,   0, 
     11,   0,   0,   0,  97,   0, 
      0,   5, 242,  16,  32,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     95,   0,   0,   4,  18,  16, 
     32,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  95,   0, 
      0,   4,  18,  16,  32,   0, 
      1,   0,   0,   0,   2,   0, 
      0,   0, 104,   0,   0,   2, 
      2,   0,   0,   0,  93,   8, 
      0,   1, 143,   0,   0,   3, 
      0,   0,  17,   0,   0,   0, 
      0,   0,  92,  40,   0,   1, 
    103,   0,   0,   4, 242,  32, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      2,   0,   0,   0,  94,   0, 
      0,   2,   4,   0,   0,   0, 
     14,   0,   0,   9,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
     26, 128,  32,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
     56,   0,   0,  11, 242,   0, 
     16,   0,   1,   0,   0,   0, 
      6, 128,  32,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0, 128,  63, 
      0,   0,   0,   0,   0,   0, 
    128, 191,  50,   0,   0,  12, 
    226,   0,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 128, 191,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      6,   1,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   8, 
    226,   0,  16,   0,   0,   0, 
      0,   0,  86,  14,  16,   0, 
      0,   0,   0,   0,   6,  25, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 114,  32,  16,   0, 
      0,   0,   0,   0, 150,   7, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   6, 130,  32, 
     16,   0,   0,   0,   0,   0, 
     58,  16,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   6,  18,  32, 
     16,   0,   1,   0,   0,   0, 
     10,  16,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     54,   0,   0,   6,  18,  32, 
     16,   0,   2,   0,   0,   0, 
     10,  16,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
    117,   0,   0,   3,   0,   0, 
     17,   0,   0,   0,   0,   0, 
     50,   0,   0,  12, 226,   0, 
     16,   0,   0,   0,   0,   0, 
      6,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    128,  63,   0,   0,   0,   0, 
      0,   0,   0,   0,   6,   1, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   8, 226,   0, 
     16,   0,   0,   0,   0,   0, 
     86,  14,  16,   0,   0,   0, 
      0,   0,   6,  25,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
    114,  32,  16,   0,   0,   0, 
      0,   0, 150,   7,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   6, 130,  32,  16,   0, 
      0,   0,   0,   0,  58,  16, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   6,  18,  32,  16,   0, 
      1,   0,   0,   0,  10,  16, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   6,  18,  32,  16,   0, 
      2,   0,   0,   0,  10,  16, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0, 117,   0, 
      0,   3,   0,   0,  17,   0, 
      0,   0,   0,   0,  50,   0, 
      0,  12, 226,   0,  16,   0, 
      0,   0,   0,   0,   6,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0, 128, 191, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 166,  11,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  12, 114,   0,  16,   0, 
      1,   0,   0,   0,   6,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 230,  10,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   8, 114,   0,  16,   0, 
      1,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     70,  18,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   8, 114,   0, 
     16,   0,   0,   0,   0,   0, 
    150,   7,  16,   0,   0,   0, 
      0,   0,  70,  18,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
    114,  32,  16,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   6, 130,  32,  16,   0, 
      0,   0,   0,   0,  58,  16, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   6,  18,  32,  16,   0, 
      1,   0,   0,   0,  10,  16, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  54,   0, 
      0,   6,  18,  32,  16,   0, 
      2,   0,   0,   0,  10,  16, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0, 117,   0, 
      0,   3,   0,   0,  17,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 114,  32,  16,   0, 
      0,   0,   0,   0,  70,   2, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   6, 130,  32, 
     16,   0,   0,   0,   0,   0, 
     58,  16,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   6,  18,  32, 
     16,   0,   1,   0,   0,   0, 
     10,  16,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     54,   0,   0,   6,  18,  32, 
     16,   0,   2,   0,   0,   0, 
     10,  16,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
    117,   0,   0,   3,   0,   0, 
     17,   0,   0,   0,   0,   0, 
    118,   0,   0,   3,   0,   0, 
     17,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  83,  84, 
     65,  84, 148,   0,   0,   0, 
     32,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      6,   0,   0,   0,  10,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0
};
