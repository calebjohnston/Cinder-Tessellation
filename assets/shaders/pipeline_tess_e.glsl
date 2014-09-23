#version 400 core

uniform mat4 Projection;
uniform mat4 Modelview;

// the first layout parameter indicates what primitives must be generated
// the second layout parameter indicates what spacing will be used for tessellation invocations
// the third layout parameter indicates the winding order (and thus, the back-face culling rule)
layout(triangles, equal_spacing, cw) in;

// the number of entries in the following array is set by the number of vertices in the patch
in vec3 tcPosition[];
in vec3 tcTangent[];
in vec3 tcBitangent[];
in vec3 tcNormal[];
in vec2 tcTexcoord[];

out vec4 tePosition;
out vec4 teTangent;
out vec4 teBitangent;
out vec4 teNormal;
out vec2 teTexcoord;

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
    tePosition = vec4(normalize(p0 + p1 + p2), 1);
	
	vec3 t0 = gl_TessCoord.x * tcTangent[0];
	vec3 t1 = gl_TessCoord.y * tcTangent[1];
	vec3 t2 = gl_TessCoord.z * tcTangent[2];
	teTangent = Projection * Modelview * vec4(normalize(t0 + t1 + t2), 0);
	
	vec3 b0 = gl_TessCoord.x * tcBitangent[0];
	vec3 b1 = gl_TessCoord.y * tcBitangent[1];
	vec3 b2 = gl_TessCoord.z * tcBitangent[2];
	teBitangent = Projection * Modelview * vec4(normalize(b0 + b1 + b2), 0);
	
	vec3 n0 = gl_TessCoord.x * tcNormal[0];
	vec3 n1 = gl_TessCoord.y * tcNormal[1];
	vec3 n2 = gl_TessCoord.z * tcNormal[2];
	teNormal = Projection * Modelview * vec4(normalize(n0 + n1 + n2), 0);
	
	vec2 tc0 = gl_TessCoord.x * tcTexcoord[0];
	vec2 tc1 = gl_TessCoord.y * tcTexcoord[1];
	vec2 tc2 = gl_TessCoord.z * tcTexcoord[2];
	teTexcoord = normalize(tc0 + tc1 + tc2);
	
    gl_Position = Projection * Modelview * tePosition;
}	