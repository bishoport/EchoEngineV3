#include "ThumbnailGenerator.h"

#include "../managers/TextureManager.h"

//#include "../Core/EngineOpenGL.h"



void libCore::ThumbnailGenerator::GenerateThumbnail(Ref<Model> model, int width, int height)
{
    //std::cout << "Starting Thumbnail Generation..." << std::endl;

    //EngineOpenGL::GetInstance().pauseRenderer(true);

    // Crear el directorio de salida "thumbnails" si no existe
    std::filesystem::path thumbnailDir = std::filesystem::path(model->importModelData.filePath) / "thumbnails";
    std::filesystem::create_directories(thumbnailDir);

    // Crear el framebuffer
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Crear la textura donde se renderizará el modelo
    GLuint texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    // Crear un renderbuffer para el depth/stencil buffer
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

   // std::cout << "Framebuffer configured successfully." << std::endl;

    // Configurar el viewport
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, width, height);
    glClearColor(0.3f, 0.2f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // std::cout << "Viewport configured and buffers cleared." << std::endl;

    // Crear y configurar el shader
    auto shader = libCore::ShaderManager::Get("snapshot");
    shader->use();

    // Renderizar y capturar cada mesh individual, de forma recursiva
    RenderModelRecursively(model, shader, width, height, thumbnailDir);

    // Limpiar
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &texColorBuffer);
    glDeleteRenderbuffers(1, &rbo);

    //EngineOpenGL::GetInstance().pauseRenderer(false);

    //std::cout << "Thumbnail generation completed." << std::endl;
}

void libCore::ThumbnailGenerator::CaptureScreenshot(const std::string& filePath, int width, int height)
{
    // Capturar la imagen
    GLubyte* pixels = new GLubyte[3 * width * height];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Guardar la imagen como PNG
    TextureManager::getInstance().SaveImage(filePath.c_str(), width, height, 3, pixels);

    delete[] pixels;

    //std::cout << "Screenshot saved to: " << filePath << std::endl;
}

void libCore::ThumbnailGenerator::RenderModelRecursively(Ref<Model> model, Shader* shader, int width, int height, const std::filesystem::path& thumbnailDir)
{
    for (size_t i = 0; i < model->meshes.size(); ++i)
    {
        auto& mesh = model->meshes[i];
        Ref<Material> material = (i < model->materials.size()) ? model->materials[i] : nullptr;

        std::filesystem::path meshOutputFile = thumbnailDir / (mesh->meshName + "_thumbnail.png");

        // Si el archivo ya existe, eliminarlo
        if (std::filesystem::exists(meshOutputFile))
        {
            std::filesystem::remove(meshOutputFile);
        }

        // Limpiar buffers antes de cada renderizado
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //std::cout << "Rendering mesh: " << mesh->meshName << std::endl;

        // Calcular el centro y el radio del AABB
        glm::vec3 aabbCenter = (mesh->aabb->minBounds + mesh->aabb->maxBounds) / 2.0f;
        float aabbRadius = glm::length(mesh->aabb->maxBounds - aabbCenter);

        // Configurar la cámara
        float distanceFactor = 3.0f; // Ajustar este factor según sea necesario
        glm::vec3 cameraDirection = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)); // Dirección de la cámara
        glm::vec3 cameraPosition = aabbCenter + cameraDirection * aabbRadius * distanceFactor;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
        glm::mat4 view = glm::lookAt(cameraPosition, aabbCenter, glm::vec3(0.0f, 1.0f, 0.0f));

        shader->use();
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        shader->setMat4("model", glm::mat4(1.0f)); // Restablecer la transformación del modelo

        // Renderizar el mesh
        RenderMesh(mesh, shader, glm::mat4(1.0f), material);

        // Capturar el mesh
        CaptureScreenshot(meshOutputFile.string(), width, height);
    }

    // Recursivamente procesar los hijos
    for (auto& child : model->children)
    {
        RenderModelRecursively(child, shader, width, height, thumbnailDir);
    }
}
void libCore::ThumbnailGenerator::RenderMesh(Ref<Mesh> mesh, Shader* shader, const glm::mat4& modelMatrix, Ref<Material> material)
{
    shader->setMat4("model", modelMatrix);

    // Vincular el albedoMap si está disponible
    if (material && material->albedoMap)
    {
        
        material->albedoMap->Bind("snapshot");
    }

    // Vincular el VAO del mesh
    glBindVertexArray(mesh->VAO.ID);

    // Dibujar el mesh usando glDrawElements
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->indices.size()), GL_UNSIGNED_INT, 0);

    // Desvincular el VAO después de dibujar
    glBindVertexArray(0);

    //std::cout << "Mesh rendered: " << mesh->meshName << std::endl;
}


void libCore::ThumbnailGenerator::DrawCubeTest(Shader* shader)
{
    std::vector<glm::vec3> cubeVertices = {
        // Frente
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),

        // Atrás
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),

        // Izquierda
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),

        // Derecha
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),

        // Abajo
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),

        // Arriba
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f)
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(glm::vec3), cubeVertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    shader->setMat4("model", glm::mat4(1.0f));
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cubeVertices.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}