#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// g_interactionOffset               texture    sint         buf             t2      1 
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
// VertexID                 0   x           0     NONE     int   x   
// START_VertexID           0    y          0     NONE     int    y  
// END_VertexID             0     z         0     NONE     int     z 
//
vs_5_0
dcl_globalFlags refactoringAllowed
dcl_resource_buffer (sint,sint,sint,sint) t2
dcl_input_sgv v0.x, vertex_id
dcl_output o0.x
dcl_output o0.y
dcl_output o0.z
dcl_temps 1
iadd r0.x, v0.x, l(-1)
ld_indexable(buffer)(sint,sint,sint,sint) r0.x, r0.xxxx, t2.xyzw
movc o0.y, v0.x, r0.x, l(0)
ld_indexable(buffer)(sint,sint,sint,sint) r0.x, v0.xxxx, t2.xyzw
mov o0.z, r0.x
mov o0.x, v0.x
ret 
// Approximately 7 instruction slots used
#endif

const BYTE g_vs_main[] =
{
     68,  88,  66,  67, 249,  56, 
     56,  88, 120, 172, 116,  36, 
     53,  98, 202,  64,  86, 197, 
    207, 245,   1,   0,   0,   0, 
     56,   3,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    212,   0,   0,   0,   8,   1, 
      0,   0, 136,   1,   0,   0, 
    156,   2,   0,   0,  82,  68, 
     69,  70, 152,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    254, 255,   0,   1,   0,   0, 
    112,   0,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
     92,   0,   0,   0,   2,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0, 255, 255, 
    255, 255,   2,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 103,  95, 105, 110, 
    116, 101, 114,  97,  99, 116, 
    105, 111, 110,  79, 102, 102, 
    115, 101, 116,   0,  77, 105, 
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
     79,  83,  71,  78, 120,   0, 
      0,   0,   3,   0,   0,   0, 
      8,   0,   0,   0,  80,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      1,  14,   0,   0,  89,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      2,  13,   0,   0, 104,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      4,  11,   0,   0,  86, 101, 
    114, 116, 101, 120,  73,  68, 
      0,  83,  84,  65,  82,  84, 
     95,  86, 101, 114, 116, 101, 
    120,  73,  68,   0,  69,  78, 
     68,  95,  86, 101, 114, 116, 
    101, 120,  73,  68,   0, 171, 
    171, 171,  83,  72,  69,  88, 
     12,   1,   0,   0,  80,   0, 
      1,   0,  67,   0,   0,   0, 
    106,   8,   0,   1,  88,   8, 
      0,   4,   0, 112,  16,   0, 
      2,   0,   0,   0,  51,  51, 
      0,   0,  96,   0,   0,   4, 
     18,  16,  16,   0,   0,   0, 
      0,   0,   6,   0,   0,   0, 
    101,   0,   0,   3,  18,  32, 
     16,   0,   0,   0,   0,   0, 
    101,   0,   0,   3,  34,  32, 
     16,   0,   0,   0,   0,   0, 
    101,   0,   0,   3,  66,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   1,   0, 
      0,   0,  30,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,  16,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0, 255, 255, 255, 255, 
     45,   0,   0, 137,  66,   0, 
      0, 128, 195, 204,  12,   0, 
     18,   0,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   2,   0,   0,   0, 
     55,   0,   0,   9,  34,  32, 
     16,   0,   0,   0,   0,   0, 
     10,  16,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     45,   0,   0, 137,  66,   0, 
      0, 128, 195, 204,  12,   0, 
     18,   0,  16,   0,   0,   0, 
      0,   0,   6,  16,  16,   0, 
      0,   0,   0,   0,  70, 126, 
     16,   0,   2,   0,   0,   0, 
     54,   0,   0,   5,  66,  32, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,  54,   0,   0,   5, 
     18,  32,  16,   0,   0,   0, 
      0,   0,  10,  16,  16,   0, 
      0,   0,   0,   0,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    148,   0,   0,   0,   7,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
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
      0,   0
};
