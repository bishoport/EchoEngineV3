#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D deferredTexture;
uniform sampler2D forwardTexture;
uniform sampler2D ssaoTexture;

uniform bool useSSAO;

void main()
{
    vec4 deferredColor = texture(deferredTexture, TexCoords);
    vec4 forwardColor = texture(forwardTexture, TexCoords);
    float ssao = useSSAO ? texture(ssaoTexture, TexCoords).r : 1.0;

    float ssaoFactor = 0.5;
    ssao = mix(1.0, ssao, ssaoFactor);

    vec4 color = deferredColor * vec4(vec3(ssao), 1.0) + forwardColor;

    FragColor = color;
}




//SIN SOMBRA
//#version 460 core
//
//out vec4 FragColor;
//
//in vec2 TexCoords;
//
//uniform sampler2D deferredTexture;
//uniform sampler2D forwardTexture;
//uniform sampler2D ssaoTexture;
//
//uniform bool useSSAO; // Uniform para activar/desactivar SSAO
//
//void main()
//{
//    vec4 deferredColor = texture(deferredTexture, TexCoords);
//    vec4 forwardColor = texture(forwardTexture, TexCoords);
//    float ssao = useSSAO ? texture(ssaoTexture, TexCoords).r : 1.0; // Aplicar SSAO si está habilitado
//
//    // Reducir el impacto del SSAO multiplicándolo por un factor (ajustar según sea necesario)
//    float ssaoFactor = 0.5;
//    ssao = mix(1.0, ssao, ssaoFactor);
//
//    // Ajustar la mezcla de los colores para no oscurecer demasiado
//    vec4 color = deferredColor * vec4(vec3(ssao), 1.0) + forwardColor;
//
//    FragColor = color;
//}



//#version 460 core
//
//out vec4 FragColor;
//
//in vec2 TexCoords;
//
//uniform sampler2D deferredTexture;
//uniform sampler2D forwardTexture;
//uniform sampler2D ssaoTexture;
//
//void main()
//{
//    vec4 deferredColor = texture(deferredTexture, TexCoords);
//    vec4 forwardColor = texture(forwardTexture, TexCoords);
//    float ssao = texture(ssaoTexture, TexCoords).r;  // Obtener el valor de SSAO
//
//    // Reducir el impacto del SSAO multiplicándolo por un factor (ajustar según sea necesario)
//    float ssaoFactor = 0.5;
//    ssao = mix(1.0, ssao, ssaoFactor);
//
//    // Ajustar la mezcla de los colores para no oscurecer demasiado
//    vec4 color = deferredColor * vec4(vec3(ssao), 1.0) + forwardColor;
//
//    FragColor = color;
//}



//#version 460 core
//
//out vec4 FragColor;
//
//in vec2 TexCoords;
//
//uniform sampler2D deferredTexture;
//uniform sampler2D forwardTexture;
//uniform sampler2D ssaoTexture;
//
//void main()
//{
//    vec4 deferredColor = texture(deferredTexture, TexCoords);
//    vec4 forwardColor = texture(forwardTexture, TexCoords);
//    float ssao = texture(ssaoTexture, TexCoords).r;
//
//    // Aplicar SSAO solo a la componente ambiental
//    vec4 color = deferredColor;
//    color.rgb *= vec3(ssao);
//
//    // Combinar con el buffer de forward rendering
//    color += forwardColor;
//
//    FragColor = color;
//}








//#version 460 core
//
//out vec4 FragColor;
//
//in vec2 TexCoords;
//
//uniform sampler2D deferredTexture;
//uniform sampler2D forwardTexture;
//
//void main()
//{
//    vec4 deferredColor = texture(deferredTexture, TexCoords);
//    vec4 forwardColor = texture(forwardTexture, TexCoords);
//    FragColor = deferredColor + forwardColor;
//}




