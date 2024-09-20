#pragma once

#include "../core/Core.h"
#include "../core/model/Model.h"

namespace libCore
{
    class ThumbnailGenerator
    {
    public:
        static void GenerateThumbnail(Ref<Model> model, int width = 128, int height = 128);
        static void RenderModelRecursively(Ref<Model> model, Shader* shader, int width, int height, const std::filesystem::path& thumbnailDir);
        static void RenderMesh(Ref<Mesh> mesh, Shader* shader, const glm::mat4& modelMatrix, Ref<Material> material);
        static void CaptureScreenshot(const std::string& filePath, int width, int height);
        static void DrawCubeTest(Shader* shader);
    };
}
