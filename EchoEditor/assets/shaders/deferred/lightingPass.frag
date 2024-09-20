#version 460 core
#include "common.glsl"

const float PI = 3.14159265359;

in vec2 TexCoords;
out vec4 FragColor;

//-- G-BUFFER MAPS
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMetallicRoughness;
uniform sampler2D gSpecular;
uniform sampler2D ssaoTexture;

//-- GLOBAL ILUMINATION
uniform float ambientLight;
uniform float exposure; //HDR
uniform bool useSSAO;

// Shadow map
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform vec3 lightDir;

//-- IBL
uniform bool useIBL;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform float iblIntensity; // Intensidad del IBL

//-- LIGHTS
const int NR_POINT_LIGHTS = 32;
const int NR_SPOT_LIGHTS = 32;
const int NR_AREA_LIGHTS = 32;

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform AreaLight areaLights[NR_AREA_LIGHTS];

uniform int numPointLights;
uniform int numSpotLights;
uniform int numAreaLights;

uniform DirectionalLight directionalLight; // Uniforme para la luz direccional

uniform vec3 viewPos;

//-- LTC Textures
uniform sampler2D LTC1;
uniform sampler2D LTC2;

//-- Uniforms for LUT
uniform float LUT_SIZE;
uniform float LUT_SCALE;
uniform float LUT_BIAS;

// Funciones de cálculo de luz
vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
vec3 CalcSpotLight(SpotLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
vec3 CalcAreaLight(AreaLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
vec3 CalcDirLight(DirectionalLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);

// Funciones de Fresnel y GGX
vec3 fresnelSchlick(float cosTheta, vec3 F0);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);

// Funciones de Integración de Área
vec3 IntegrateEdgeVec(vec3 v1, vec3 v2);
vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided);

// Funciones para IBL
vec3 getIBLDiffuse(vec3 normal);
vec3 getIBLSpecular(vec3 normal, vec3 viewDir, float roughness);

float ShadowCalculation(vec4 fragPosLightSpace);

void main()
{      
    const float gamma = 2.2;
    
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb); // Asegúrate de normalizar las normales
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float Specular = texture(gSpecular, TexCoords).r;
    vec3 metallicRoughness = texture(gMetallicRoughness, TexCoords).rgb;

    float metallic = clamp(metallicRoughness.r, 0.0, 1.0);
    float roughness = clamp(metallicRoughness.g, 0.04, 1.0); // Evitar valores de roughness muy bajos

    float ssao = useSSAO ? texture(ssaoTexture, TexCoords).r : 1.0;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, Diffuse, metallic);

    vec3 ambient = vec3(ambientLight * Diffuse * ssao);
    vec3 lighting = ambient;

    // Calcular iluminación direccional
    if (directionalLight.isActive) {
        lighting += CalcDirLight(directionalLight, FragPos, Normal, Diffuse, Specular, metallic, roughness);
    }

    for(int i = 0; i < numPointLights; ++i)
    {
        lighting += CalcPointLight(pointLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
    }

    for(int i = 0; i < numSpotLights; ++i)
    {
        lighting += CalcSpotLight(spotLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
    }

    for(int i = 0; i < numAreaLights; ++i)
    {
        lighting += CalcAreaLight(areaLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
    }

    if (useIBL) {
        // Integración de IBL
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 diffuseIBL = getIBLDiffuse(Normal) * iblIntensity;
        vec3 specularIBL = getIBLSpecular(Normal, viewDir, roughness) * iblIntensity;
        lighting += diffuseIBL * Diffuse + specularIBL * Specular;
    }

    vec3 hdrColor = vec3(1.0) - exp(-lighting * exposure);
    hdrColor = pow(hdrColor, vec3(1.0 / gamma));
    FragColor = vec4(hdrColor, 1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005; // Adjust bias as necessary
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    return shadow;
}

vec3 CalcDirLight(DirectionalLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    vec3 radiance = light.diffuse;

    float NDF = distributionGGX(Normal, halfwayDir, roughness);
    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), mix(vec3(0.04), Diffuse, metallic));

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(Normal, lightDir), 0.0);

    // Calculate shadow
    float shadow = 1.0;
    if (light.drawShadows) {
        vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
        shadow = ShadowCalculation(fragPosLightSpace);
    }

    return (kD * Diffuse / PI + specular) * radiance * NdotL * (1.0 - shadow * light.shadowIntensity);
}


vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {

    float distance = length(light.Position - FragPos);
    
    if (distance >= light.Radius + light.LightSmoothness)
        return vec3(0.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float distanceSquared = distance * distance;
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distanceSquared);

    if (distance > light.Radius) {
        float smoothFactor = (light.Radius + light.LightSmoothness - distance) / light.LightSmoothness;
        attenuation *= smoothstep(0.0, 1.0, smoothFactor);
    }

    vec3 radiance = light.colorAmbient * attenuation;

    float NDF = distributionGGX(Normal, halfwayDir, roughness);
    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), mix(vec3(0.04), Diffuse, metallic));

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(Normal, lightDir), 0.0);
    return (kD * Diffuse / PI + specular) * radiance * NdotL;
}

vec3 CalcSpotLight(SpotLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
   float distance = length(light.Position - FragPos);

    if (distance >= light.Radius + light.LightSmoothness)
        return vec3(0.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(light.Position - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float distanceSquared = distance * distance;
    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distanceSquared);

    if (distance > light.Radius) {
        float smoothFactor = (light.Radius + light.LightSmoothness - distance) / light.LightSmoothness;
        attenuation *= smoothstep(0.0, 1.0, smoothFactor);
    }

    // Spot light specific attenuation
    float theta = dot(lightDir, normalize(-light.Direction));
    float epsilon = light.innerCutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
    attenuation *= intensity;

    vec3 radiance = light.colorAmbient * attenuation;

    float NDF = distributionGGX(Normal, halfwayDir, roughness);
    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), mix(vec3(0.04), Diffuse, metallic));

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(Normal, lightDir), 0.0);
    return (kD * Diffuse / PI + specular) * radiance * NdotL;
}

vec3 CalcAreaLight(AreaLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
    vec3 V = normalize(viewPos - FragPos);
    vec3 P = FragPos;
    float dotNV = clamp(dot(Normal, V), 0.0, 1.0);

    vec2 uv = vec2(roughness, sqrt(1.0 - dotNV));
    uv = uv * LUT_SCALE + LUT_BIAS;

    vec4 t1 = texture(LTC1, uv);
    vec4 t2 = texture(LTC2, uv);

    mat3 Minv = mat3(
        vec3(t1.x, 0, t1.y),
        vec3(0, 1, 0),
        vec3(t1.z, 0, t1.w)
    );

    vec3 diffuse = LTC_Evaluate(Normal, V, P, mat3(1), light.points, light.twoSided);
    vec3 specular = LTC_Evaluate(Normal, V, P, Minv, light.points, light.twoSided);

    specular *= Specular * t2.x + (1.0 - Specular) * t2.y;

    return light.color * light.intensity * (specular + Diffuse * diffuse);
}

vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided) {
    vec3 T1, T2;
    T1 = normalize(V - N * dot(V, N));
    T2 = cross(N, T1);

    Minv = Minv * transpose(mat3(T1, T2, N));

    vec3 L[4];
    L[0] = Minv * (points[0] - P);
    L[1] = Minv * (points[1] - P);
    L[2] = Minv * (points[2] - P);
    L[3] = Minv * (points[3] - P);

    vec3 dir = points[0] - P;
    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
    bool behind = (dot(dir, lightNormal) < 0.0);

    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    vec3 vsum = vec3(0.0);
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    float len = length(vsum);
    float z = vsum.z / len;
    if (behind)
        z = -z;

    vec2 uv = vec2(z * 0.5 + 0.5, len);
    uv = uv * LUT_SCALE + LUT_BIAS;

    float scale = texture(LTC2, uv).w;

    float sum = len * scale;
    if (!behind && !twoSided)
        sum = 0.0;

    vec3 Lo_i = vec3(sum, sum, sum);
    return Lo_i;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 IntegrateEdgeVec(vec3 v1, vec3 v2) {
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

vec3 getIBLDiffuse(vec3 normal) {
    return texture(irradianceMap, normal).rgb;
}

vec3 getIBLSpecular(vec3 normal, vec3 viewDir, float roughness) {
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 R = reflect(-viewDir, normal);
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
    return prefilteredColor * (brdf.x + brdf.y);
}



//SIN SOMBRAS
//#version 460 core
//#include "common.glsl"
//
//const float PI = 3.14159265359;
//
//in vec2 TexCoords;
//out vec4 FragColor;
//
////-- G-BUFFER MAPS
//uniform sampler2D gPosition;
//uniform sampler2D gNormal;
//uniform sampler2D gAlbedo;
//uniform sampler2D gMetallicRoughness;
//uniform sampler2D gSpecular;
//uniform sampler2D ssaoTexture;
//
////-- GLOBAL ILUMINATION
//uniform float ambientLight;
//uniform float exposure; //HDR
//uniform bool useSSAO;
//
////-- IBL
//uniform bool useIBL;
//uniform samplerCube irradianceMap;
//uniform samplerCube prefilterMap;
//uniform sampler2D brdfLUT;
//uniform float iblIntensity; // Intensidad del IBL
//
////-- LIGHTS
//const int NR_POINT_LIGHTS = 32;
//const int NR_SPOT_LIGHTS = 32;
//const int NR_AREA_LIGHTS = 32;
//
//uniform PointLight pointLights[NR_POINT_LIGHTS];
//uniform SpotLight spotLights[NR_SPOT_LIGHTS];
//uniform AreaLight areaLights[NR_AREA_LIGHTS];
//
//uniform int numPointLights;
//uniform int numSpotLights;
//uniform int numAreaLights;
//
//uniform vec3 viewPos;
//
////-- LTC Textures
//uniform sampler2D LTC1;
//uniform sampler2D LTC2;
//
////-- Uniforms for LUT
//uniform float LUT_SIZE;
//uniform float LUT_SCALE;
//uniform float LUT_BIAS;
//
//// Funciones de cálculo de luz
//vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
//vec3 CalcSpotLight(SpotLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
//vec3 CalcAreaLight(AreaLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
//
//// Funciones de Fresnel y GGX
//vec3 fresnelSchlick(float cosTheta, vec3 F0);
//float distributionGGX(vec3 N, vec3 H, float roughness);
//float geometrySchlickGGX(float NdotV, float roughness);
//float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
//
//// Funciones de Integración de Área
//vec3 IntegrateEdgeVec(vec3 v1, vec3 v2);
//vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided);
//
//// Funciones para IBL
//vec3 getIBLDiffuse(vec3 normal);
//vec3 getIBLSpecular(vec3 normal, vec3 viewDir, float roughness);
//
//void main()
//{      
//    const float gamma = 2.2;
//    
//    vec3 FragPos = texture(gPosition, TexCoords).rgb;
//    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb); // Asegúrate de normalizar las normales
//    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
//    float Specular = texture(gSpecular, TexCoords).r;
//    vec3 metallicRoughness = texture(gMetallicRoughness, TexCoords).rgb;
//
//    float metallic = clamp(metallicRoughness.r, 0.0, 1.0);
//    float roughness = clamp(metallicRoughness.g, 0.04, 1.0); // Evitar valores de roughness muy bajos
//
//    float ssao = useSSAO ? texture(ssaoTexture, TexCoords).r : 1.0;
//
//    vec3 F0 = vec3(0.04); 
//    F0 = mix(F0, Diffuse, metallic);
//
//    vec3 ambient = vec3(ambientLight * Diffuse * ssao);
//    vec3 lighting = ambient;
//
//    for(int i = 0; i < numPointLights; ++i)
//    {
//        lighting += CalcPointLight(pointLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
//    }
//
//    for(int i = 0; i < numSpotLights; ++i)
//    {
//        lighting += CalcSpotLight(spotLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
//    }
//
//    for(int i = 0; i < numAreaLights; ++i)
//    {
//        lighting += CalcAreaLight(areaLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
//    }
//
//    if (useIBL) {
//        // Integración de IBL
//        vec3 viewDir = normalize(viewPos - FragPos);
//        vec3 diffuseIBL = getIBLDiffuse(Normal) * iblIntensity;
//        vec3 specularIBL = getIBLSpecular(Normal, viewDir, roughness) * iblIntensity;
//        lighting += diffuseIBL * Diffuse + specularIBL * Specular;
//    }
//
//    vec3 hdrColor = vec3(1.0) - exp(-lighting * exposure);
//    hdrColor = pow(hdrColor, vec3(1.0 / gamma));
//    FragColor = vec4(hdrColor, 1.0);
//}
//
//vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
//
//    float distance = length(light.Position - FragPos);
//    
//    if (distance >= light.Radius + light.LightSmoothness)
//        return vec3(0.0);
//
//    vec3 viewDir = normalize(viewPos - FragPos);
//    vec3 lightDir = normalize(light.Position - FragPos);
//    vec3 halfwayDir = normalize(lightDir + viewDir);
//    float distanceSquared = distance * distance;
//    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distanceSquared);
//
//    if (distance > light.Radius) {
//        float smoothFactor = (light.Radius + light.LightSmoothness - distance) / light.LightSmoothness;
//        attenuation *= smoothstep(0.0, 1.0, smoothFactor);
//    }
//
//    vec3 radiance = light.colorAmbient * attenuation;
//
//    float NDF = distributionGGX(Normal, halfwayDir, roughness);
//    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
//    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), mix(vec3(0.04), Diffuse, metallic));
//
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(Normal, lightDir), 0.0);
//    return (kD * Diffuse / PI + specular) * radiance * NdotL;
//}
//
//vec3 CalcSpotLight(SpotLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
//   float distance = length(light.Position - FragPos);
//
//    if (distance >= light.Radius + light.LightSmoothness)
//        return vec3(0.0);
//
//    vec3 viewDir = normalize(viewPos - FragPos);
//    vec3 lightDir = normalize(light.Position - FragPos);
//    vec3 halfwayDir = normalize(lightDir + viewDir);
//    float distanceSquared = distance * distance;
//    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distanceSquared);
//
//    if (distance > light.Radius) {
//        float smoothFactor = (light.Radius + light.LightSmoothness - distance) / light.LightSmoothness;
//        attenuation *= smoothstep(0.0, 1.0, smoothFactor);
//    }
//
//    // Spot light specific attenuation
//    float theta = dot(lightDir, normalize(-light.Direction));
//    float epsilon = light.innerCutoff - light.outerCutoff;
//    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
//    attenuation *= intensity;
//
//    vec3 radiance = light.colorAmbient * attenuation;
//
//    float NDF = distributionGGX(Normal, halfwayDir, roughness);
//    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
//    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), mix(vec3(0.04), Diffuse, metallic));
//
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(Normal, lightDir), 0.0);
//    return (kD * Diffuse / PI + specular) * radiance * NdotL;
//}
//
//vec3 CalcAreaLight(AreaLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
//    vec3 V = normalize(viewPos - FragPos);
//    vec3 P = FragPos;
//    float dotNV = clamp(dot(Normal, V), 0.0, 1.0);
//
//    vec2 uv = vec2(roughness, sqrt(1.0 - dotNV));
//    uv = uv * LUT_SCALE + LUT_BIAS;
//
//    vec4 t1 = texture(LTC1, uv);
//    vec4 t2 = texture(LTC2, uv);
//
//    mat3 Minv = mat3(
//        vec3(t1.x, 0, t1.y),
//        vec3(0, 1, 0),
//        vec3(t1.z, 0, t1.w)
//    );
//
//    vec3 diffuse = LTC_Evaluate(Normal, V, P, mat3(1), light.points, light.twoSided);
//    vec3 specular = LTC_Evaluate(Normal, V, P, Minv, light.points, light.twoSided);
//
//    specular *= Specular * t2.x + (1.0 - Specular) * t2.y;
//
//    return light.color * light.intensity * (specular + Diffuse * diffuse);
//}
//
//vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided) {
//    vec3 T1, T2;
//    T1 = normalize(V - N * dot(V, N));
//    T2 = cross(N, T1);
//
//    Minv = Minv * transpose(mat3(T1, T2, N));
//
//    vec3 L[4];
//    L[0] = Minv * (points[0] - P);
//    L[1] = Minv * (points[1] - P);
//    L[2] = Minv * (points[2] - P);
//    L[3] = Minv * (points[3] - P);
//
//    vec3 dir = points[0] - P;
//    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
//    bool behind = (dot(dir, lightNormal) < 0.0);
//
//    L[0] = normalize(L[0]);
//    L[1] = normalize(L[1]);
//    L[2] = normalize(L[2]);
//    L[3] = normalize(L[3]);
//
//    vec3 vsum = vec3(0.0);
//    vsum += IntegrateEdgeVec(L[0], L[1]);
//    vsum += IntegrateEdgeVec(L[1], L[2]);
//    vsum += IntegrateEdgeVec(L[2], L[3]);
//    vsum += IntegrateEdgeVec(L[3], L[0]);
//
//    float len = length(vsum);
//    float z = vsum.z / len;
//    if (behind)
//        z = -z;
//
//    vec2 uv = vec2(z * 0.5 + 0.5, len);
//    uv = uv * LUT_SCALE + LUT_BIAS;
//
//    float scale = texture(LTC2, uv).w;
//
//    float sum = len * scale;
//    if (!behind && !twoSided)
//        sum = 0.0;
//
//    vec3 Lo_i = vec3(sum, sum, sum);
//    return Lo_i;
//}
//
//vec3 fresnelSchlick(float cosTheta, vec3 F0) {
//    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}
//
//float distributionGGX(vec3 N, vec3 H, float roughness) {
//    float a = roughness * roughness;
//    float a2 = a * a;
//    float NdotH = max(dot(N, H), 0.0);
//    float NdotH2 = NdotH * NdotH;
//
//    float num = a2;
//    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//    denom = PI * denom * denom;
//
//    return num / denom;
//}
//
//float geometrySchlickGGX(float NdotV, float roughness) {
//    float r = (roughness + 1.0);
//    float k = (r * r) / 8.0;
//
//    float num = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//
//    return num / denom;
//}
//
//float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
//    float ggx2 = geometrySchlickGGX(NdotV, roughness);
//    float ggx1 = geometrySchlickGGX(NdotL, roughness);
//
//    return ggx1 * ggx2;
//}
//
//vec3 IntegrateEdgeVec(vec3 v1, vec3 v2) {
//    float x = dot(v1, v2);
//    float y = abs(x);
//
//    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
//    float b = 3.4175940 + (4.1616724 + y) * y;
//    float v = a / b;
//
//    float theta_sintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;
//
//    return cross(v1, v2) * theta_sintheta;
//}
//
//vec3 getIBLDiffuse(vec3 normal) {
//    return texture(irradianceMap, normal).rgb;
//}
//
//vec3 getIBLSpecular(vec3 normal, vec3 viewDir, float roughness) {
//    const float MAX_REFLECTION_LOD = 4.0;
//    vec3 R = reflect(-viewDir, normal);
//    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
//    vec2 brdf = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
//    return prefilteredColor * (brdf.x + brdf.y);
//}





