#version 460 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gPositionView;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

uniform float radius;
uniform float bias;
uniform float intensity;
uniform float power;

uniform mat4 projection;
uniform mat4 view;

uniform float screenWidth;
uniform float screenHeight;

void main()
{
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 fragPosView = texture(gPositionView, TexCoords).rgb;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * vec2(screenWidth / 4.0, screenHeight / 4.0)).xyz);
    
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    for(int i = 0; i < 64; ++i)
    {
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPosView + samplePos * radius;
        
        vec4 offset = projection * vec4(samplePos, 1.0); // from view-space to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        float sampleDepth = texture(gPositionView, offset.xy).z; // get depth value of kernel sample

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPosView.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / 64.0);
    occlusion = pow(occlusion, power); // Apply power to adjust contrast
    FragColor = mix(1.0, occlusion, intensity); // Use intensity to scale the overall effect
}





//EL BUENO
//#version 460 core
//out float FragColor;
//
//in vec2 TexCoords;
//
//uniform sampler2D gPosition;
//uniform sampler2D gNormal;
//uniform sampler2D texNoise;
//
//uniform vec3 samples[64];
//
//uniform float radius;
//uniform float bias;
//uniform float intensity;
//uniform float power;
//
//uniform mat4 projection;
//uniform mat4 view;
//
//uniform float screenWidth;
//uniform float screenHeight;
//
//void main()
//{
//    vec3 fragPos = texture(gPosition, TexCoords).xyz;
//    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
//    vec3 randomVec = normalize(texture(texNoise, TexCoords * vec2(screenWidth / 4.0, screenHeight / 4.0)).xyz);
//
//    // Create TBN change-of-basis matrix: from tangent-space to view-space
//    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
//    vec3 bitangent = cross(normal, tangent);
//    mat3 TBN = mat3(tangent, bitangent, normal);
//
//    float occlusion = 0.0;
//    for(int i = 0; i < 64; ++i)
//    {
//        // Transform sample to view space
//        vec3 samplePos = TBN * samples[i];
//        samplePos = fragPos + samplePos * radius;
//        
//        // Transform sample to clip space
//        vec4 offset = vec4(samplePos, 1.0);
//        offset = projection * offset; // from view to clip-space
//        offset.xyz /= offset.w; // perspective divide
//        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
//        
//        // Get depth value of kernel sample
//        float sampleDepth = texture(gPosition, offset.xy).z;
//        
//        // Range check to avoid artifacts
//        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
//        
//        // Accumulate occlusion based on depth difference
//        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
//    }
//
//    // Normalize and apply intensity and power
//    occlusion = 1.0 - (occlusion / 64.0);
//    occlusion = pow(occlusion, power);
//    
//    // Adjust the intensity influence
//    occlusion = mix(1.0, occlusion, intensity);
//
//    FragColor = occlusion;
//}