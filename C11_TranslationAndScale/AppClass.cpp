#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh2 = new MyMesh();
	//m_pMesh->GenerateCube(1.0f, C_WHITE);
	m_pMesh->GenerateSphere(1.0f, 5, C_WHITE);
	m_pMesh2->GenerateTorus(1, .5, 12, 12, C_PURPLE);
	m_pCameraMngr->SetPositionTargetAndUpward(vector3(0, 0, 5), vector3(0, 0, 0), AXIS_Y);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	/*
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	static float value = 0.0f;
	matrix4 m4Translate = glm::translate(IDENTITY_M4, vector3(value, 2.0f, 3.0f));
	value += 0.01f;
	*/
	
	matrix4 m4Model;

	/*uint uClock = m_pSystem->GenClock();
	float fDeltaTime = m_pSystem->GetDeltaTime(uClock);
	std::cout << fDeltaTime << std::endl;
	*/

	//Translation

	//using GLM for translation
	//matrix4 m4Trans = glm::translate(vector3(1, 0, 0));
	static float fTransX = 0.0f;
	matrix4 m4Trans = glm::translate(IDENTITY_M4, vector3(fTransX, 0, 0));
	fTransX += 0.01f;

	/*
	//modify matrix directly
	vector4 v4 = m4Model[3];
	m4Model[3][0] = 1.0f;
	*/

	/*
	// modify by save in v4
	v4.x = 1.0f;
	m4Matrix[3] = v4;
	*/

	//Scaling 

	//modify matrix directly to scale
	/*
	for (size_t i = 0; i < 3; i++)
	{
		m4Model[i][i] = 5;
	}
	*/

	//glm for scaling
	matrix4 m4Scale = glm::scale(m4Trans, vector3(1));
	m_pMesh->Render(m4Projection, m4View, m4Scale);
	m4Scale = glm::scale(m4Trans, vector3(4));
	m_pMesh2->Render(m4Projection, m4View, m4Scale);

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);
	SafeDelete(m_pMesh2);
	//release GUI
	ShutdownGUI();
}