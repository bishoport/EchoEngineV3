#include "Mesh.h"

namespace libCore
{
    //MAIN MESH
    void Mesh::SetupMesh() {
        VAO.Bind();

        // Genera el VBO y lo enlaza con los vértices
        VBO VBO(vertices);

        // Genera el EBO y lo enlaza con los índices
        EBO EBO(indices);

        // Habilitar y configurar los atributos de vértices
        // 
        // Posiciones de vértices
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Coordenadas de textura de vértices
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texUV));

        // Normales de vértices
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

        // Tangentes de vértices
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

        // Bitangentes de vértices
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));


        //SKELETAL
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);



        // Desvincular para evitar modificaciones accidentales
        glBindVertexArray(0);
        VBO.Unbind();
        EBO.Unbind();

        aabb = CreateRef<AABB>();
        aabb->CalculateAABB(vertices);
    }

    void Mesh::SetupInstanceVBO(const std::vector<glm::mat4>& instanceMatrices)
    {
        VAO.Bind();

        if (instanceVBO == 0) {
            glGenBuffers(1, &instanceVBO);
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), &instanceMatrices[0], GL_STATIC_DRAW);

        for (unsigned int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(5 + i);
            glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(5 + i, 1);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Mesh::Draw()
    {
        VAO.Bind();

        if (drawLike == DRAW_GEOM_LIKE::DOT)
        {
            glPointSize(5.0f);
            glDrawArrays(GL_POINTS, 0, 1);
        }
        else if (drawLike == DRAW_GEOM_LIKE::LINE)
        {
            glLineWidth(1.0f);
            glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        }
        else if (drawLike == DRAW_GEOM_LIKE::TRIANGLE)
        {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        }

        VAO.Unbind();
    }

    void Mesh::DrawInstanced(GLsizei instanceCount, const std::vector<glm::mat4>& instanceMatrices)
    {
        SetupInstanceVBO(instanceMatrices);
        glBindVertexArray(VAO.ID);
        glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0, instanceCount);
        glBindVertexArray(0);
    }
}
