#pragma once
#include "../Core.h"
#include "../geometry/VAO.h"
#include "../geometry/EBO.h"
#include "../geometry/AABB.h"
#include "Transform.h"

namespace libCore
{
    class Mesh
    {
    public:
        Mesh() = default;

        //--MAIN MESH DATA
        std::string meshName = "unknown_mesh_name";
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        DRAW_GEOM_LIKE drawLike = DRAW_GEOM_LIKE::TRIANGLE;
        Ref<AABB> aabb = nullptr;

        //--THUMBNAIL TEXTURE ID
        GLuint thumbnailTextureID = 0; // Almacena el ID de la textura del thumbnail

        //--DRAW MESH
        void SetupMesh();
        void Draw();
        void DrawInstanced(GLsizei instanceCount, const std::vector<glm::mat4>& instanceMatrices);
        void SetupInstanceVBO(const std::vector<glm::mat4>& instanceMatrices);

        void DrawAABB()
        {
            VAO.Bind();

            glLineWidth(2.0f);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

            VAO.Unbind();
        }

        //--MAIN MESH
        VAO VAO;

        // Instancing data
        GLuint instanceVBO = 0;        
    };
}