//--SIN IBL
//#version 460 core
//#include "common.glsl"
//
//const float PI = 3.14159265359;
//
//in vec2 TexCoords;
//out vec4 FragColor;
//
////-- G-BUFFER MAPS
//uniform sampler2D gPosition;
//uniform sampler2D gNormal;
//uniform sampler2D gAlbedo;
//uniform sampler2D gMetallicRoughness;
//uniform sampler2D gSpecular;
//uniform sampler2D ssaoTexture;
//
////-- GLOBAL ILUMINATION
//uniform float ambientLight;
//uniform float exposure; //HDR
//uniform bool useSSAO;
//
////-- LIGHTS
//const int NR_POINT_LIGHTS = 32;
//const int NR_SPOT_LIGHTS = 32;
//const int NR_AREA_LIGHTS = 32;
//
//uniform PointLight pointLights[NR_POINT_LIGHTS];
//uniform SpotLight spotLights[NR_SPOT_LIGHTS];
//uniform AreaLight areaLights[NR_AREA_LIGHTS];
//
//uniform int numPointLights;
//uniform int numSpotLights;
//uniform int numAreaLights;
//
//
//uniform vec3 viewPos;
//
////-- LTC Textures
//uniform sampler2D LTC1;
//uniform sampler2D LTC2;
//
////-- Uniforms for LUT
//uniform float LUT_SIZE;
//uniform float LUT_SCALE;
//uniform float LUT_BIAS;
//
//// Funciones de cálculo de luz
//vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
//vec3 CalcSpotLight(SpotLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
//vec3 CalcAreaLight(AreaLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
//
//// Funciones de Fresnel y GGX
//vec3 fresnelSchlick(float cosTheta, vec3 F0);
//float distributionGGX(vec3 N, vec3 H, float roughness);
//float geometrySchlickGGX(float NdotV, float roughness);
//float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
//
//// Funciones de Integración de Área
//vec3 IntegrateEdgeVec(vec3 v1, vec3 v2);
//vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided);
//
//void main()
//{      
//    const float gamma = 2.2;
//    
//    vec3 FragPos = texture(gPosition, TexCoords).rgb;
//    vec3 Normal = normalize(texture(gNormal, TexCoords).rgb); // Asegúrate de normalizar las normales
//    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
//    float Specular = texture(gSpecular, TexCoords).r;
//    vec3 metallicRoughness = texture(gMetallicRoughness, TexCoords).rgb;
//
//    float metallic = clamp(metallicRoughness.r, 0.0, 1.0);
//    float roughness = clamp(metallicRoughness.g, 0.04, 1.0); // Evitar valores de roughness muy bajos
//
//    float ssao = useSSAO ? texture(ssaoTexture, TexCoords).r : 1.0;
//
//    vec3 F0 = vec3(0.04); 
//    F0 = mix(F0, Diffuse, metallic);
//
//    vec3 ambient = vec3(ambientLight * Diffuse * ssao);
//    vec3 lighting = ambient;
//
//    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
//    {
//        lighting += CalcPointLight(pointLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
//    }
//
//    for(int i = 0; i < NR_SPOT_LIGHTS; ++i)
//    {
//        lighting += CalcSpotLight(spotLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
//    }
//
//    for(int i = 0; i < numAreaLights; ++i)
//    {
//        lighting += CalcAreaLight(areaLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
//    }
//
//    vec3 hdrColor = vec3(1.0) - exp(-lighting * exposure);
//    hdrColor = pow(hdrColor, vec3(1.0 / gamma));
//    FragColor = vec4(hdrColor, 1.0);
//}
//
//vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
//
//    float distance = length(light.Position - FragPos);
//    
//    if (distance >= light.Radius + light.LightSmoothness)
//        return vec3(0.0);
//
//    vec3 viewDir = normalize(viewPos - FragPos);
//    vec3 lightDir = normalize(light.Position - FragPos);
//    vec3 halfwayDir = normalize(lightDir + viewDir);
//    float distanceSquared = distance * distance;
//    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distanceSquared);
//
//    if (distance > light.Radius) {
//        float smoothFactor = (light.Radius + light.LightSmoothness - distance) / light.LightSmoothness;
//        attenuation *= smoothstep(0.0, 1.0, smoothFactor);
//    }
//
//    vec3 radiance = light.colorAmbient * attenuation;
//
//    float NDF = distributionGGX(Normal, halfwayDir, roughness);
//    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
//    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), mix(vec3(0.04), Diffuse, metallic));
//
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(Normal, lightDir), 0.0);
//    return (kD * Diffuse / PI + specular) * radiance * NdotL;
//}
//
//vec3 CalcSpotLight(SpotLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
//   float distance = length(light.Position - FragPos);
//
//    if (distance >= light.Radius + light.LightSmoothness)
//        return vec3(0.0);
//
//    vec3 viewDir = normalize(viewPos - FragPos);
//    vec3 lightDir = normalize(light.Position - FragPos);
//    vec3 halfwayDir = normalize(lightDir + viewDir);
//    float distanceSquared = distance * distance;
//    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distanceSquared);
//
//    if (distance > light.Radius) {
//        float smoothFactor = (light.Radius + light.LightSmoothness - distance) / light.LightSmoothness;
//        attenuation *= smoothstep(0.0, 1.0, smoothFactor);
//    }
//
//    // Spot light specific attenuation
//    float theta = dot(lightDir, normalize(-light.Direction));
//    float epsilon = light.innerCutoff - light.outerCutoff;
//    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
//    attenuation *= intensity;
//
//    vec3 radiance = light.colorAmbient * attenuation;
//
//    float NDF = distributionGGX(Normal, halfwayDir, roughness);
//    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
//    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), mix(vec3(0.04), Diffuse, metallic));
//
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(Normal, lightDir), 0.0);
//    return (kD * Diffuse / PI + specular) * radiance * NdotL;
//}
//
//vec3 CalcAreaLight(AreaLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
//    vec3 V = normalize(viewPos - FragPos);
//    vec3 P = FragPos;
//    float dotNV = clamp(dot(Normal, V), 0.0, 1.0);
//
//    vec2 uv = vec2(roughness, sqrt(1.0 - dotNV));
//    uv = uv * LUT_SCALE + LUT_BIAS;
//
//    vec4 t1 = texture(LTC1, uv);
//    vec4 t2 = texture(LTC2, uv);
//
//    mat3 Minv = mat3(
//        vec3(t1.x, 0, t1.y),
//        vec3(0, 1, 0),
//        vec3(t1.z, 0, t1.w)
//    );
//
//    vec3 diffuse = LTC_Evaluate(Normal, V, P, mat3(1), light.points, light.twoSided);
//    vec3 specular = LTC_Evaluate(Normal, V, P, Minv, light.points, light.twoSided);
//
//    specular *= Specular * t2.x + (1.0 - Specular) * t2.y;
//
//    return light.color * light.intensity * (specular + Diffuse * diffuse);
//}
//
//vec3 LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], bool twoSided) {
//    vec3 T1, T2;
//    T1 = normalize(V - N * dot(V, N));
//    T2 = cross(N, T1);
//
//    Minv = Minv * transpose(mat3(T1, T2, N));
//
//    vec3 L[4];
//    L[0] = Minv * (points[0] - P);
//    L[1] = Minv * (points[1] - P);
//    L[2] = Minv * (points[2] - P);
//    L[3] = Minv * (points[3] - P);
//
//    vec3 dir = points[0] - P;
//    vec3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
//    bool behind = (dot(dir, lightNormal) < 0.0);
//
//    L[0] = normalize(L[0]);
//    L[1] = normalize(L[1]);
//    L[2] = normalize(L[2]);
//    L[3] = normalize(L[3]);
//
//    vec3 vsum = vec3(0.0);
//    vsum += IntegrateEdgeVec(L[0], L[1]);
//    vsum += IntegrateEdgeVec(L[1], L[2]);
//    vsum += IntegrateEdgeVec(L[2], L[3]);
//    vsum += IntegrateEdgeVec(L[3], L[0]);
//
//    float len = length(vsum);
//    float z = vsum.z / len;
//    if (behind)
//        z = -z;
//
//    vec2 uv = vec2(z * 0.5 + 0.5, len);
//    uv = uv * LUT_SCALE + LUT_BIAS;
//
//    float scale = texture(LTC2, uv).w;
//
//    float sum = len * scale;
//    if (!behind && !twoSided)
//        sum = 0.0;
//
//    vec3 Lo_i = vec3(sum, sum, sum);
//    return Lo_i;
//}
//
//vec3 fresnelSchlick(float cosTheta, vec3 F0) {
//    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}
//
//float distributionGGX(vec3 N, vec3 H, float roughness) {
//    float a = roughness * roughness;
//    float a2 = a * a;
//    float NdotH = max(dot(N, H), 0.0);
//    float NdotH2 = NdotH * NdotH;
//
//    float num = a2;
//    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//    denom = PI * denom * denom;
//
//    return num / denom;
//}
//
//float geometrySchlickGGX(float NdotV, float roughness) {
//    float r = (roughness + 1.0);
//    float k = (r * r) / 8.0;
//
//    float num = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//
//    return num / denom;
//}
//
//float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
//    float ggx2 = geometrySchlickGGX(NdotV, roughness);
//    float ggx1 = geometrySchlickGGX(NdotL, roughness);
//
//    return ggx1 * ggx2;
//}
//
//vec3 IntegrateEdgeVec(vec3 v1, vec3 v2) {
//    float x = dot(v1, v2);
//    float y = abs(x);
//
//    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
//    float b = 3.4175940 + (4.1616724 + y) * y;
//    float v = a / b;
//
//    float theta_sintheta = (x > 0.0) ? v : 0.5 * inversesqrt(max(1.0 - x * x, 1e-7)) - v;
//
//    return cross(v1, v2) * theta_sintheta;
//}





