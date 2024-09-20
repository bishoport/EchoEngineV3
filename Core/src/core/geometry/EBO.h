#pragma once

#include "../Core.h"

//-- Element Buffer Object(EBO)------------------------------
//A veces también llamado Index Buffer Object(IBO), es otro tipo de buffer de memoria que almacena índices 
//que OpenGL utiliza para decidir qué vértices forman una primitiva(por ejemplo, un triángulo).
//Utilizar EBOs es eficiente porque te permite reutilizar vértices que son compartidos entre varias primitivas, 
//lo que puede disminuir significativamente la cantidad de datos que necesitas almacenar y procesar.
//------------------------------------------------------------------------------------------
namespace libCore
{
	class EBO
	{
	public:

		// ID reference of Elements Buffer Object
		GLuint ID;

		// Constructor that generates a Elements Buffer Object and links it to indices
		EBO(std::vector<GLuint>& indices);

		// Binds the EBO
		void Bind();
		// Unbinds the EBO
		void Unbind();
		// Deletes the EBO
		void Delete();
	};
}