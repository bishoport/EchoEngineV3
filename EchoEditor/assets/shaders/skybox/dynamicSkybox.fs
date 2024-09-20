#version 460 core

uniform vec3 sunPosition;
uniform vec3 sunDiskSize; // Tamaño del disco solar
uniform float gradientIntensity; // Intensidad del degradado
uniform float uAuraSize; // Tamaño del aura alrededor del sol
uniform float uAuraIntensity; // Intensidad del aura
uniform vec3 uDayLightColor;
uniform vec3 uSunsetColor; // Color intermedio para la puesta de sol
uniform vec3 uDayNightColor;
uniform vec3 uGroundColor;  // Color del suelo
uniform float uLowerBound;
uniform float uUpperBound;
uniform bool useSkyboxTexture; // Controla el uso de la textura del skybox

uniform float starDensity; // Densidad de estrellas
uniform float starSizeMin; // Tamaño mínimo de las estrellas
uniform float starSizeMax; // Tamaño máximo de las estrellas
uniform float starBrightnessMin; // Brillo mínimo de las estrellas
uniform float starBrightnessMax; // Brillo máximo de las estrellas
uniform vec2 starCoordScale; // Escala de las coordenadas para las estrellas

out vec4 FragColor;

uniform samplerCube skyboxTexture;

in vec3 TexCoords;

vec3 calculateSkyColor(vec3 normalizedPosition, vec3 normalizedSun) 
{
    // Establecemos el intervalo de transición cuando el sol comienza a tocar el horizonte.
    float firstMixFactor = smoothstep(uLowerBound, (uLowerBound + uUpperBound) * 0.5, normalizedSun.y);
    float secondMixFactor = smoothstep((uLowerBound + uUpperBound) * 0.5, uUpperBound, normalizedSun.y);
    vec3 intermediateColor = mix(uDayNightColor, uSunsetColor, firstMixFactor);

    vec3 currentTextureFragV3;
    if (useSkyboxTexture) {
        // Obtener el fragmento de textura actual
        vec4 currentTextureFragV4 = texture(skyboxTexture, TexCoords);
        currentTextureFragV3 = currentTextureFragV4.rgb;
    } else {
        currentTextureFragV3 = uDayLightColor; // Usar color de día como base si no se usa la textura del skybox
    }

    vec3 currentSkyColor = mix(intermediateColor, currentTextureFragV3, secondMixFactor);

    // Limitar el color naranja a una bruma en el horizonte justo donde cruza el sol
    float horizonEffect = max(0.0, 1.0 - abs(normalizedPosition.y / normalizedSun.y));
    vec3 horizonColor = mix(currentSkyColor, uSunsetColor, horizonEffect);

    if (secondMixFactor > 0.0 && secondMixFactor < 1.0) {
        currentSkyColor = horizonColor;
    }

    return currentSkyColor;
}

// Función para generar ruido simple para las estrellas
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// Función para generar la intensidad y el tamaño de la estrella
float starIntensityAndSize(vec3 direction, out float size) {
    vec2 coord = direction.xy * starCoordScale; // Escalamos las coordenadas para más estrellas
    float noiseValue = rand(coord);
    if (noiseValue > 1.0 - starDensity) 
    {
        size = mix(starSizeMin, starSizeMax, rand(coord * 2.0)); // Tamaño aleatorio
        return mix(starBrightnessMin, starBrightnessMax, rand(coord * 3.0)); // Brillo aleatorio
    } 
    else 
    {
        size = 0.0;
        return 0.0;
    }
}




void main() {
    vec3 normalizedPosition = normalize(TexCoords);
    vec3 normalizedSun = normalize(sunPosition);

    vec3 currentSkyColor = calculateSkyColor(normalizedPosition, normalizedSun);

    float difference = length(normalizedPosition - normalizedSun);

    // No pintar el sol y usar el color uGroundColor cuando está por debajo de la línea del ecuador.
    if (normalizedPosition.y < 0.0) {
        FragColor = vec4(uGroundColor, 1.0); // Usar el color del suelo
        return;
    }

    // Si la diferencia es menor que el tamaño del disco solar ajustado por gradientIntensity, pintamos el sol blanco.
    if (difference < sunDiskSize.x * gradientIntensity) 
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    } 
    else if (difference < sunDiskSize.x) 
    {
        float innerGradient = smoothstep(sunDiskSize.x * gradientIntensity, sunDiskSize.x, difference);
        vec3 transitionColor = mix(vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), innerGradient);
        FragColor = vec4(transitionColor, 1.0);
    } 
    else if (difference < (sunDiskSize.x + uAuraSize)) {
        float outerAura = 1.0 - smoothstep(sunDiskSize.x, sunDiskSize.x + uAuraSize, difference);
        vec3 auraColor = mix(currentSkyColor, vec3(1.0, 1.0, 0.0), outerAura * uAuraIntensity);
        FragColor = vec4(auraColor, 1.0);
    } else 
    {
        // Añadir estrellas cuando es de noche
        float starVisibility = smoothstep(0.0, uLowerBound, normalizedSun.y);
        float starSize;
        float starIntensityValue = starIntensityAndSize(normalizedPosition, starSize) * starVisibility;
        vec3 starColor = vec3(starIntensityValue) * starSize;

        FragColor = vec4(currentSkyColor + starColor, 1.0);
    }
}









