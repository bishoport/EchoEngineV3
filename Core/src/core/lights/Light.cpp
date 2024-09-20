#include "Light.h"
#include "../../managers/ShaderManager.h"

namespace libCore
{
	void Light::UpdateLightPosition()
    {
        auto [sceneCenter, sceneRadius] = SceneBounds;

        currentSceneRadius = sceneRadius;

        if (sceneRadius > 0.0f)
        {
            sceneRadius += sceneRadiusOffset;

            orthoLeft = -sceneRadius;
            orthoRight = sceneRadius;
            orthoBottom = -sceneRadius;
            orthoTop = sceneRadius;
            orthoNear = -sceneRadius - orthoNearOffset;
            orthoFar = (2 * sceneRadius) + orthoFarOffset;

            // Calcula la posición de la luz basada en los ángulos y la distancia al centro de la escena
            transform.position.x = sceneCenter.x + sceneRadius * sin(angleX) * cos(angleY);
            transform.position.y = sceneCenter.y + sceneRadius * cos(angleX);
            transform.position.z = sceneCenter.z + sceneRadius * sin(angleX) * sin(angleY);

            // Actualiza la dirección de la luz
            direction = glm::normalize(sceneCenter - transform.position);
        }
    }
    void Light::SetLightDataToShader(const std::string& shader)
    {
        glm::vec3 scaledColor = color * intensity; // Escalar el color de la luz según la intensidad

        if (type == POINT)
        {
            libCore::ShaderManager::Get(shader)->setVec3("pointLights[" + std::to_string(id) + "].Position", transform.position);
            libCore::ShaderManager::Get(shader)->setVec3("pointLights[" + std::to_string(id) + "].colorAmbient", scaledColor); // Usar color escalado
            libCore::ShaderManager::Get(shader)->setFloat("pointLights[" + std::to_string(id) + "].Linear", Linear);
            libCore::ShaderManager::Get(shader)->setFloat("pointLights[" + std::to_string(id) + "].Quadratic", Quadratic);
            libCore::ShaderManager::Get(shader)->setFloat("pointLights[" + std::to_string(id) + "].Radius", Radius);
            libCore::ShaderManager::Get(shader)->setFloat("pointLights[" + std::to_string(id) + "].LightSmoothness", LightSmoothness);
        }
        else if (type == SPOT)
        {
            libCore::ShaderManager::Get(shader)->setVec3("spotLights[" + std::to_string(id) + "].Position", transform.position);
            libCore::ShaderManager::Get(shader)->setVec3("spotLights[" + std::to_string(id) + "].colorAmbient", scaledColor); // Usar color escalado
            libCore::ShaderManager::Get(shader)->setFloat("spotLights[" + std::to_string(id) + "].Linear", Linear);
            libCore::ShaderManager::Get(shader)->setFloat("spotLights[" + std::to_string(id) + "].Quadratic", Quadratic);
            libCore::ShaderManager::Get(shader)->setFloat("spotLights[" + std::to_string(id) + "].Radius", Radius);
            libCore::ShaderManager::Get(shader)->setFloat("spotLights[" + std::to_string(id) + "].LightSmoothness", LightSmoothness);
            libCore::ShaderManager::Get(shader)->setVec3("spotLights[" + std::to_string(id) + "].Direction", transform.getRotationMatrix() * glm::vec4(0, 0, -1, 0));
            libCore::ShaderManager::Get(shader)->setFloat("spotLights[" + std::to_string(id) + "].innerCutoff", innerCutoff);
            libCore::ShaderManager::Get(shader)->setFloat("spotLights[" + std::to_string(id) + "].outerCutoff", outerCutoff);
        }
        else if (type == AREA)
        {
            libCore::ShaderManager::Get(shader)->setVec3("areaLights[" + std::to_string(id) + "].color", scaledColor); // Usar color escalado
            libCore::ShaderManager::Get(shader)->setFloat("areaLights[" + std::to_string(id) + "].intensity", intensity);

            for (size_t i = 0; i < areaLightpoints.size(); ++i)
            {
                glm::vec3 transformedPoint = transform.position + areaLightpoints[i];
                libCore::ShaderManager::Get(shader)->setVec3("areaLights[" + std::to_string(id) + "].points[" + std::to_string(i) + "]", transformedPoint);
            }
            libCore::ShaderManager::Get(shader)->setBool("areaLights[" + std::to_string(id) + "].twoSided", twoSided);
        }
        else if (type == DIRECTIONAL)
        {
            libCore::ShaderManager::Get(shader)->setBool("directionalLight.isActive", true);
            libCore::ShaderManager::Get(shader)->setVec3("directionalLight.direction", direction);
            libCore::ShaderManager::Get(shader)->setVec3("directionalLight.ambient", ambient * intensity);
            libCore::ShaderManager::Get(shader)->setVec3("directionalLight.diffuse", diffuse * intensity);
            libCore::ShaderManager::Get(shader)->setVec3("directionalLight.specular", specular * intensity);

            // Enviar datos de sombras
            libCore::ShaderManager::Get(shader)->setBool("directionalLight.drawShadows", drawShadows);
            libCore::ShaderManager::Get(shader)->setFloat("directionalLight.shadowIntensity", shadowIntensity);
            libCore::ShaderManager::Get(shader)->setBool("directionalLight.usePoisonDisk", usePoisonDisk);
            //libCore::ShaderManager::Get(shader)->setInt("directionalLight.shadowMap", chanelShadowTexture);
            libCore::ShaderManager::Get(shader)->setMat4("directionalLight.shadowBiasMVP", shadowBias * shadowMVP);
        }

        // Enviar uniformes adicionales
        libCore::ShaderManager::Get(shader)->setFloat("LUT_SIZE", LUT_SIZE);
        libCore::ShaderManager::Get(shader)->setFloat("LUT_SCALE", LUT_SCALE);
        libCore::ShaderManager::Get(shader)->setFloat("LUT_BIAS", LUT_BIAS);
    }
    void Light::DrawDebugLight(const std::string& shader)
    {
        if (!showDebug) return;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.position);

