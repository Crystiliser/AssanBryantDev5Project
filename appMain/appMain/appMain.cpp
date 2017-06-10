// appMain.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "fbxsdk.h"
#include "appMain.h"
#include "Loader.h"
#include "GraphicsSystem.h"
#include <chrono>
#include <vector>

//leak detection
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define MAX_LOADSTRING 100
#define BACKBUFFER_WIDTH 500
#define BACKBUFFER_HEIGHT 500

//My Global Variables
functionLibrary::FBXLoader* theMageLoader = new functionLibrary::FBXLoader("Idle.fbx");
exportFile* magePoseFile = new exportFile;
GraphicsSystem::object* mageMesh = new GraphicsSystem::object;
GraphicsSystem::object* magePose = new GraphicsSystem::object;

functionLibrary::FBXLoader* theTeddyLoader = new functionLibrary::FBXLoader("Teddy_Idle.fbx");
exportFile* teddyPoseFile = new exportFile;
GraphicsSystem::object* teddyMesh = new GraphicsSystem::object;
GraphicsSystem::object* teddyPose = new GraphicsSystem::object;

GraphicsSystem::object* debugObject = new GraphicsSystem::object;
unsigned int debugVertCount = 0;
std::vector<GraphicsSystem::vertex> debugVerts;


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd;

GraphicsSystem graphicsStuff;
GraphicsSystem::pipelineData pipeData;


//camera Stuff
XMFLOAT4X4 camera;
bool rotatingCamera = false;
XMFLOAT2 point = { 0,0 };
XMFLOAT2 prePoint = { 0,0 };

//timer stuff

using time_point_t = decltype(std::chrono::high_resolution_clock::now());

time_point_t last_time;
time_point_t curr_time;
double accum_time{ 0.0 };

float moveSpeed = 5.0f;
float rotSpd = 0.5f;

//Functions

