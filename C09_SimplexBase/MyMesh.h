/*----------------------------------------------
Programmer: TJ Wolschon
Date: 2019-2-7
----------------------------------------------*/
#ifndef __MYMESHCLASS_H_
#define __MYMESHCLASS_H_

#include "Definitions.h"

#endif

class MyMesh
{
public:
	GLuint m_uShaderProgramID; //Shader Program
	GLuint m_uVAO; //Vertex Array Obj
	GLuint m_uVBO; //Vertex buffer object
	
	std::vector<vector3> m_lVertex; // List of Vertex

	std::vector<vector3> m_lVertexPos; // List of Positions
	std::vector<vector3> m_lVertexCol; // List of Colors



	MyMesh();	
	~MyMesh();
	void Init();

	void AddPosition(vector3 a_v3Pos);
	void AddColor(vector3 a_v3Col);
	void CompileOpenGL3X();
	void Render(matrix4 a_m4Model, matrix4 a_m4View, matrix4 a_m4Projection);

private:
	void CompleteMesh();
};