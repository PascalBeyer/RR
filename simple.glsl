#define v2 vec2
#define V2 vec2

#define v3 vec3
#define V3 vec3

#define v4 vec4
#define V4 vec4

#define v4i ivec4
#define V4i ivec4

#define f32 float
#define u32 int
#define i32 int

#ifdef VertexCode // Vertex Code

in v2 vertP;
in v2 vertUV;

out v2 fragCoord;
void main(void)
{
   
   fragCoord = vertUV;
   
   gl_Position = V4(vertP, -1, 1);
   
}

#endif

#ifdef FragmentCode

smooth in vec2 fragCoord;
uniform sampler2D textureSampler;

out vec4 resultColor;

void main(void)
{
   resultColor = texture(textureSampler, fragCoord);
}

#endif