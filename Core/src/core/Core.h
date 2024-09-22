#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <iomanip>
#include <stdexcept>
#include <map>
#include <memory>
#include <filesystem>
#include <limits>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <mutex>
#include <windows.h>
#include <random>


//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/ext/matrix_float2x2.hpp>

//--

//OpenGL-GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../tools/ConsoleLog.h"

#include "../managers/MainThreadTaskManager.h"

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}


//GLOBAL STRUCTS
enum TEXTURE_TYPES {
	ALBEDO,
	NORMAL,
	METALLIC,
	ROUGHNESS,
	AO
};
enum DRAW_GEOM_LIKE
{
	DOT,
	LINE,
	TRIANGLE
};
enum PRIMITIVES_3D
{
	PRIMITIVE_DOT,
	PRIMITIVE_LINE,
	PRIMIVITE_QUAD,
	PRIMIVITE_PLANE,
	PRIMIVITE_CUBE,
	PRIMIVITE_SPHERE
};
enum CAMERA_CONTROLLERS
{
	EDITOR,
	GAME
};
struct ImportModelData {
	std::string filePath = "";
	std::string fileName = "";
	int modelID = 0;
	bool invertUV = false;
	bool rotate90 = false;
	bool skeletal = false;
	bool useCustomTransform = true;
	bool processLights = false;
	float globalScaleFactor = 1.0f; // Ajusta según tus necesidades
};
struct ImportLUA_ScriptData {
	std::string filePath = "";
	std::string name = "";
};
struct ImageData {
	unsigned char* data;
	int width;
	int height;
	int numChannels;
};
enum class GizmoOperation
{
	Translate,
	Rotate2D, Rotate3D,
	Scale
};

// Structure to standardize the vertices used in the meshes
struct Vertex
{
	glm::vec3 position;
	glm::vec2 texUV;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	//int m_BoneIDs[MAX_BONE_INFLUENCE]; //bone indexes which will influence this vertex
	//float m_Weights[MAX_BONE_INFLUENCE]; //weights from each bone
};



class Texture;
class Material;
class AABB;
class Mesh;
class Model;