void setupMeshData(functionLibrary::FBXLoader* theLoader, GraphicsSystem::object* theMesh, XMMATRIX* perspectiveMatrix, XMVECTOR eye, XMVECTOR at, XMVECTOR up)
{
	theMesh->topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//move vertex Data over to object for drawing
	theMesh->theObject = new GraphicsSystem::vertex[theLoader->theData.uniqueVerticeCount];
	theMesh->vertexCount = theLoader->theData.uniqueVerticeCount;
	for (unsigned int i = 0; i < theLoader->theData.uniqueVerticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(Purple);

		temp.position.x = theLoader->theData.myData[i].position.x;
		temp.position.y = theLoader->theData.myData[i].position.y;
		temp.position.z = theLoader->theData.myData[i].position.z;
		temp.position.w = theLoader->theData.myData[i].position.w;


		theMesh->theObject[i] = temp;
	}

	//move index data over to object to draw
	theMesh->indexCount = theLoader->theData.indexCount;
	theMesh->indices = new unsigned int[theMesh->indexCount];
	for (unsigned int i = 0; i < theMesh->indexCount; i++)
	{
		theMesh->indices[i] = theLoader->theData.indicies[i];
	}

	//initialize drawing stuff in pipeline
	XMStoreFloat4x4(&theMesh->theMatrix.projection, XMMatrixTranspose(*perspectiveMatrix));
	XMStoreFloat4x4(&theMesh->theMatrix.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

void setupPoseData(exportFile* theFile, GraphicsSystem::object* thePose,  XMMATRIX* perspectiveMatrix, XMVECTOR eye, XMVECTOR at, XMVECTOR up)
{
	thePose->topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	if (theFile->uniqueVerticeCount % 2 != 0)
	{
		thePose->theObject = new GraphicsSystem::vertex[theFile->uniqueVerticeCount + (theFile->uniqueVerticeCount * 6) + 1];
		thePose->vertexCount = theFile->uniqueVerticeCount + (theFile->uniqueVerticeCount * 6) + 1;
	}
	else
	{
		thePose->theObject = new GraphicsSystem::vertex[theFile->uniqueVerticeCount + (theFile->uniqueVerticeCount * 6)];
		thePose->vertexCount = theFile->uniqueVerticeCount + (theFile->uniqueVerticeCount * 6);
	}

	for (unsigned int i = 0; i < theFile->uniqueVerticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(White);

		temp.position.x = theFile->myData[i].position.x;
		temp.position.y = theFile->myData[i].position.y;
		temp.position.z = theFile->myData[i].position.z;
		temp.position.w = theFile->myData[i].position.w;

		thePose->theObject[i] = temp;
		//debugVerts.push_back(temp);
	}

	XMStoreFloat4x4(&thePose->theMatrix.projection, XMMatrixTranspose(*perspectiveMatrix));
	XMStoreFloat4x4(&thePose->theMatrix.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));


#if 1
	unsigned int endPoint = theFile->uniqueVerticeCount;
	unsigned int isOdd = 0;
	if (theFile->uniqueVerticeCount % 2 != 0)
	{
		GraphicsSystem::vertex bufferVert;
		bufferVert.position = thePose->theObject[theFile->uniqueVerticeCount - 1 + debugVertCount].position;
		bufferVert.color = XMFLOAT4(White);
		thePose->theObject[theFile->uniqueVerticeCount] = bufferVert;
		endPoint++;
		isOdd++;
	}

	for (unsigned int i = 0 /*+ debugVertCount*/; i < (endPoint - 1)/* + debugVertCount*/; i++)
	{

		GraphicsSystem::vertex xAxis1;
		xAxis1.color = XMFLOAT4(Red);
		xAxis1.position.x = thePose->theObject[i].position.x;
		xAxis1.position.y = thePose->theObject[i].position.y;
		xAxis1.position.z = thePose->theObject[i].position.z;
		xAxis1.position.w = thePose->theObject[i].position.w;
		thePose->theObject[(i * 6) + theFile->uniqueVerticeCount + isOdd] = xAxis1;
		//debugVerts.push_back(xAxis1);

		GraphicsSystem::vertex xAxis2;
		xAxis2.color = XMFLOAT4(Red);
		xAxis2.position.x = thePose->theObject[i].position.x + 0.1;
		xAxis2.position.y = thePose->theObject[i].position.y;
		xAxis2.position.z = thePose->theObject[i].position.z;
		xAxis2.position.w = thePose->theObject[i].position.w;
		thePose->theObject[(i * 6) + theFile->uniqueVerticeCount + 1 + isOdd] = xAxis2;
		//debugVerts.push_back(xAxis2);

		GraphicsSystem::vertex yAxis1;
		yAxis1.color = XMFLOAT4(Green);
		yAxis1.position.x = thePose->theObject[i].position.x;
		yAxis1.position.y = thePose->theObject[i].position.y;
		yAxis1.position.z = thePose->theObject[i].position.z;
		yAxis1.position.w = thePose->theObject[i].position.w;
		thePose->theObject[(i * 6) + theFile->uniqueVerticeCount + 2 + isOdd] = yAxis1;
		//debugVerts.push_back(yAxis1);

		GraphicsSystem::vertex yAxis2;
		yAxis2.color = XMFLOAT4(Green);
		yAxis2.position.x = thePose->theObject[i].position.x;
		yAxis2.position.y = thePose->theObject[i].position.y + 0.1;
		yAxis2.position.z = thePose->theObject[i].position.z;
		yAxis2.position.w = thePose->theObject[i].position.w;
		thePose->theObject[(i * 6) + theFile->uniqueVerticeCount + 3 + isOdd] = yAxis2;
		//debugVerts.push_back(yAxis2);

		GraphicsSystem::vertex zAxis1;
		zAxis1.color = XMFLOAT4(Blue);
		zAxis1.position.x = thePose->theObject[i].position.x;
		zAxis1.position.y = thePose->theObject[i].position.y;
		zAxis1.position.z = thePose->theObject[i].position.z;
		zAxis1.position.w = thePose->theObject[i].position.w;
		thePose->theObject[(i * 6) + theFile->uniqueVerticeCount + 4 + isOdd] = zAxis1;
		//debugVerts.push_back(zAxis1);

		GraphicsSystem::vertex zAxis2;
		zAxis2.color = XMFLOAT4(Blue);
		zAxis2.position.x = thePose->theObject[i].position.x;
		zAxis2.position.y = thePose->theObject[i].position.y;
		zAxis2.position.z = thePose->theObject[i].position.z + 0.1;
		zAxis2.position.w = thePose->theObject[i].position.w;
		thePose->theObject[(i * 6) + theFile->uniqueVerticeCount + 5 + isOdd] = zAxis2;
		//debugVerts.push_back(zAxis2);
	}
	//debugVertCount += debugVerts.size() - 1;
#endif

}



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);



	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_APPMAIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	//import and save mage stuff
	theMageLoader->importer();
	theMageLoader->save();
	theMageLoader->savePose(magePoseFile);

	//import and save Teddy stuff
	theTeddyLoader->importer();
	theTeddyLoader->save();
	theTeddyLoader->savePose(teddyPoseFile);


