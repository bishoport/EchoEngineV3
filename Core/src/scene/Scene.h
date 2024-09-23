#pragma once

#include "../core/Core.h"
#include "../core/Timestep.h"

namespace libCore {

    class Scene {

    public:
        // Nombre de la escena
        std::string sceneName;

        // Constructor que recibe el nombre de la escena
        Scene(const std::string& name);

        //--SERIALIZATION SCENE
        void SerializeScene();
        void DeserializeScene(const std::string& _sceneName);

        // Serialización y deserialización solo de componentes (archivo temporal)
        void SerializeComponents();
        void DeserializeComponents();
    };
}
