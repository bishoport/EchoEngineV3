#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
	FragColor = texture(screenTexture, TexCoords);
}





//#version 460 core
//out vec4 FragColor;
//
//in vec2 TexCoords;
//
//uniform sampler2D screenTexture;
//uniform sampler2D depthTexture;
//uniform float focusDepth;
//uniform float blurRadius;
//uniform float blurAmount;
//uniform int sampleCount; // Nuevo uniform para controlar el número de samples
//
//void main()
//{
//    float depth = texture(depthTexture, TexCoords).r;
//    vec3 col = texture(screenTexture, TexCoords).rgb;
//    
//    float factor = abs(depth - focusDepth);
//    float smoothFactor = 1.0 - smoothstep(0.0, blurRadius, factor);
//    
//    vec3 blurredCol = vec3(0.0);
//    float blurSize = 0.005 * smoothFactor * blurAmount;
//    
//    // Modificado para usar sampleCount
//    for (int i = -sampleCount / 2; i <= sampleCount / 2; ++i)
//    {
//        for (int j = -sampleCount / 2; j <= sampleCount / 2; ++j)
//        {
//            vec2 sampleCoords = TexCoords + vec2(i, j) * blurSize;
//            blurredCol += texture(screenTexture, sampleCoords).rgb;
//        }
//    }
//    blurredCol /= float(sampleCount * sampleCount);
//    col = mix(col, blurredCol, smoothFactor);
//
//    FragColor = vec4(col, 1.0);
//}







//#version 460 core
//out vec4 FragColor;
//
//in vec2 TexCoords;
//
//uniform sampler2D screenTexture;
//
//void main()
//{
//    // Obtener el color original de la textura
//    vec3 col = texture(screenTexture, TexCoords).rgb;
//    
//    // Invertir cada componente de color
//    vec3 invertedCol = 1.0 - col;
//    
//    // Asignar el color invertido a FragColor con alfa de 1.0
//    FragColor = vec4(invertedCol, 1.0);
//}



