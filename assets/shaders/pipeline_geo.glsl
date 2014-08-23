#version 400 core

uniform mat4 Modelview;
uniform mat3 NormalMatrix;

// valid input primitives: points, lines, lines_adjacency, triangles, triangles_adjacency
// valid values for invocations are: [32,MAX_GEOMETRY_SHADER_INVOCATIONS)
// the invocations value determines how many times the geometry shader is invoked.
layout(triangles, invocations = 1) in;

// valid output primitives: points, line_strip, triangle_strip
// valid values for max_vertices is: [256, MAX_GEOMETRY_OUTPUT_VERTICES)
layout(triangle_strip, max_vertices = 3) out;

in vec3 tePosition[3];
in vec3 tePatchDistance[3];

out vec3 gFacetNormal;
out vec3 gPatchDistance;
out vec3 gTriDistance;

// built-in inputs for this stage:
//   in gl_PerVertex
//   {
//      vec4 gl_Position;
//      float gl_PointSize;
//      float gl_ClipDistance[];
//   } gl_in[x];
//   in int gl_PrimitiveIDIn;
//   in int gl_InvocationID;  // Requires GLSL 4.0 or ARB_gpu_shader5
// where x = vertex count for input primitive, which are:
//   points​              1
//   lines               2
//   lines_adjacency​     4
//   triangles           3
//   triangles_adjacency​ 6

// built-in outputs for this stage:
//   out gl_PerVertex {
//      vec4 gl_Position;
//      float gl_PointSize;
//      float gl_ClipDistance[];
//   };
//   out int gl_PrimitiveID; (defaults to gl_PrimitiveIDIn)
//
// and when using layered framebuffer output:
//   out int gl_Layer;
//   out int gl_ViewportIndex; //Requires GL 4.1 or ARB_viewport_array.

void main()
{
    vec3 A = tePosition[2] - tePosition[0];
    vec3 B = tePosition[1] - tePosition[0];
    gFacetNormal = NormalMatrix * normalize(cross(A, B));
    
	// the number of times EmitVertex() is called before EndPrimitive()
	// must match the number of vertices for the output primitive type specfied above.
	
    gPatchDistance = tePatchDistance[0];
    gTriDistance = vec3(1, 0, 0);
    gl_Position = gl_in[0].gl_Position; EmitVertex();

    gPatchDistance = tePatchDistance[1];
    gTriDistance = vec3(0, 1, 0);
    gl_Position = gl_in[1].gl_Position; EmitVertex();

    gPatchDistance = tePatchDistance[2];
    gTriDistance = vec3(0, 0, 1);
    gl_Position = gl_in[2].gl_Position; EmitVertex();

    EndPrimitive();
}
