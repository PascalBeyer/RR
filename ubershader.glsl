
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
// ShadowMapping
// Phong
// Textured
// Animated
// ZBias
// MultiTextured

#ifdef VertexCode
// Vertex Code

// uniforms
uniform mat4x4 projection;
uniform mat4x4 cameraTransform;

uniform vec4 scaleColor;

#ifdef ShadowMapping
uniform mat4x4 shadowTransform;
#endif

// in
in vec3 vertP;
in vec4 vertC;

#ifdef MultiTextured
in u32 textureIndex
in v2 vertUV;
flat out fragIndex;
smooth out v2 fragCoord;
#endif

#ifdef Textured
in v2 vertUV;
smooth out v2 fragCoord;
#endif

#ifdef ShadowMapping
smooth out vec4 shadowCoord;
#endif

#ifdef Phong
uniform vec3 lightPos;
// allready transfomed for now, so we do not need a third matrix, that is the transform with out the object Transform

in vec3 vertN;
uniform f32 specularExponent;
uniform v3 ka;
uniform v3 kd;
uniform v3 ks;

smooth out v3 ambient;
smooth out v3 diffuse;
smooth out v3 specular;
#endif

#ifdef Animated
const int Max_Num_Bones = 50;
uniform mat4x4 boneStates[Max_Num_Bones];
in v4i boneIndices;
in v4  boneWeights;
#endif

smooth out vec4 fragColor;

void main(void)
{
   //pass through
   fragColor = vertC * scaleColor;
   
   v4 inputVertex = vec4(vertP, 1);
   
#ifdef Textured
   fragCoord = vertUV;
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
   // we premul the cameraTransform by the objectTransform
   vec4 vertexInCameraSpace = cameraTransform * inputVertex;
   gl_Position = projection * vertexInCameraSpace;
   
#ifdef ZBias
   gl_Position.z = -2.0f; // what should this value be?
#endif
   
   
#ifdef ShadowMapping
#ifdef Animated // hack
   inputVertex = boneWeights[0] * boneStates[boneIndices[0]] * inputVertex;
#endif
   shadowCoord = shadowTransform * inputVertex;
#endif
   
#ifdef Phong
   // todo pass the object transform for this? I don't know why we work in camera space.
   // but the camera transform should be an isometry so whatevs
   
   // simple ambient light ka
   ambient = ka;
   
   // simple diffuse light
   vec3 point = vertexInCameraSpace.xyz;
   vec4 transformedNormal = cameraTransform * vec4(vertN, 0);
   vec3 normal = normalize(transformedNormal.xyz);
   
   vec3 lightDirection = normalize(lightPos - point);
   
   diffuse = max(dot(lightDirection, normal), 0) * kd;
   
   // simple specular light
   vec3 incomingLightDir = -lightDirection;
   vec3 reflected = reflect(incomingLightDir, normal); //incomingLightDir - 2.0 * dot(incomingLightDir, normal) * normal;
   vec3 viewing = normalize(-point);
   float specularBase = max(dot(viewing, reflected),0);
   specular = pow(specularBase, specularExponent) * ks;
#endif
   
#ifdef MultiTextured
   fragCoord = vertUV;
   fragIndex = textureIndex;
#endif
   
}

#else
//Fragment Code

smooth in vec4 fragColor;

#ifdef Phong
smooth in v3 ambient;
smooth in v3 specular;
smooth in v3 diffuse;
#endif

#ifdef ShadowMapping
smooth in vec4 shadowCoord;
uniform sampler2D depthTexture;
#endif

#ifdef MultiTextured
flat in u32 fragIndex;
smooth in vec2 fragCoord;
uniform sampler2DArray textureSampler;
#endif

#ifdef Textured
uniform u32 textureIndex;
smooth in vec2 fragCoord;
uniform sampler2DArray textureSampler;
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