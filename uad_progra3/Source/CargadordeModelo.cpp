#include "../Include/CargadordeModelo.h"

#include "../stdafx.h"

#include <iostream> 
#include<fstream>
#include <sstream>
using namespace std;

#include "../Include/Globals.h"

#include "../Include/CWideStringHelper.h"
#include "../Include/CTextureLoader.h"


/* */
CargadordeModelo::CargadordeModelo() :
	CargadordeModelo(CGameWindow::DEFAULT_WINDOW_WIDTH, CGameWindow::DEFAULT_WINDOW_HEIGHT)
{
}

/* */
CargadordeModelo::CargadordeModelo(int window_width, int window_height) :
	CApp(window_width, window_height)
{
	cout << "Constructor: CargadordeModelo(int window_width, int window_height)" << endl;
	RotationSpeed = 90;
	ObjectRotation = 0;
	CurrentTime = 0;
	// Initialize class member variables here
	// ======================================
	//
	// ======================================
}

/* */
CargadordeModelo::~CargadordeModelo()
{
	cout << "Destructor: ~CargadordeModelo()" << endl;

	getOpenGLRenderer()->deleteTexture(&IdentificadorTextura);
	getOpenGLRenderer()->freeGraphicsMemoryForObject(&IdentificadorGeometria);
	// Free memory allocated in this class instance here
	// =================================================
	//
	// =================================================
}

bool CargadordeModelo::LoadFBX(const string& FilePath)
{
	std::ifstream file(FilePath);
	if (!file.is_open())
	{
		std::cerr << "Error no se pudo abrir el archivo FBX" << std::endl;
		return;
	}

	string Line;
	vector<float> Vertices;
	vector <unsigned short > Indices_Vertices;
	vector < float> UVs;
	bool In_Vertices_Section = false;
	bool In_Faces_section = false;
	bool In_UVs_Section = false;

	while (getline(file, Line))
	{
		if (Line.find("vertices:") != string::npos)
		{
			In_Vertices_Section = true;
			In_Faces_section = false;
			In_UVs_Section = false;
			continue;
		}

		if (Line.find("PolygonVertexIndex:") != string::npos)
		{
			In_Faces_section = true;
			In_Vertices_Section = false;
			In_UVs_Section = false;
			continue;
		}
		if (Line.find("UV:") != string::npos)
		{
			In_UVs_Section = true;
			In_Vertices_Section = false;
			In_Faces_section = false;
			continue;
		}

		if (In_Vertices_Section)
		{
			std::stringstream SS(Line);
			{
				float X, Y, Z;
				char Coma;
				while (SS >> X >> Coma >> Y >> Coma >> Z)
				{
					Vertices.push_back(X);
					Vertices.push_back(Y);
					Vertices.push_back(Z);
				}
			}
		}
		if (In_Faces_section)
		{
			std::stringstream SS(Line);
			int Index;
			vector < int > FaceIndex;
			while (SS >> Index)
			{
				FaceIndex.push_back(Index < 0 ? -Index : Index);
				if (Index < 0)
				{
					break;
				}
			}
			if (FaceIndex.size() == 3)
			{
				Indices_Vertices.push_back(FaceIndex[0]);
				Indices_Vertices.push_back(FaceIndex[1]);
				Indices_Vertices.push_back(FaceIndex[2]);
			}
			else if (FaceIndex.size() == 4)
			{
				Indices_Vertices.push_back(FaceIndex[0]);
				Indices_Vertices.push_back(FaceIndex[1]);
				Indices_Vertices.push_back(FaceIndex[2]);

				Indices_Vertices.push_back(FaceIndex[0]);
				Indices_Vertices.push_back(FaceIndex[2]);
				Indices_Vertices.push_back(FaceIndex[3]);
			}
			else
			{
				std::cerr << "Error, Ngon en: " << FaceIndex.size() << " vertices" << std::endl;
			}

			if (In_UVs_Section)
			{
				stringstream SS(Line);
				float U, V;
				char Coma;
				while (SS >> U >> Coma >> V)
				{
					UVs.push_back(U);
					UVs.push_back(V);
				}
			}
		}
	}
	file.close();

	if (fbxVertices.empty() || fbxIndicesVertices.empty()) {
		std::cerr << "Error: No se pudieron cargar los datos del archivo .fbx" << std::endl;
		return false;
	}

	return true;
}

