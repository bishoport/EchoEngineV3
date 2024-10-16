#pragma once
#include "../Core.h"

namespace libCore {
    class Transform {
    public:
        // Inicializa la Transformación
        Transform() = default;

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

        // Global space information concatenate in matrix
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Dirty flag
        bool isDirty = true;

        glm::mat4 getLocalModelMatrix() const {
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rotationMatrix = glm::toMat4(rotation);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
            return translationMatrix * rotationMatrix * scaleMatrix;
        }

        glm::mat4 getMatrix() const {
            return getLocalModelMatrix();
        }

        glm::mat4 getRotationMatrix() const {
            return glm::toMat4(rotation);
        }

        void setMatrix(const glm::mat4& matrix) {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(matrix, scale, rotation, position, skew, perspective);
            // Normalizar la rotación
            rotation = glm::normalize(rotation);
            // Actualizar ángulos de Euler
            eulerAngles = glm::eulerAngles(rotation);
            isDirty = true;
        }

        void updateRotationFromEulerAngles() {
            rotation = glm::quat(eulerAngles);
            rotation = glm::normalize(rotation);
            isDirty = true;
        }

        // Funciones para obtener y establecer la posición
        glm::vec3 GetPosition() const { return position; }
        void SetPosition(const glm::vec3& pos) { position = pos; isDirty = true; }

        // Funciones para obtener y establecer la rotación
        glm::quat GetRotation() const { return rotation; }
        void SetRotation(const glm::quat& rot) {
            rotation = rot;
            rotation = glm::normalize(rotation);
            eulerAngles = glm::eulerAngles(rotation);
            isDirty = true;
        }
        void SetRotation(const glm::vec3& euler) {
            eulerAngles = euler;
            updateRotationFromEulerAngles();
            isDirty = true;
        }

        // Funciones para obtener y establecer la escala
        glm::vec3 GetScale() const { return scale; }
        void SetScale(const glm::vec3& scl) { scale = scl; isDirty = true; }

        // Función para obtener y establecer los ángulos de Euler
        glm::vec3 GetEulerAngles() const { return eulerAngles; }
        void SetEulerAngles(const glm::vec3& euler) {
            eulerAngles = euler;
            updateRotationFromEulerAngles();
            isDirty = true;
        }

        // Computa la matriz del modelo local
        void computeModelMatrix() {
            modelMatrix = getLocalModelMatrix();
            isDirty = false;
        }

        // Computa la matriz del modelo global con una matriz de padre
        void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix) {
            modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
            isDirty = false;
        }

        // Funciones para obtener los vectores de dirección globales
        glm::vec3 getRight() const {
            return glm::normalize(glm::vec3(modelMatrix[0]));
        }

        glm::vec3 getUp() const {
            return glm::normalize(glm::vec3(modelMatrix[1]));
        }

        glm::vec3 getForward() const {
            return glm::normalize(-glm::vec3(modelMatrix[2]));
        }

        // Funciones para obtener la posición y escala global
        glm::vec3 getGlobalPosition() const {
            return glm::vec3(modelMatrix[3]);
        }

        glm::vec3 getGlobalScale() const {
            return { glm::length(getRight()), glm::length(getUp()), glm::length(getForward()) };
        }

        // Función para verificar si la transformación está "sucia"
        bool isTransformDirty() const {
            return isDirty;
        }
    };
}



//#pragma once
//#include "../Core.h"
//
//namespace libCore {
//    class Transform {
//    public:
//        // Inicializa la Transformación
//        Transform() = default;
//
//        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
//        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
//        glm::vec3 eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
//        glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
//
//        glm::mat4 getLocalModelMatrix() const {
//            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
//            glm::mat4 rotationMatrix = glm::toMat4(rotation);
//            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
//            return translationMatrix * rotationMatrix * scaleMatrix;
//        }
//
//        glm::mat4 getMatrix() const {
//            return getLocalModelMatrix();
//        }
//
//        glm::mat4 getRotationMatrix() const {
//            return glm::toMat4(rotation);
//        }
//
//        void setMatrix(const glm::mat4& matrix) {
//            glm::vec3 skew;
//            glm::vec4 perspective;
//            glm::decompose(matrix, scale, rotation, position, skew, perspective);
//            // Normalizar la rotación
//            rotation = glm::normalize(rotation);
//            // Actualizar ángulos de Euler
//            eulerAngles = glm::eulerAngles(rotation);
//        }
//
//        void updateRotationFromEulerAngles() {
//            rotation = glm::quat(eulerAngles);
//            rotation = glm::normalize(rotation);
//        }
//
//        // Funciones para obtener y establecer la posición
//        glm::vec3 GetPosition() const { return position; }
//        void SetPosition(const glm::vec3& pos) { position = pos; }
//
//        // Funciones para obtener y establecer la rotación
//        glm::quat GetRotation() const { return rotation; }
//        void SetRotation(const glm::quat& rot) {
//            rotation = rot;
//            rotation = glm::normalize(rotation);
//            eulerAngles = glm::eulerAngles(rotation);
//        }
//        void SetRotation(const glm::vec3& euler) {
//            eulerAngles = euler;
//            updateRotationFromEulerAngles();
//        }
//
//        // Funciones para obtener y establecer la escala
//        glm::vec3 GetScale() const { return scale; }
//        void SetScale(const glm::vec3& scl) { scale = scl; }
//
//        // Función para obtener y establecer los ángulos de Euler
//        glm::vec3 GetEulerAngles() const { return eulerAngles; }
//        void SetEulerAngles(const glm::vec3& euler) {
//            eulerAngles = euler;
//            updateRotationFromEulerAngles();
//        }
//    };
//}