#ifndef NDEBUG
	AllocConsole();

	FILE* new_std_in_out;
	freopen_s(&new_std_in_out, "CONOUT$", "w", stdout);
	freopen_s(&new_std_in_out, "CONIN$", "r", stdin);
#if 0


	std::cout << "Vertice Count is: " << theMageLoader->theData.uniqueVerticeCount << "\n";
	for (unsigned int i = 0; i < theMageLoader->theData.uniqueVerticeCount; i++)
	{
		std::cout << "Vertice: " << i << "\n";
		std::cout << "x: " << (theMageLoader->theData.myData[i].position.x) << " "
			<< "y: " << (theMageLoader->theData.myData[i].position.y) << " "
			<< "z: " << (theMageLoader->theData.myData[i].position.z) << " "
			<< "w: " << (theMageLoader->theData.myData[i].position.w) << "\n";
	}
	
	std::cout << "End of vertices\n\n";
	
	std::cout << "Index Count is: " << theMageLoader->theData.indexCount << "\n";
	for (unsigned int i = 0; i < theMageLoader->theData.indexCount; i++)
	{
		std::cout << "Index: " << i << " is: " << theMageLoader->theData.indicies[i] << "\n";
	}
	std::cout << "End of indices";
#endif
#endif

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APPMAIN));

	MSG msg;
	bool running = true;

#pragma region InitCameraStuff
	float aspectRatio = BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT;
	float fovAngleY = 70 * XM_PI / 180.0f;

	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100);


	static const XMVECTORF32 eye = { 0.0f, 0.7f, -1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.1f, 0.0f,0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&camera, XMMatrixInverse(nullptr, XMMatrixLookAtLH(eye, at, up)));
