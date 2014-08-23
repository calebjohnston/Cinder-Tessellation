#version 400 core

in vec4 Position;
out vec3 vPosition;

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
}