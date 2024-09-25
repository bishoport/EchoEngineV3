#pragma once

#include<glm/glm.hpp>

namespace libCore
{
    struct BoneInfo
    {
        int id;  // ID del hueso
        glm::mat4 offset;  // Offset matrix, usada para convertir el v�rtice a coordenadas locales del hueso
        glm::mat4 finalTransformation;  // Matriz final de transformaci�n
    };

}
