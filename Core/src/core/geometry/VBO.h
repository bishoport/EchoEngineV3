#pragma once

#include "../Core.h"

namespace libCore
{
#define MAX_BONE_INFLUENCE 4
	//---Vertex Buffer Object(VBO)
	//-------------------------------------------------------------------------
	//Es un contenedor de memoria en la GPU que almacena datos de vértices.
	//Un VBO puede contener información como la posición de los vértices, las coordenadas de textura, las normales, 
	//los colores, y otros datos de atributos de vértices que definen la forma y la apariencia de tu objeto 3D.
	//----------------------------------------------------------------------------------------------------------------

	class VBO
	{
	public:
		// Reference ID of the Vertex Buffer Object
		GLuint ID;
		// Constructor that generates a Vertex Buffer Object and links it to vertices
		VBO(std::vector<Vertex>& vertices);

		// Binds the VBO
		void Bind();
		// Unbinds the VBO
		void Unbind();
		// Deletes the VBO
		void Delete();
	};
}