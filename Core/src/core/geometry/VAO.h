#pragma once

#include "../Core.h"
#include"VBO.h"


//-- Vertex Array Object(VAO) -------------------------------------------------------------------------------------
//Actúa como un administrador de los VBOs y EBOs.Un VAO guarda la configuración de cómo los datos de vértices
//y los índices están almacenados en los VBOs y EBOs respectivamente.Esto incluye información sobre el formato 
//de los datos de vértices, cómo acceder a ellos, y qué VBO contiene qué datos.El VAO no almacena los datos de 
//vértices por sí mismo, sino que mantiene referencias a los VBOs y EBOs que sí lo hacen.
//---------------------------------------------------------------------------------------------------------------

namespace libCore
{
	class VAO
	{
	public:
		// ID reference for the Vertex Array Object
		GLuint ID;
		// Constructor that generates a VAO ID
		VAO();

		// Links a VBO Attribute such as a position or color to the VAO
		void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizei stride, void* offset);
		// Binds the VAO
		void Bind();
		// Unbinds the VAO
		void Unbind();
		// Deletes the VAO
		void Delete();
	};
}


//--La relación de trabajo entre estos objetos es la siguiente :
//--Creas un VBO y subes tus datos de vértices a la GPU.
//--Opcionalmente, creas un EBO y subes tus índices, si estás usando indexación para tus vértices.
//Creas un VAO y configuras cómo OpenGL debe interpretar los datos del VBO(y opcionalmente del EBO).
//Esto incluye indicar el layout de los datos del vértice y la configuración de los índices del EBO si es que usas uno.
//Cuando dibujas tu objeto, enlazas(bind) el VAO, y OpenGL utiliza toda la configuración almacenada en el VAO para obtener 
//los datos del VBO y EBO para renderizar la geometría.
// 
//Cada vez que quieras dibujar un objeto, solo necesitas enlazar el VAO correspondiente y ejecutar el comando de dibujo.
//Esto hace que la gestión de múltiples objetos en la escena sea mucho más eficiente y organizada.
