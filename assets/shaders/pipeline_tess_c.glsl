#version 400 core

uniform int TessLevelInner;
uniform int TessLevelOuter;

// The following layout directive overrides the glPatchParameteri(GL_PATCH_VERTICES, n) command.
// Valid values are of the range: [32,GL_MAX_PATCH_VERTICES)
layout(vertices = 3) out;

// both of the following arrays will always only have as many entries as the above vertices
// declaration specifies.
in vec3 vPosition[];
out vec3 tcPosition[];

// built-in inputs for this stage:
//   in gl_PerVertex
//   {
//      vec4 gl_Position;
//      float gl_PointSize;
//      float gl_ClipDistance[];
//   } gl_in[x];
//   in int gl_PatchVerticesIn;
//   in int gl_PrimitiveID;
//   in int gl_InvocationID;
// where x = gl_PatchVerticesIn

// built-in outputs for this stage:
//   patch out float gl_TessLevelOuter[n];
//   patch out float gl_TessLevelInner[m];
//
// where for quads and isolines:
//   n = 4; m = 2
// where for triangles:
//   n = 3; m = 1

#define ID gl_InvocationID

void main()
{
    // the invocation ID is the unique identifier for this particular invocation
	tcPosition[ID] = vPosition[ID];
	
	// gl_TessLevelInner and gl_TessLevelOuter values will determine the amount of
	// tessellation applied to the patch.
	
	// For Triangles, only the first gl_TessLevelInner value is needed and the first
	// three values of the gl_TessLevelOuter are needed.
	
	// For Quads and Isolines, the first two gl_TessLevelInner values are needed and
	// the first four values of gl_TessLevelOuter are needed.
	if (ID == 0) {
        gl_TessLevelInner[0] = TessLevelInner;
        gl_TessLevelOuter[0] = TessLevelOuter;
        gl_TessLevelOuter[1] = TessLevelOuter;
        gl_TessLevelOuter[2] = TessLevelOuter;
    }
}