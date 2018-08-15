#include "AppClass.h"

using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUp(
		vector3(0.0f, 10.0f, -5.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light

	// create the square field
	uint uInstances = 49;
	int nSquare = static_cast<int>(std::sqrt(uInstances));
	m_uObjects = nSquare * nSquare;
	uint uIndex = -1;

	for (int i = 0; i < nSquare; i++)
	{
		for (int j = 0; j < nSquare; j++)
		{
			uIndex++;
			m_pEntityMngr->AddEntity("Minecraft\\Cube.obj");

			vector3 v3Position = vector3(i, 0.0f, j);
			nodes[i] = vector4(v3Position, 1.0f);

			matrix4 m4Position = glm::translate(v3Position);
			m_pEntityMngr->SetModelMatrix(m4Position);
		}
	}

	//m_uOctantLevels = 3;
	//m_pRoot = new MyOctant(m_uOctantLevels, 15);
	//m_pEntityMngr->Update();
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
	m_pEntityMngr->Update();

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	//display octree
	//m_pRoot->Display();
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
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
	//release GUI
	ShutdownGUI();
}


// A* Code
// use a map for both storing the previous node visited and the cost of movement
void ShortestPath(vector3 start, vector3 goal, std::map<vector3, vector3>& prevNode, std::map<vector3, float>& cost)
{
	// create a priority queue to determine the path
	std::priority_queue<vector3, float> path;
	path.push(start);	// set the starting position as the first entry

	prevNode[start] = start;
	cost[start] = 0;

	while (path.empty() == false)
	{
		vector3 current;

		// if the X and Z components are the same, then the goal has been reached
		if (current.x == goal.x && current.z == goal.z) {
			// ---collision resolution---
			return;
		}

		// go through each neighboring node via edges
		/*
		for ()
		{
			float newCost = cost[current] + graph.cost(current, next);

			// if the new calculated total cost is lower, then add this node to the queue
			if (cost.find(next) == cost.end() || newCost < cost[next])
			{
				cost[next] = newCost;
				float priority = newCost + FindDistance(next, goal);
				path.push(next);
				prevNode[next] = current;
			}
		}*/

	}
	
}

// Manhattan Distance -- find the distance between the current position and the goal's position
// uses X and Z components since Y is fixed at 1.0f
float FindDistance(vector3 currentPos, vector3 goalPos)
{
	return std::abs(currentPos.x - goalPos.x) + std::abs(currentPos.z - goalPos.z);
}