#include "AppClass.h"
void Application::InitVariables(void)
{
	//Change this to your name and email
	m_sProgrammer = "Catherine Nemechek - crn4802@rit.edu";
	
	//Set the position and target of the camera
	//(I'm at [0,0,10], looking at [0,0,0] and up is the positive Y axis)
	m_pCameraMngr->SetPositionTargetAndUp(AXIS_Z * 20.0f, ZERO_V3, AXIS_Y);

	//if the light position is zero move it
	if (m_pLightMngr->GetPosition(1) == ZERO_V3)
		m_pLightMngr->SetPosition(vector3(0.0f, 0.0f, 3.0f));

	//if the background is cornflowerblue change it to black (its easier to see)
	if (vector3(m_v4ClearColor) == C_BLUE_CORNFLOWER)
	{
		m_v4ClearColor = vector4(ZERO_V3, 1.0f);
	}
	
	//if there are no segments create 7
	if(m_uOrbits < 1)
		m_uOrbits = 7;

	float fSize = 1.0f; //initial size of orbits

	//creating a color using the spectrum 
	uint uColor = 650; //650 is Red
	//prevent division by 0
	float decrements = 250.0f / (m_uOrbits > 1? static_cast<float>(m_uOrbits - 1) : 1.0f); //decrement until you get to 400 (which is violet)
	/*
		This part will create the orbits, it start at 3 because that is the minimum subdivisions a torus can have
	*/
	uint uSides = 3; //start with the minimal 3 sides
	for (uint i = uSides; i < m_uOrbits + uSides; i++)
	{
		vector3 v3Color = WaveLengthToRGB(uColor); //calculate color based on wavelength
		m_shapeList.push_back(m_pMeshMngr->GenerateTorus(fSize, fSize - 0.1f, 3, i, v3Color)); //generate a custom torus and add it to the meshmanager
		fSize += 0.5f; //increment the size for the next orbit
		uColor -= static_cast<uint>(decrements); //decrease the wavelength
	}

	// New Code
	// Initialize the route and time values and set size to the orbits
	for (int i = 0; i < m_uOrbits; ++i)
	{
		m_routes.push_back(0);
		m_time.push_back(0.0f);
	}

	// radius multiplier for each orbit
	float radius = 0.95f;

	// initialize the stops
	for (int i = uSides; i < m_uOrbits + uSides; ++i)
	{
		// angle
		float theta = glm::radians(360.0f / i);

		// create the list of stops
		std::vector <vector3> v3stops;

		for (int j = 0; j < i; ++j)
		{
			// add the stops
			vector3 stop(glm::cos(j * theta) * radius, glm::sin(j * theta) * radius, 0);
			v3stops.push_back(stop);
		}

		// add this to the list of stops
		m_stopsList.push_back(v3stops);

		// increment size for the following orbit
		radius += 0.5f;
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

	matrix4 m4View = m_pCameraMngr->GetViewMatrix(); //view Matrix
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix(); //Projection Matrix
	matrix4 m4Offset = IDENTITY_M4; //offset of the orbits, starts as the global coordinate system
	/*
		The following offset will orient the orbits as in the demo, start without it to make your life easier.
	*/
	m4Offset = glm::rotate(IDENTITY_M4, 1.5708f, AXIS_Z);

	// TIME
	// increment the time
	for (int i = 0; i < m_uOrbits; i++)
	{
		m_time[i] += 0.01f;
	}

	// draw a shapes
	for (uint i = 0; i < m_uOrbits; ++i)
	{
		m_pMeshMngr->AddMeshToRenderList(m_shapeList[i], glm::rotate(m4Offset, 1.5708f, AXIS_X));

	#pragma region LERPing Code
		// Set the starting and ending positions
		vector3 v3StartPos;
		vector3 v3EndPos;
		v3StartPos = m_stopsList[i][m_routes[i]];
		v3EndPos = m_stopsList[i][(m_routes[i] + 1) % m_stopsList[i].size()];

		// Get the percentage
		float fTimeBetween = 0.3f;
		float fPercent = MapValue(m_time[i], 0.0f, fTimeBetween, 0.0f, 1.0f);

		// Calculate the current position
		vector3 v3CurrentPos = glm::lerp(v3StartPos, v3EndPos, fPercent);
		matrix4 m4Model = glm::translate(m4Offset, v3CurrentPos);

		// Change the route
		if (fPercent >= 1.0f)
		{
			m_routes[i]++;
			m_time[i] = 0.0f;
			m_routes[i] %= m_stopsList[i].size();
		}
	#pragma endregion

		//draw spheres
		m_pMeshMngr->AddSphereToRenderList(m4Model * glm::scale(vector3(0.1)), C_WHITE);
	}

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
	//release GUI
	ShutdownGUI();
}