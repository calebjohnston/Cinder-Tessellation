#version 400 core

uniform vec3 LightPosition;
uniform vec3 DiffuseMaterial;
uniform vec3 AmbientMaterial;

// Modify the depth test logic by using the following:
//   layout (depth_<condition>) out float gl_FragDepth;
// where <condition> can be: any, greater, less, or unchanged.

// Control draw buffer output declaration using a layout directive as follows:
//   layout(location = 2) out vec3 diffuseColor;
// this would write the diffuseColor value to the 3rd output color buffer specified.

in vec3 gFacetNormal;
in vec3 gTriDistance;
in vec3 gPatchDistance;
in float gPrimitive;

out vec4 FragColor;

// built-in inputs for this stage:
//   in vec4 gl_FragCoord;
//   in bool gl_FrontFacing;
//   in vec2 gl_PointCoord;
//   in int gl_SampleID;			// should elect to use multisampling instead
//   in vec2 gl_SamplePosition;		// should elect to use multisampling instead
//   in int gl_SampleMaskIn[];		// should elect to use multisampling instead
//   in float gl_ClipDistance[];
//   in int gl_PrimitiveID;			// used for vertex rendering
//   in int gl_Layer;				// Requires GL 4.3
//   in int gl_ViewportIndex;		// Requires GL 4.3

// built-in outputs for this stage:
//   out float gl_FragDepth;
//   out int gl_SampleMask[];

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2 * d * d);
    return d;
}

void main()
{
	// compute phong shading per face
    vec3 N = normalize(gFacetNormal);
    vec3 L = LightPosition;
    float df = abs(dot(N, L));
    vec3 color = AmbientMaterial + df * DiffuseMaterial;

	// highlight wireframe for patches and subdivisions
    float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
    float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
    color = amplify(d1, 50, -0.5) * amplify(d2, 100, -0.5) * color;

    FragColor = vec4(color, 1.0);
}