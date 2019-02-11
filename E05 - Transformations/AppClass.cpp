#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	//m_pMesh = new MyMesh();
	//m_pMesh->GenerateCube(1.0f, C_WHITE);
	//m_pMesh->GenerateSphere(1.0f, 5, C_WHITE);

	for (uint i = 0; i < 46; i++)
	{
		MyMesh* mesh = new MyMesh();
		meshList.push_back(mesh);
		meshList[i]->GenerateCube(1, C_PURPLE);
	}
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
	

	static float value = 0.00f;
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));
	
	for (uint i = 0; i < meshList.size(); i++)
	{
		vector3 position = DrawInvader() + vector3(value, sin(value), 0);
		matrix4 m4Translate = glm::translate(IDENTITY_M4, position);

		matrix4 m4Model = m4Scale * m4Translate;

		meshList[i]->Render(m4Projection, m4View, m4Model);
	}
	ResetInvaderArray();
	value += 0.01f;
	
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
	//SafeDelete(m_pMesh);
	for (uint i = 0; i < meshList.size(); i++)
	{
		SafeDelete(meshList[i]);
	}
	//release GUI
	ShutdownGUI();
}

vector3 Application::DrawInvader()
{
	
	for (int x = 0; x < invader[0].size(); x++)
	{
		for (int y = 0; y < invader.size(); y++)
		{
			if (invader[y][x])
			{
				invader[y][x] = false;
				return vector3(x-5, -y+4, -3); // -5, 4
			}
		}
	}

}

void Application::ResetInvaderArray()
{	
	std::vector<std::vector<bool>> temp = 
	{
		std::vector<bool>{0,0,1,0,0,0,0,0,1,0,0},
		std::vector<bool>{0,0,0,1,0,0,0,1,0,0,0},
		std::vector<bool>{0,0,1,1,1,1,1,1,1,0,0},
		std::vector<bool>{0,1,1,0,1,1,1,0,1,1,0},
		std::vector<bool>{1,1,1,1,1,1,1,1,1,1,1},
		std::vector<bool>{1,0,1,1,1,1,1,1,1,0,1},
		std::vector<bool>{1,0,1,0,0,0,0,0,1,0,1},
		std::vector<bool>{0,0,0,1,1,0,1,1,0,0,0}
	};
	invader = temp;
}