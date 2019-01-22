
//Header Code

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


// these are all the defines in this file:
// VertexCode
// FragmentCode
// ShadowMapping
// Phong
// Textured
// Animated
// ZBias
// MultiTextured

#ifdef VertexCode // Vertex Code

// uniforms
uniform mat4x4 projection;
uniform mat4x4 cameraTransform;
uniform mat4x4 objectTransform;
uniform v4 scaleColor;

#ifdef ShadowMapping
uniform mat4x4 shadowTransform;
#endif

#ifdef Phong
uniform v3 cameraPos;
uniform v3 lightPos;

uniform f32 specularExponent;
uniform v3 ka;
uniform v3 kd;
uniform v3 ks;
#endif

#ifdef Animated
const int Max_Num_Bones = 50;
uniform mat4x4 boneStates[Max_Num_Bones];
#endif

// in
in vec3 vertP;
in vec4 vertC;

#ifdef MultiTextured
in u32 textureIndex;
in v2 vertUV;
#endif

#ifdef Textured
in v2 vertUV;
#endif

#ifdef Phong
in vec3 vertN;
#endif


#ifdef Animated
in v4i boneIndices;
in v4  boneWeights;
#endif

out VS_OUT{
   // out
#ifdef MultiTextured
   flat out u32 fragIndex;
   smooth out v2 fragCoord;
#endif
   
#ifdef Textured
   smooth out v2 fragCoord;
#endif
   
#ifdef ShadowMapping
   smooth out vec4 shadowCoord;
#endif
   
#ifdef Phong
   smooth out v3 ambient;
   smooth out v3 diffuse;
   smooth out v3 specular;
#endif
   
   smooth out vec4 fragColor;
} vs_out;

void main(void)
{
   //pass through
   vs_out.fragColor = vertC * scaleColor;
   
   v4 inputVertex = vec4(vertP, 1);
   
#ifdef Textured
   vs_out.fragCoord = vertUV;
#endif
   
#ifdef Animated
   v3 animatedVertex = V3(0, 0, 0);
   
   v3 valx = (boneStates[boneIndices.x] * inputVertex).xyz;
   v3 valy = (boneStates[boneIndices.y] * inputVertex).xyz; 
   v3 valz = (boneStates[boneIndices.z] * inputVertex).xyz;
   v3 valw = (boneStates[boneIndices.w] * inputVertex).xyz;
   
   animatedVertex = (boneWeights.x * valx) + (boneWeights.y * valy) + (boneWeights.z * valz) + (boneWeights.w * valw);
   inputVertex = V4(animatedVertex, 1.0f);
#endif
   
   vec4 vertexInWorldSpace  = objectTransform * inputVertex;
   vec4 vertexInCameraSpace = cameraTransform * vertexInWorldSpace;
   gl_Position = projection * vertexInCameraSpace;
   
#ifdef ZBias
   gl_Position = gl_Position / gl_Position.w;
   gl_Position.z = -1.0f; // what should this value be?
#endif
   
   
#ifdef ShadowMapping
   vs_out.shadowCoord = shadowTransform * vertexInWorldSpace;
#endif
   
#ifdef Phong
   // simple ambient light ka
   vs_out.ambient = ka;
   
   // simple diffuse light
   vec3 point = vertexInWorldSpace.xyz;
   vec4 transformedNormal = objectTransform * vec4(vertN, 0);
   vec3 normal = normalize(transformedNormal.xyz); // we neeed this, as objectTransform scales.
   
   vec3 lightDirection = normalize(lightPos - point);
   vs_out.diffuse = max(dot(lightDirection, normal), 0) * kd;
   
   
   // simple specular light
   v3 incomingLightDir = -lightDirection;
   v3 reflected = reflect(incomingLightDir, normal); 
   
   // we can just pass this in
   vec3 viewing = normalize(cameraPos - point);
   float specularBase = max(dot(viewing, reflected),0);
   vs_out.specular = pow(specularBase, specularExponent) * ks;
#endif
   
#ifdef MultiTextured
   vs_out.fragCoord = vertUV;
   vs_out.fragIndex = textureIndex;
#endif
   
}
#endif