//CON ESTRELLAS
//#version 460 core
//
//in vec3 TexCoords;
//
//uniform vec3 sunPosition;
//uniform vec3 sunDiskSize; // Tamaño del disco solar
//uniform float gradientIntensity; // Intensidad del degradado
//uniform float uAuraSize; // Tamaño del aura alrededor del sol
//uniform float uAuraIntensity; // Intensidad del aura
//uniform vec3 uDayLightColor;
//uniform vec3 uSunsetColor; // Color intermedio para la puesta de sol
//uniform vec3 uDayNightColor;
//uniform vec3 uGroundColor;  // Color del suelo
//uniform float uLowerBound;
//uniform float uUpperBound;
//uniform bool useSkyboxTexture; // Controla el uso de la textura del skybox
//
//// Uniforms para controlar estrellas
//uniform float starDensity; // Densidad de estrellas
//uniform float starSizeMin; // Tamaño mínimo de las estrellas
//uniform float starSizeMax; // Tamaño máximo de las estrellas
//uniform float starBrightnessMin; // Brillo mínimo de las estrellas
//uniform float starBrightnessMax; // Brillo máximo de las estrellas
//uniform vec2 starCoordScale; // Escala de las coordenadas para las estrellas
//
//out vec4 FragColor;
//
//uniform samplerCube skyboxTexture;
//
//vec3 calculateSkyColor(vec3 normalizedPosition, vec3 normalizedSun) {
//    // Establecemos el intervalo de transición cuando el sol comienza a tocar el horizonte.
//    float firstMixFactor = smoothstep(uLowerBound, (uLowerBound + uUpperBound) * 0.5, normalizedSun.y);
//    float secondMixFactor = smoothstep((uLowerBound + uUpperBound) * 0.5, uUpperBound, normalizedSun.y);
//    vec3 intermediateColor = mix(uDayNightColor, uSunsetColor, firstMixFactor);
//
//    vec3 currentTextureFragV3;
//    if (useSkyboxTexture) {
//        // Obtener el fragmento de textura actual
//        vec4 currentTextureFragV4 = texture(skyboxTexture, TexCoords);
//        currentTextureFragV3 = currentTextureFragV4.rgb;
//    } else {
//        currentTextureFragV3 = uDayLightColor; // Usar color de día como base si no se usa la textura del skybox
//    }
//
//    vec3 currentSkyColor = mix(intermediateColor, currentTextureFragV3, secondMixFactor);
//
//    // Limitar el color naranja a una bruma en el horizonte justo donde cruza el sol
//    float horizonEffect = max(0.0, 1.0 - abs(normalizedPosition.y / normalizedSun.y));
//    vec3 horizonColor = mix(currentSkyColor, uSunsetColor, horizonEffect);
//
//    if (secondMixFactor > 0.0 && secondMixFactor < 1.0) {
//        currentSkyColor = horizonColor;
//    }
//
//    return currentSkyColor;
//}
//
//// Función para generar ruido simple para las estrellas
//float rand(vec2 co) {
//    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
//}
//
//// Función para generar la intensidad y el tamaño de la estrella
//float starIntensityAndSize(vec3 direction, out float size) {
//    vec2 coord = direction.xy * starCoordScale; // Escalamos las coordenadas para más estrellas
//    float noiseValue = rand(coord);
//    if (noiseValue > 1.0 - starDensity) {
//        size = mix(starSizeMin, starSizeMax, rand(coord * 2.0)); // Tamaño aleatorio
//        return mix(starBrightnessMin, starBrightnessMax, rand(coord * 3.0)); // Brillo aleatorio
//    } else {
//        size = 0.0;
//        return 0.0;
//    }
//}
//
//void main() {
//    vec3 normalizedPosition = normalize(TexCoords);
//    vec3 normalizedSun = normalize(sunPosition);
//
//    vec3 currentSkyColor = calculateSkyColor(normalizedPosition, normalizedSun);
//
//    float difference = length(normalizedPosition - normalizedSun);
//
//    // No pintar el sol y usar el color uGroundColor cuando está por debajo de la línea del ecuador.
//    if (normalizedPosition.y < 0.0) {
//        FragColor = vec4(uGroundColor, 1.0); // Usar el color del suelo
//        return;
//    }
//
//    // Si la diferencia es menor que el tamaño del disco solar ajustado por gradientIntensity, pintamos el sol blanco.
//    if (difference < sunDiskSize.x * gradientIntensity) {
//        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
//    } else if (difference < sunDiskSize.x) {
//        float innerGradient = smoothstep(sunDiskSize.x * gradientIntensity, sunDiskSize.x, difference);
//        vec3 transitionColor = mix(vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), innerGradient);
//        FragColor = vec4(transitionColor, 1.0);
//    } else if (difference < (sunDiskSize.x + uAuraSize)) {
//        float outerAura = 1.0 - smoothstep(sunDiskSize.x, sunDiskSize.x + uAuraSize, difference);
//        vec3 auraColor = mix(currentSkyColor, vec3(1.0, 1.0, 0.0), outerAura * uAuraIntensity);
//        FragColor = vec4(auraColor, 1.0);
//    } else {
//        // Añadir estrellas cuando es de noche
//        float starVisibility = smoothstep(0.0, uLowerBound, normalizedSun.y);
//        float starSize;
//        float starIntensityValue = starIntensityAndSize(normalizedPosition, starSize) * starVisibility;
//        vec3 starColor = vec3(starIntensityValue) * starSize;
//
//        FragColor = vec4(currentSkyColor + starColor, 1.0);
//    }
//}