        libCore::ShaderManager::Get(shader)->setVec4("u_Color", glm::vec4(color, 1.0f));
        libCore::ShaderManager::Get(shader)->setMat4("model", model);

        glBindVertexArray(VAO);

        glPointSize(30.0f);

        if (type == POINT || type == SPOT)
        {
            // Dibujar un solo punto
            glDrawArrays(GL_POINTS, 0, 1);
        }
        else if (type == AREA)
        {
            // Dibujar los puntos del área
            glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertices.size()));


            // Dibujar líneas entre los puntos para formar el polígono
            std::vector<GLuint> indices;
            for (GLsizei i = 0; i < vertices.size(); ++i)
            {
                indices.push_back(i);
                indices.push_back((i + 1) % vertices.size());
            }

            GLuint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_DYNAMIC_DRAW);

            glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDeleteBuffers(1, &EBO);
        }
        else if (type == DIRECTIONAL)
        {
            // Dibujar el punto de la luz direccional
            glDrawArrays(GL_POINTS, 0, 1);

            // Crea un array con los puntos de la línea
            glm::vec3 rotationVector = direction;

            glm::vec3 linePoints[] = {
                transform.position,
                transform.position + rotationVector * 10.0f  // Aquí asumimos que la longitud de la línea es 10
            };

            // Crea el VBO y el VAO para la línea
            GLuint lineVBO, lineVAO;
            glGenBuffers(1, &lineVBO);
            glGenVertexArrays(1, &lineVAO);

            // Rellena el VBO con los puntos de la línea
            glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(linePoints), linePoints, GL_STATIC_DRAW);

            // Enlaza el VBO al VAO
            glBindVertexArray(lineVAO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            // Dibuja la línea
            glLineWidth(2);

            glBindVertexArray(lineVAO);
            glDrawArrays(GL_LINES, 0, 2);

            // Elimina el VBO y el VAO de la línea
            glDeleteBuffers(1, &lineVBO);
            glDeleteVertexArrays(1, &lineVAO);
        }

        glBindVertexArray(0);
    }
    void Light::UpdateVertices()
    {
        vertices.clear();

        if (type == POINT || type == SPOT)
        {
            vertices.push_back(glm::vec3(0.0f)); // Usar (0,0,0) ya que el modelo se encargará del desplazamiento
        }
        else if (type == AREA)
        {
            for (const auto& point : areaLightpoints)
            {
                vertices.push_back(point); // Solo agregar los puntos del área sin transformarlos aquí
            }
        }
        else if (type == DIRECTIONAL)
        {
            vertices.push_back(transform.position); // Añadir el punto de origen
            vertices.push_back(transform.position + direction); // Añadir el punto en la dirección de la luz
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    void Light::InitializeBuffers()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 100, nullptr, GL_DYNAMIC_DRAW); // Prealoca espacio para 100 vértices

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

}