#ifdef GeometryCode

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
#ifdef MultiTextured
   flat   in u32 fragIndex;
   smooth in v2 fragCoord;
#endif
   
#ifdef Textured
   smooth in v2 fragCoord;
#endif
   
#ifdef ShadowMapping
   smooth in vec4 shadowCoord;
#endif
   
#ifdef Phong
   smooth in v3 ambient;
   smooth in v3 diffuse;
   smooth in v3 specular;
#endif
   
   smooth in vec4 fragColor;
} gs_in[];

// out
#ifdef Phong
smooth out v3 ambient;
smooth out v3 specular;
smooth out v3 diffuse;
#endif

#ifdef ShadowMapping
smooth out vec4 shadowCoord;
#endif

#ifdef MultiTextured
flat   out u32 fragIndex;
smooth out v2  fragCoord;
#endif

#ifdef Textured
smooth out vec2 fragCoord;
#endif

smooth out vec4 fragColor;

void main()
{
   for(u32 i = 0; i < 3; i++)
   {
#ifdef Phong
      ambient   = gs_in[i].ambient;
      specular  = gs_in[i].specular;
      diffuse   = gs_in[i].diffuse;
#endif
      
#ifdef ShadowMapping
      shadowCoord = gs_in[i].shadowCoord;
#endif
      
#ifdef MultiTextured
      fragIndex = gs_in[i].fragIndex;
      fragCoord = gs_in[i].fragCoord;
#endif
      
#ifdef Textured
      fragCoord = gs_in[i].fragCoord;
#endif
      fragColor   = gs_in[i].fragColor;
      
      gl_Position = gl_in[i].gl_Position;
      
      EmitVertex();
   }
   EndPrimitive();
} 


#endif

#ifdef FragmentCode // FragmentCode

// uniform
#ifdef ShadowMapping
uniform sampler2D depthTexture;
#endif

#ifdef MultiTextured
uniform sampler2DArray textureSampler;
#endif

#ifdef Textured
uniform u32 textureIndex;
uniform sampler2DArray textureSampler;
#endif


// in
smooth in vec4 fragColor;

#ifdef Phong
smooth in v3 ambient;
smooth in v3 specular;
smooth in v3 diffuse;
#endif

#ifdef ShadowMapping
smooth in vec4 shadowCoord;
#endif

#ifdef MultiTextured
flat   in u32 fragIndex;
smooth in v2  fragCoord;
#endif

#ifdef Textured
smooth in vec2 fragCoord;
#endif

// out
out vec4 resultColor;

void main(void)
{
   f32 colorFactor = 1.0f;
   
#ifdef ShadowMapping
   float visibility = 1.0f;
   vec3 projectedShadowCoord = shadowCoord.xyz / shadowCoord.w;
   float bias = 0.001;
   if (texture2D(depthTexture, projectedShadowCoord.xy).r < projectedShadowCoord.z - bias)
   {
      visibility = 0.5f;
   }
   colorFactor *= visibility;
   
#endif
   resultColor =  vec4(colorFactor * fragColor.rgb, fragColor.a);
   
#ifdef Textured
   v3 textureCoord = V3(fragCoord, textureIndex);
   resultColor *= texture(textureSampler, textureCoord);
#endif
   
#ifdef MultiTextured
   v3 textureCoord = V3(fragCoord, fragIndex);
   resultColor *= texture(textureSampler, textureCoord);
#endif
   
#ifdef Phong
   resultColor = V4(diffuse, 1.0f) * resultColor + 0.1 * V4(ambient, 0.0f) + V4(specular, 0.0f);
#endif
   
}

#endif // fragment Code