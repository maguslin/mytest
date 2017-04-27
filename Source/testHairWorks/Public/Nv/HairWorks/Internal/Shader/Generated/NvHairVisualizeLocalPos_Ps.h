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
// SV_Position              0   xyzw        0      POS   float       
// HairID                   0   x           1     NONE     int   x   
// SV_PrimitiveID           0    y          1   PRIMID    uint    y  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Target                0   xyzw        0   TARGET   float   xyzw
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer CB0[10], immediateIndexed
dcl_input_ps constant v1.x
dcl_input_ps_sgv constant v1.y, primitive_id
dcl_output o0.xyzw
dcl_temps 2
ige r0.x, v1.x, cb0[9].x
ige r0.y, cb0[9].y, v1.x
ilt r0.z, l(0), cb0[9].z
and r0.w, v1.x, l(0x80000000)
imax r1.x, v1.x, -v1.x
imax r1.y, cb0[9].z, -cb0[9].z
udiv null, r1.x, r1.x, r1.y
ineg r1.y, r1.x
movc r0.w, r0.w, r1.y, r1.x
ine r0.w, r0.w, l(0)
and r0.z, r0.w, r0.z
not r0.z, r0.z
and r0.y, r0.z, r0.y
and r0.x, r0.y, r0.x
discard_z r0.x
and r0.x, v1.y, l(1)
switch r0.x
  case l(0)
  mov r0.xy, l(1.000000,0,0,0)
  break 
  case l(1)
  mov r0.xy, l(0,1.000000,0,0)
  break 
endswitch 
mov o0.xy, r0.xyxx
mov o0.zw, l(0,0,1.000000,1.000000)
ret 
// Approximately 27 instruction slots used
#endif

const BYTE g_ps_main[] =
{
     68,  88,  66,  67, 123,  91, 
    199, 193, 128, 137,  20,  46, 
    191,   5, 182, 182, 173, 214, 
    191, 204,   1,   0,   0,   0, 
    252,   6,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    236,   2,   0,   0, 104,   3, 
      0,   0, 156,   3,   0,   0, 
     96,   6,   0,   0,  82,  68, 
     69,  70, 176,   2,   0,   0, 
      1,   0,   0,   0, 104,   0, 
      0,   0,   1,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    255, 255,   0,   1,   0,   0, 
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
     71,  78, 116,   0,   0,   0, 
      3,   0,   0,   0,   8,   0, 
      0,   0,  80,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0,  92,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      1,   0,   0,   0,   1,   1, 
      0,   0,  99,   0,   0,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   2,   2, 
      0,   0,  83,  86,  95,  80, 
    111, 115, 105, 116, 105, 111, 
    110,   0,  72,  97, 105, 114, 
     73,  68,   0,  83,  86,  95, 
     80, 114, 105, 109, 105, 116, 
    105, 118, 101,  73,  68,   0, 
    171, 171,  79,  83,  71,  78, 
     44,   0,   0,   0,   1,   0, 
      0,   0,   8,   0,   0,   0, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,  15,   0,   0,   0, 
     83,  86,  95,  84,  97, 114, 
    103, 101, 116,   0, 171, 171, 
     83,  72,  69,  88, 188,   2, 
      0,   0,  80,   0,   0,   0, 
    175,   0,   0,   0, 106,   8, 
      0,   1,  89,   0,   0,   4, 
     70, 142,  32,   0,   0,   0, 
      0,   0,  10,   0,   0,   0, 
     98,   8,   0,   3,  18,  16, 
     16,   0,   1,   0,   0,   0, 
     99,   8,   0,   4,  34,  16, 
     16,   0,   1,   0,   0,   0, 
      7,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      0,   0,   0,   0, 104,   0, 
      0,   2,   2,   0,   0,   0, 
     33,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     10,  16,  16,   0,   1,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,   9,   0, 
      0,   0,  33,   0,   0,   8, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  26, 128,  32,   0, 
      0,   0,   0,   0,   9,   0, 
      0,   0,  10,  16,  16,   0, 
      1,   0,   0,   0,  34,   0, 
      0,   8,  66,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     42, 128,  32,   0,   0,   0, 
      0,   0,   9,   0,   0,   0, 
      1,   0,   0,   7, 130,   0, 
     16,   0,   0,   0,   0,   0, 
     10,  16,  16,   0,   1,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0, 128,  36,   0, 
      0,   8,  18,   0,  16,   0, 
      1,   0,   0,   0,  10,  16, 
     16,   0,   1,   0,   0,   0, 
     10,  16,  16, 128,  65,   0, 
      0,   0,   1,   0,   0,   0, 
     36,   0,   0,  10,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     42, 128,  32,   0,   0,   0, 
      0,   0,   9,   0,   0,   0, 
     42, 128,  32, 128,  65,   0, 
      0,   0,   0,   0,   0,   0, 
      9,   0,   0,   0,  78,   0, 
      0,   8,   0, 208,   0,   0, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
     40,   0,   0,   5,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  55,   0,   0,   9, 
    130,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  39,   0,   0,   7, 
    130,   0,  16,   0,   0,   0, 
      0,   0,  58,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   7,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     58,   0,  16,   0,   0,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  59,   0, 
      0,   5,  66,   0,  16,   0, 
      0,   0,   0,   0,  42,   0, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   7,  34,   0, 
     16,   0,   0,   0,   0,   0, 
     42,   0,  16,   0,   0,   0, 
      0,   0,  26,   0,  16,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   7,  18,   0,  16,   0, 
      0,   0,   0,   0,  26,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  13,   0,   0,   3, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  26,  16,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     76,   0,   0,   3,  10,   0, 
     16,   0,   0,   0,   0,   0, 
      6,   0,   0,   3,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     54,   0,   0,   8,  50,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
    128,  63,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
      6,   0,   0,   3,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     54,   0,   0,   8,  50,   0, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0, 128,  63, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   1, 
     23,   0,   0,   1,  54,   0, 
      0,   5,  50,  32,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     54,   0,   0,   8, 194,  32, 
     16,   0,   0,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 128,  63,   0,   0, 
    128,  63,  62,   0,   0,   1, 
     83,  84,  65,  84, 148,   0, 
      0,   0,  27,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,   7,   0,   0,   0, 
      3,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   0,   0,   0,   1,   0, 
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
