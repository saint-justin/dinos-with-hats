#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 5.0f, 25.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

	m_pEntityMngr->AddEntity("Dinos\\RaptorDuck.fbx", "Steve");
	m_pEntityMngr->UsePhysicsSolver();

	//load all the different texture colors
	MaterialManager* pMatManager = MaterialManager::GetInstance();
	std::vector<String> diffuseNames = {"21_BrightRed.png", "23_BrightBlue.png", "24_BrightYellow.png", "37_BrightGreen.png", 
		"42_TransparentLightBlue.png", "151_SandGreen.png", "154_DarkRed.png", "212_LightRoyalBlue.png", "221_BrightPurple.png",
		"222_mLightPurple.png", "226_CoolYellow.png", "297_WarmGold.png", "311_mTransparentBrightGreen.png", "330_OliveGreen.png"};

	for (uint i = 0; i < diffuseNames.size(); i++)
	{
		Material newMat(diffuseNames[i]);
		newMat.LoadDiffuse(diffuseNames[i]);
		pMatManager->AddMaterial(newMat);
	}


	//seed for RNG
	srand((unsigned int)time(NULL));
	for (int i = 0; i < 100; i++)
	{
		int RandomValue = rand() % 4;

		//load and randomize the different dinisaur models
		switch (RandomValue)
		{
		case 0: 
			m_pEntityMngr->AddEntity("Dinos\\RaptorDuck.fbx", "Cube_" + std::to_string(i));
			break;
		case 1:
			m_pEntityMngr->AddEntity("Dinos\\BrachSafetyFBX.fbx", "Cube_" + std::to_string(i));
			break;
		case 2:
			m_pEntityMngr->AddEntity("Dinos\\TRexParty.fbx", "Cube_" + std::to_string(i));
			break;
		default:
			m_pEntityMngr->AddEntity("Dinos\\TrichCap.fbx", "Cube_" + std::to_string(i));
			break;
		}
		//set the position of the models
		vector3 v3Position = vector3(glm::sphericalRand(50.0f));
		v3Position.y = 0.0f;
		matrix4 m4Position = glm::translate(v3Position);
		m_pEntityMngr->SetModelMatrix(m4Position * glm::scale(vector3(2.0f)));
		m_pEntityMngr->UsePhysicsSolver();
		m_pEntityMngr->SetMass(2);

		// Set the material of the model
		MyEntity* pTempEntity = m_pEntityMngr->GetEntity();
		Model* pTempModel = pTempEntity->GetModel();
		int randomMatIndex = rand() % diffuseNames.size();
		pTempModel->ChangeMaterialOfGroup(diffuseNames[randomMatIndex], "ALL");
	}
	m_uOctantLevels = 1;
	m_pEntityMngr->Update();
	
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Update Entity Manager
	m_pRoot = new MyOctant(m_uOctantLevels, 5);
	m_pEntityMngr->Update();

	//Set the model matrix for the main object
	//m_pEntityMngr->SetModelMatrix(m_m4Steve, "Steve");

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
	//m_pEntityMngr->AddEntityToRenderList(-1, true);
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	//display octree
	if (m_uOctantID == -1)
		m_pRoot->Display();
	else
		m_pRoot->Display(m_uOctantID);

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList("Skybox_02.png");

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui,
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//Release MyEntityManager
	MyEntityManager::ReleaseInstance();

	//Release the octree
	SafeDelete(m_pRoot);

	//release GUI
	ShutdownGUI();
}