/* */
void CargadordeModelo::initialize()
{
	std::string fbxFilePath = "File path";
	std::string textureFilePath;
	std::wstring wTexturePack;

	
	bool fbxLoaded = LoadFBX(fbxFilePath);
	if (!fbxLoaded)
	{
		std::cout << "No se pudo cargar el archivo .fbx" << std::endl;
		return;
	}

	
	if (!CWideStringHelper::GetResourceFullPath(MC_GRASS_2_DIRT_TEXTURE, wTexturePack, textureFilePath))
	{
		std::cout << "No se pudo cargar la textura" << std::endl;
		return;
	}


	newModelShaderId = getOpenGLRenderer()->getShaderProgramID(SHADER_PROGRAM_TEXTURED_OBJECT);

	
	CTextureLoader::loadTexture(textureFilePath.c_str(), &IdentificadorTextura, getOpenGLRenderer());


	bool success = getOpenGLRenderer()->allocateGraphicsMemoryForObject(
		&newModelShaderId,
		&IdentificadorGeometria,
		fbxVertices.data(),
		fbxVertices.size() / 3,
		fbxNormales.data(),
		fbxNormales.size() / 3,
		fbxUVs.data(),
		fbxUVs.size() / 2,
		fbxIndicesVertices.data(),
		fbxIndicesVertices.size() / 3,
		fbxIndicesNormales.data(),
		fbxIndicesNormales.size() / 3,
		fbxIndicesUVs.data(),
		fbxIndicesUVs.size() / 3
	);

	if (!success)
	{
		std::cout << "Fallo al asignar memoria gráfica para el objeto" << std::endl;
	}

	/*
	float vertices[12] = 
	{
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f,  -1.0f, 0.0f,
		 1.0f, 1.0f, 0.0f
	};

	unsigned short indicesVertices[6] = 
	{   //NUM TRIANGULOS * NUM VERTICES EN CADA TRIANGULO
		0, 1, 3,
		3, 1, 2
	};

	float normales[3] =
	{
		0.0f, 0.0f, 1.0f
	};

	unsigned short indicesNormales[6] =
	{ // NUM TRIANGULOS * NUM NORMALES EN CADA TRIANGULO
			0, 0, 0,
			0, 0, 0
	};

	float UVs[12] = 
	{     // NUM TRIANGULOS * NUM VERTICES EN CADA TRIANGULO * NUM COMPONENTES (U, V)
		 0.25f, 0.25f,
		 0.25f, 0.75f,
		 0.75f, 0.25f,
		 0.25f, 0.9f,
		 0.9f, 0.9f,
		 0.9f, 0.25
	};
	unsigned short indicesUVs[6] = 
	{     // NUM TRIANGULOS * NUM UVs EN CADA TRIANGULO
	0, 1, 2,
	5, 3, 4
	};

	std::string TexturePath;
	std::wstring WTexturePack;
	if (!CWideStringHelper::GetResourceFullPath(MC_GRASS_2_DIRT_TEXTURE, WTexturePack, TexturePath))
	{
		std::cout << "No se pudo cargar la textura" << std::endl;
		return;
	}
	newModelShaderId = getOpenGLRenderer()->getShaderProgramID(SHADER_PROGRAM_TEXTURED_OBJECT);
	CTextureLoader::loadTexture(TexturePath.c_str(), &IdentificadorTextura, getOpenGLRenderer());
	bool Succes = getOpenGLRenderer()->allocateGraphicsMemoryForObject(&newModelShaderId, &IdentificadorGeometria, &vertices[0], 4, &normales[0], 1, &UVs[0], 6, &indicesVertices[0], 2, &indicesNormales[0], 2, &indicesUVs[0], 2);
	*/
}


void CargadordeModelo::run()
{
	// Check if CGameWindow object AND Graphics API specific Window library have been initialized
	if (canRun())
	{
		// Create the Window 
		// Note: The following create() method creates the Window itself. Do NOT make any OpenGL calls before this line, your app WILL crash.
		if (getGameWindow()->create(CAPP_PROGRA3_EMPTYAPP_WINDOW_TITLE))
		{
			initialize();

			// Set initial clear screen color
			getOpenGLRenderer()->setClearScreenColor(0.25f, 0.0f, 0.75f);

			// Enter main loop
			cout << "Entering Main loop" << endl;
			getGameWindow()->mainLoop(this);
		}
	}
}

/* */
void CargadordeModelo::update(double deltaTime)
{
	// Do not update if delta time is < 0
	if (deltaTime <= 0.0f)
	{
		return;
	}

	CurrentTime = deltaTime;

	ObjectRotation += RotationSpeed * (CurrentTime / 1000);

	if (ObjectRotation > 360)
	{
		ObjectRotation -= 360;
	}

	// Update app-specific stuff here
	// ===============================
	//
	// ===============================
}

/* */
void CargadordeModelo::render()
{
	CGameMenu* menu = getMenu();

	// If menu is active, render menu
	if (menu != NULL
		&& menu->isInitialized()
		&& menu->isActive())
	{
		//...
	}
	else // Otherwise, render app-specific stuff here...
	{
		// White 
				// Colors are in the 0..1 range, if you want to use RGB, use (R/255, G/255, G/255)
		float color[3] = { 1.0f, 1.0f, 1.0f };
		CVector3 objectPosition;
			// Get a matrix that has both the object rotation and translation
		double DegreestoRadians = ObjectRotation * 3.1416 / 180;
			MathHelper::Matrix4 modelMatrix = MathHelper::SimpleModelMatrixRotationTranslation((float) DegreestoRadians, objectPosition);


			getOpenGLRenderer()->renderObject(
				&newModelShaderId,
				&IdentificadorGeometria,
				&IdentificadorTextura,
				2,
				color,
				&modelMatrix,
				COpenGLRenderer::EPRIMITIVE_MODE::TRIANGLES,
				false);
		
	}
}

/* */
void CargadordeModelo::onMouseMove(float deltaX, float deltaY)
{
	// Update app-specific stuff when mouse moves here 
	// ===============================================
	//
	// ===============================================
}

/* */
void CargadordeModelo::executeMenuAction()
{
	if (getMenu() != NULL)
	{
		// Execute app-specific menu actions here
		// ======================================
		//
		// ======================================
	}
}