//ESTA VERSION ES LA PRIMERA, SOLO POINT_LIGHT (Lo tengo de referencia x ahora )
//#version 460 core
//#include "common.glsl"
//
//const float PI = 3.14159265359;
//
//in vec2 TexCoords;
//out vec4 FragColor;
//
////-- G-BUFFER MAPS
//uniform sampler2D gPosition;
//uniform sampler2D gNormal;
//uniform sampler2D gAlbedo;
//uniform sampler2D gMetallicRoughness;
//uniform sampler2D gSpecular;
//uniform sampler2D ssaoTexture;  // Añadido para SSAO
//
////-- GLOBAL ILUMINATION
//uniform float ambientLight;
//uniform float exposure; //HDR
//
////-- LIGHTS
//const int NR_POINT_LIGHTS = 32;
//uniform PointLight pointLights[NR_POINT_LIGHTS];
//uniform vec3 viewPos;
//
//vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness);
//
//vec3 fresnelSchlick(float cosTheta, vec3 F0) {
//    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}
//
//float distributionGGX(vec3 N, vec3 H, float roughness) {
//    float a = roughness * roughness;
//    float a2 = a * a;
//    float NdotH = max(dot(N, H), 0.0);
//    float NdotH2 = NdotH * NdotH;
//
//    float num = a2;
//    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
//    denom = 3.14159265 * denom * denom;
//
//    return num / denom;
//}
//
//float geometrySchlickGGX(float NdotV, float roughness) {
//    float r = (roughness + 1.0);
//    float k = (r * r) / 8.0;
//
//    float num = NdotV;
//    float denom = NdotV * (1.0 - k) + k;
//
//    return num / denom;
//}
//
//float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
//    float ggx2 = geometrySchlickGGX(NdotV, roughness);
//    float ggx1 = geometrySchlickGGX(NdotL, roughness);
//
//    return ggx1 * ggx2;
//}
//
//void main()
//{      
//    const float gamma = 2.2;
//    
//    vec3 FragPos = texture(gPosition, TexCoords).rgb;
//    vec3 Normal = texture(gNormal, TexCoords).rgb;
//    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
//    float Specular = texture(gSpecular, TexCoords).r;
//    vec3 metallicRoughness = texture(gMetallicRoughness, TexCoords).rgb;
//    float ssao = texture(ssaoTexture, TexCoords).r;
//
//    float metallic = metallicRoughness.r;
//    float roughness = metallicRoughness.g;
//
//    vec3 F0 = vec3(0.04); 
//    F0 = mix(F0, Diffuse, metallic);
//
//    vec3 ambient = vec3(ambientLight * Diffuse * ssao);
//    vec3 lighting = ambient;
//
//    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
//    {
//        lighting += CalcPointLight(pointLights[i], FragPos, Normal, Diffuse, Specular, metallic, roughness);
//    }   
//
//    vec3 hdrColor = vec3(1.0) - exp(-lighting * exposure);
//    hdrColor = pow(hdrColor, vec3(1.0 / gamma));
//    FragColor = vec4(hdrColor, 1.0);
//}
//
//vec3 CalcPointLight(PointLight light, vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular, float metallic, float roughness) {
//    
//    float distance = length(light.Position - FragPos);
//    
//    if (distance >= light.Radius + light.LightSmoothness)
//        return vec3(0.0);
//
//    vec3 viewDir = normalize(viewPos - FragPos);
//    vec3 lightDir = normalize(light.Position - FragPos);
//    vec3 halfwayDir = normalize(lightDir + viewDir);
//    float distanceSquared = distance * distance;
//    float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distanceSquared);
//
//    if (distance > light.Radius) {
//        float smoothFactor = (light.Radius + light.LightSmoothness - distance) / light.LightSmoothness;
//        attenuation *= smoothstep(0.0, 1.0, smoothFactor);
//    }
//
//    vec3 radiance = light.colorAmbient * attenuation;
//
//    float NDF = distributionGGX(Normal, halfwayDir, roughness);
//    float G = geometrySmith(Normal, viewDir, lightDir, roughness);
//    vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), vec3(0.04));
//
//    vec3 numerator = NDF * G * F;
//    float denominator = 4.0 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, lightDir), 0.0) + 0.0001;
//    vec3 specular = numerator / denominator;
//
//    vec3 kS = F;
//    vec3 kD = vec3(1.0) - kS;
//    kD *= 1.0 - metallic;
//
//    float NdotL = max(dot(Normal, lightDir), 0.0);
//    return (kD * Diffuse / PI + specular) * radiance * NdotL;
//}