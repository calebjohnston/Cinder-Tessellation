#version 400 core

uniform mat4 Projection;
uniform mat4 Modelview;

// the first layout parameter indicates what primitives must be generated
// the second layout parameter indicates what spacing will be used for tessellation invocations
// the third layout parameter indicates the winding order (and thus, the back-face culling rule)
layout(triangles, equal_spacing, cw) in;

// the number of entries in the following array is set by the number of vertices in the patch
in vec3 tcPosition[];

out vec3 tePosition;
out vec3 tePatchDistance;

// NOTE: can optionally pass patch-specific data to the next stage using patch qualifier:
//   patch out vec4 data;

// built-in inputs for this stage:
//   in vec3 gl_TessCoord;
//   in int gl_PatchVerticesIn;
//   in int gl_PrimitiveID;

// built-in outputs for this stage:
//   out gl_PerVertex {
//      vec4 gl_Position;
//      float gl_PointSize;
//      float gl_ClipDistance[];
//   };

void main()
{
	// Regarding the gl_TessCoord input: for isolines​ and quads​, only the XY components
	// have valid values. For triangles​, all three components have valid values.
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
	
    tePatchDistance = gl_TessCoord;
    tePosition = normalize(p0 + p1 + p2);
	
    gl_Position = Projection * Modelview * vec4(tePosition, 1);
}	