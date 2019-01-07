
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

#ifdef VertexCode
// Vertex Code

// uniforms
uniform mat4x4 projection;
uniform mat4x4 cameraTransform;

uniform vec4 scaleColor;

#ifdef ShadowMapping
uniform mat4x4 shadowTransform;
#endif

uniform vec3 lightPos;
// allready transfomed for now, so we do not need a third matrix, that is the transform with out the object Transform

// in
in vec3 vertP;
in vec4 vertC;

#ifdef Textured
in vec2 vertUV;
#endif

#ifdef Textured
smooth out vec2 fragCoord;
#endif

#ifdef ShadowMapping
smooth out vec4 shadowCoord;
#endif

#ifdef Phong
in vec3 vertN;
uniform float specularExponent;
smooth out float cosinAttenuation;
smooth out float specular;
#endif

#ifdef Animated
const int Max_Num_Bones = 100;
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
   
#ifdef Animated
   v3 animatedVertex = V3(0, 0, 0);
   
   animatedVertex += boneWeights.x * (boneStates[boneIndices.x] * inputVertex).xyz;
   animatedVertex += boneWeights.y * (boneStates[boneIndices.y] * inputVertex).xyz;
   animatedVertex += boneWeights.z * (boneStates[boneIndices.z] * inputVertex).xyz;
   animatedVertex += boneWeights.w * (boneStates[boneIndices.w] * inputVertex).xyz;
   
   inputVertex = V4(animatedVertex, 1.0f);
   
#endif
   // we premul the cameraTransform by the objectTransform
   vec4 vertexInCameraSpace = cameraTransform * inputVertex;
   gl_Position = projection * vertexInCameraSpace;
   
   
#ifdef Textured
   fragCoord = vertUV; // not neccesary if I call them the same? inout is a thing
#endif
   
#ifdef ShadowMapping
#ifdef Animated // hack
   inputVertex = boneWeights[0] * boneStates[boneIndices[0]] * inputVertex;
#endif
   shadowCoord = shadowTransform * inputVertex;
#endif
   
#ifdef Phong
   // simple diffuse light
   vec3 point = vertexInCameraSpace.xyz;
   vec3 lightDirection = normalize(lightPos - point);
   
   vec4 transformedNormal = cameraTransform * vec4(vertN, 0);
   vec3 normal = normalize(transformedNormal.xyz);
   f32 interpolationC = 0.1;
   cosinAttenuation = (1-interpolationC) * max(dot(lightDirection, normal), 0) + interpolationC;
   
   
   // simple specular light
   vec3 incomingLightDir = -lightDirection;
   vec3 reflected = reflect(incomingLightDir, normal); //incomingLightDir - 2.0 * dot(incomingLightDir, normal) * normal;
   vec3 viewing = normalize(-point);
   float specularBase = max(dot(viewing, reflected),0);
   specular = pow(specularBase, specularExponent);
   specular = 0;
#endif
   
}

#else
//Fragment Code

smooth in vec4 fragColor;

#ifdef Phong
smooth in float cosinAttenuation;
smooth in float specular;
#endif

#ifdef ShadowMapping
smooth in vec4 shadowCoord;
uniform sampler2D depthTexture;
#endif

#ifdef Textured
smooth in vec2 fragCoord;
uniform sampler2D texture;
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
   
#ifdef Phong
   colorFactor *= cosinAttenuation;
#endif
   
   resultColor =  vec4(colorFactor * fragColor.rgb, fragColor.a);
   
#ifdef Textured
   resultColor *= texture2D(texture, fragCoord);
#endif
   
#ifdef Phong
   resultColor += vec4(specular);
#endif
}

#endif // fragment Code