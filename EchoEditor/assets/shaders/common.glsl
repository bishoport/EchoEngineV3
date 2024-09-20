struct PointLight {
    vec3 Position;
    vec3 colorAmbient;
    float Linear;
    float Quadratic;
    float Radius;
    float LightSmoothness;
};

struct SpotLight {
    vec3 Position;
    vec3 Direction;
    vec3 colorAmbient;
    float Linear;
    float Quadratic;
    float Radius;
    float LightSmoothness;
    float innerCutoff;
    float outerCutoff;
};

struct AreaLight {
    vec3 color;
    float intensity;
    vec3 points[4];
    bool twoSided;
};



struct DirectionalLight {
    bool isActive;

    vec3 direction;
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    //SHADOWS
    bool drawShadows;
    float shadowIntensity;
    bool usePoisonDisk;
    sampler2D shadowMap;
    mat4 shadowBiasMVP;
};