#pragma endregion


	setupMeshData(theMageLoader, mageMesh, &perspectiveMatrix, eye, at, up);
	delete theMageLoader;
	setupPoseData(magePoseFile, magePose, &perspectiveMatrix, eye, at, up);
	delete[] magePoseFile->myData;

	setupMeshData(theTeddyLoader, teddyMesh, &perspectiveMatrix, eye, at, up);
	delete theTeddyLoader;
	setupPoseData(teddyPoseFile, teddyPose, &perspectiveMatrix, eye, at, up);
	delete[] teddyPoseFile->myData;

	//debugObject->theObject = &debugVerts[0];
	//debugObject->vertexCount = debugVerts.size();

	//XMStoreFloat4x4(&debugObject->theMatrix.model, XMMatrixTranspose(XMMatrixTranslation))
	

	//Init model offsets, size, ect
	//Mage
	XMStoreFloat4x4(&mageMesh->theMatrix.model, XMMatrixTranspose(XMMatrixTranslation(-10.0f, 0.0f, 0.0f)));
	XMStoreFloat4x4(&magePose->theMatrix.model, XMMatrixTranspose(XMMatrixTranslation(-10.0f, 0.0f, 0.0f)));

	//Teddy
	XMStoreFloat4x4(&teddyMesh->theMatrix.model, XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(0.0f, 0.0f, 0.0f), XMMatrixScaling(0.2f,0.2f,0.2f))));
	XMStoreFloat4x4(&teddyPose->theMatrix.model, XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslation(0.0f, 0.0f, 0.0f), XMMatrixScaling(0.2f,0.2f,0.2f))));



	graphicsStuff.initOverall(&pipeData, hWnd, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);

	graphicsStuff.basicSetUpIndexBuffer(&pipeData, mageMesh);
	graphicsStuff.basicSetUpInOrderBuffer(&pipeData, magePose);

	graphicsStuff.basicSetUpIndexBuffer(&pipeData, teddyMesh);
	graphicsStuff.basicSetUpInOrderBuffer(&pipeData, teddyPose);


	while (running)
	{
		//timer stuff
		curr_time = std::chrono::high_resolution_clock::now();

		double delta_time = std::chrono::duration<double>(curr_time - last_time).count();

		last_time = curr_time;

		accum_time += delta_time;
		//////////////////////

		XMStoreFloat4x4(&mageMesh->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));
		XMStoreFloat4x4(&magePose->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));

		XMStoreFloat4x4(&teddyMesh->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));
		XMStoreFloat4x4(&teddyPose->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));



		graphicsStuff.setGeneralPipelineStages(&pipeData);

		//mageMesh draw 
		graphicsStuff.setObjectPipelineStages(&pipeData, mageMesh);
		graphicsStuff.drawIndex(&pipeData, mageMesh);

		//magePose draw 
		graphicsStuff.setObjectPipelineStages(&pipeData, magePose);
		graphicsStuff.drawInOrder(&pipeData, magePose);

		//teddyMesh draw
		graphicsStuff.setObjectPipelineStages(&pipeData, teddyMesh);
		graphicsStuff.drawIndex(&pipeData, teddyMesh);

		//teddyPose draw
		graphicsStuff.setObjectPipelineStages(&pipeData, teddyPose);
		graphicsStuff.drawInOrder(&pipeData, teddyPose);


		pipeData.swapchain->Present(1, 0);


		if (rotatingCamera)
		{
			float dx = point.x - prePoint.x;
			float dy = point.y - prePoint.y;

			XMFLOAT4 pos = XMFLOAT4(camera._41, camera._42, camera._43, camera._44);
			camera._41 = 0;
			camera._42 = 0;
			camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * (float)delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * (float)delta_time);

			XMMATRIX tempCam = XMLoadFloat4x4(&camera);
			tempCam = XMMatrixMultiply(rotX, tempCam);
			tempCam = XMMatrixMultiply(tempCam, rotY);

			XMStoreFloat4x4(&camera, tempCam);

			camera._41 = pos.x;
			camera._42 = pos.y;
			camera._43 = pos.z;

		}

		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (msg.message)
			{
			case WM_RBUTTONDOWN:
			{
				rotatingCamera = true;
			}
			break;
			case WM_RBUTTONUP:
			{
				rotatingCamera = false;
			}
			break;
			case WM_MOUSEMOVE:
			{
				XMFLOAT2 pos;
				pos.x = LOWORD(msg.lParam);
				pos.y = HIWORD(msg.lParam);

				prePoint = point;
				point = pos;
			}
			case WM_KEYDOWN:
				switch (msg.wParam)
				{
				case 'W':
				{
					XMMATRIX translation = XMMatrixTranslation(0, 0, moveSpeed * (float)delta_time);
					XMMATRIX tempCam = XMLoadFloat4x4(&camera);
					XMMATRIX result = XMMatrixMultiply(translation, tempCam);
					XMStoreFloat4x4(&camera, result);
				}
					break;
				case 'S':
				{
					XMMATRIX translation = XMMatrixTranslation(0, 0, -moveSpeed * (float)delta_time);
					XMMATRIX tempCam = XMLoadFloat4x4(&camera);
					XMMATRIX result = XMMatrixMultiply(translation, tempCam);
					XMStoreFloat4x4(&camera, result);
				}
					break;
				case 'A':
				{
					XMMATRIX translation = XMMatrixTranslation(-moveSpeed * (float)delta_time, 0, 0);
					XMMATRIX tempCam = XMLoadFloat4x4(&camera);
					XMMATRIX result = XMMatrixMultiply(translation, tempCam);
					XMStoreFloat4x4(&camera, result);
				}
					break;
				case 'D':
				{
					XMMATRIX translation = XMMatrixTranslation(moveSpeed * (float)delta_time, 0, 0);
					XMMATRIX tempCam = XMLoadFloat4x4(&camera);
					XMMATRIX result = XMMatrixMultiply(translation, tempCam);
					XMStoreFloat4x4(&camera, result);
				}
					break;
				default:
					break;
				}
				break;
			case WM_QUIT:
				running = false;
				graphicsStuff.cleanUpPipeLine(&pipeData);

				break;
			default:
				break;
			}
		}
	}

#ifndef NDEBUG
	FreeConsole();
#endif 

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPMAIN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(NULL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, (WS_OVERLAPPEDWINDOW ^ (WS_OVERLAPPEDWINDOW & WS_MAXIMIZEBOX)),
      CW_USEDEFAULT, 0, BACKBUFFER_WIDTH + 16, BACKBUFFER_HEIGHT + 39, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
