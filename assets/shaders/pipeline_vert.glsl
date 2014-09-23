#version 400 core

in vec3 Position;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 Normal;
in vec2 Texcoord;

out vec3 vPosition;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vNormal;
out vec2 vTexcoord;

// built-in inputs for this stage:
//   in int gl_VertexID;
//   in int gl_InstanceID;

// built-in outputs for this stage:
//   out gl_PerVertex
//   {
//      vec4 gl_Position;
//      float gl_PointSize;
//      float gl_ClipDistance[];
//   }

void main()
{
    vPosition = Position.xyz;
    vTangent = Tangent.xyz;
    vBitangent = Bitangent.xyz;
    vNormal = Normal.xyz;
    vTexcoord = Texcoord.xy;
}