//SIN ESTRELLAS
//#version 460 core
//
//in vec3 TexCoords;
//
//uniform vec3 sunPosition;
//uniform vec3 sunDiskSize; // Tamaño del disco solar
//uniform float gradientIntensity; // Intensidad del degradado
//uniform float uAuraSize; // Tamaño del aura alrededor del sol
//uniform float uAuraIntensity; // Intensidad del aura
//uniform vec3 uDayLightColor;
//uniform vec3 uSunsetColor; // Color intermedio para la puesta de sol
//uniform vec3 uDayNightColor;
//uniform vec3 uGroundColor;  // Color del suelo
//uniform float uLowerBound;
//uniform float uUpperBound;
//uniform bool useSkyboxTexture; // Controla el uso de la textura del skybox
//
//out vec4 FragColor;
//
//uniform samplerCube skyboxTexture;
//
//
//
//vec3 calculateSkyColor(vec3 normalizedPosition, vec3 normalizedSun) {
//    // Establecemos el intervalo de transición cuando el sol comienza a tocar el horizonte.
//    float firstMixFactor = smoothstep(uLowerBound, (uLowerBound + uUpperBound) * 0.5, normalizedSun.y);
//    float secondMixFactor = smoothstep((uLowerBound + uUpperBound) * 0.5, uUpperBound, normalizedSun.y);
//    vec3 intermediateColor = mix(uDayNightColor, uSunsetColor, firstMixFactor);
//
//    vec3 currentTextureFragV3;
//    if (useSkyboxTexture) {
//        // Obtener el fragmento de textura actual
//        vec4 currentTextureFragV4 = texture(skyboxTexture, TexCoords);
//        currentTextureFragV3 = currentTextureFragV4.rgb;
//    } else {
//        currentTextureFragV3 = uDayLightColor; // Usar color de día como base si no se usa la textura del skybox
//    }
//
//    vec3 currentSkyColor = mix(intermediateColor, currentTextureFragV3, secondMixFactor);
//
//    // Limitar el color naranja a una bruma en el horizonte justo donde cruza el sol
//    float horizonEffect = max(0.0, 1.0 - abs(normalizedPosition.y / normalizedSun.y));
//    vec3 horizonColor = mix(currentSkyColor, uSunsetColor, horizonEffect);
//
//    if (secondMixFactor > 0.0 && secondMixFactor < 1.0) {
//        currentSkyColor = horizonColor;
//    }
//
//    return currentSkyColor;
//}
//
//void main() {
//    vec3 normalizedPosition = normalize(TexCoords);
//    vec3 normalizedSun = normalize(sunPosition);
//
//    vec3 currentSkyColor = calculateSkyColor(normalizedPosition, normalizedSun);
//
//    float difference = length(normalizedPosition - normalizedSun);
//
//    // No pintar el sol y usar el color uGroundColor cuando está por debajo de la línea del ecuador.
//    if (normalizedPosition.y < 0.0) {
//        FragColor = vec4(uGroundColor, 1.0); // Usar el color del suelo
//        return;
//    }
//
//    // Si la diferencia es menor que el tamaño del disco solar ajustado por gradientIntensity, pintamos el sol blanco.
//    if (difference < sunDiskSize.x * gradientIntensity) {
//        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
//    } else if (difference < sunDiskSize.x) {
//        float innerGradient = smoothstep(sunDiskSize.x * gradientIntensity, sunDiskSize.x, difference);
//        vec3 transitionColor = mix(vec3(1.0, 1.0, 1.0), vec3(1.0, 1.0, 0.0), innerGradient);
//        FragColor = vec4(transitionColor, 1.0);
//    } else if (difference < (sunDiskSize.x + uAuraSize)) {
//        float outerAura = 1.0 - smoothstep(sunDiskSize.x, sunDiskSize.x + uAuraSize, difference);
//        vec3 auraColor = mix(currentSkyColor, vec3(1.0, 1.0, 0.0), outerAura * uAuraIntensity);
//        FragColor = vec4(auraColor, 1.0);
//    } else {
//        FragColor = vec4(currentSkyColor, 1.0);
//    }
//}