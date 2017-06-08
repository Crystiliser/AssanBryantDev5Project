// appMain.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "fbxsdk.h"
#include "appMain.h"
#include "Loader.h"
#include "GraphicsSystem.h"
#include <chrono>

//leak detection
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define MAX_LOADSTRING 100
#define BACKBUFFER_WIDTH 500
#define BACKBUFFER_HEIGHT 500

//My Global Variables
functionLibrary::FBXLoader* theLoader = new functionLibrary::FBXLoader("Idle.fbx");
exportFile poseFile;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd;

GraphicsSystem graphicsStuff;
GraphicsSystem::pipelineData pipeData;
GraphicsSystem::object* mesh = new GraphicsSystem::object;
GraphicsSystem::object* pose = new GraphicsSystem::object;

//debug stuff
GraphicsSystem::object debugger;
unsigned int count;

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


	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_APPMAIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	theLoader->importer();
	theLoader->save();
	theLoader->savePose(&poseFile);
#ifndef NDEBUG
	AllocConsole();

	FILE* new_std_in_out;
	freopen_s(&new_std_in_out, "CONOUT$", "w", stdout);
	freopen_s(&new_std_in_out, "CONIN$", "r", stdin);
#if 0


	std::cout << "Vertice Count is: " << theLoader->theData.uniqueVerticeCount << "\n";
	for (unsigned int i = 0; i < theLoader->theData.uniqueVerticeCount; i++)
	{
		std::cout << "Vertice: " << i << "\n";
		std::cout << "x: " << (theLoader->theData.myData[i].position.x) << " "
			<< "y: " << (theLoader->theData.myData[i].position.y) << " "
			<< "z: " << (theLoader->theData.myData[i].position.z) << " "
			<< "w: " << (theLoader->theData.myData[i].position.w) << "\n";
	}
	
	std::cout << "End of vertices\n\n";
	
	std::cout << "Index Count is: " << theLoader->theData.indexCount << "\n";
	for (unsigned int i = 0; i < theLoader->theData.indexCount; i++)
	{
		std::cout << "Index: " << i << " is: " << theLoader->theData.indicies[i] << "\n";
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

	XMMATRIX perspective = graphicsStuff.perspectiveProjection(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);
#pragma endregion

#pragma region SetupPoseDrawData
	pose->topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

	pose->theObject = new GraphicsSystem::vertex[poseFile.uniqueVerticeCount];
	pose->vertexCount = poseFile.uniqueVerticeCount;
	for (unsigned int i = 0; i < poseFile.uniqueVerticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(Blue);

		temp.position.x = poseFile.myData[i].position.x;
		temp.position.y = poseFile.myData[i].position.y;
		temp.position.z = poseFile.myData[i].position.z;
		temp.position.w = poseFile.myData[i].position.w;

		pose->theObject[i] = temp;
	}

	pose->indexCount = poseFile.indexCount;
	pose->indices = new unsigned int[poseFile.indexCount];
	for (unsigned int i = 0; i < poseFile.indexCount; i++)
	{
		pose->indices[i] = poseFile.indicies[i];

	}

	XMStoreFloat4x4(&pose->theMatrix.projection, XMMatrixTranspose(perspectiveMatrix));
	XMStoreFloat4x4(&pose->theMatrix.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
#pragma endregion

#pragma region SetupMeshDrawData
	mesh->topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	//move vertex Data over to object for drawing
	mesh->theObject = new GraphicsSystem::vertex[theLoader->theData.uniqueVerticeCount];
	mesh->vertexCount = theLoader->theData.uniqueVerticeCount;
	for (unsigned int i = 0; i < theLoader->theData.uniqueVerticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(Red);

		temp.position.x = theLoader->theData.myData[i].position.x;
		temp.position.y = theLoader->theData.myData[i].position.y;
		temp.position.z = theLoader->theData.myData[i].position.z;
		temp.position.w = theLoader->theData.myData[i].position.w;


		mesh->theObject[i] = temp;
	}

	//move index data over to object to draw
	mesh->indexCount = theLoader->theData.indexCount;
	mesh->indices = new unsigned int[mesh->indexCount];
	for (unsigned int i = 0; i < mesh->indexCount; i++)
	{
		mesh->indices[i] = theLoader->theData.indicies[i];
	}

	delete theLoader;


	//initialize drawing stuff in pipeline
	XMStoreFloat4x4(&mesh->theMatrix.projection, XMMatrixTranspose(perspectiveMatrix));
	XMStoreFloat4x4(&mesh->theMatrix.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
#pragma endregion


	graphicsStuff.initOverall(&pipeData, hWnd, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);

	graphicsStuff.basicSetUpBuffer(&pipeData, mesh);
	graphicsStuff.basicSetUpBuffer(&pipeData, pose);


	while (running)
	{
		//timer stuff
		curr_time = std::chrono::high_resolution_clock::now();

		double delta_time = std::chrono::duration<double>(curr_time - last_time).count();

		last_time = curr_time;

		accum_time += delta_time;
		//////////////////////

		XMStoreFloat4x4(&mesh->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));
		XMStoreFloat4x4(&pose->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));

		graphicsStuff.setGeneralPipelineStages(&pipeData);

		//Mesh draw 
		graphicsStuff.setObjectPipelineStages(&pipeData, mesh);

		graphicsStuff.drawIndex(&pipeData, mesh);

		//Pose draw 
		graphicsStuff.setObjectPipelineStages(&pipeData, pose);

		graphicsStuff.drawInOrder(&pipeData, pose);


		pipeData.swapchain->Present(1, 0);


		if (rotatingCamera)
		{
			float dx = point.x - prePoint.x;
			float dy = point.y - prePoint.y;

			XMFLOAT4 pos = XMFLOAT4(camera._41, camera._42, camera._43, camera._44);
			camera._41 = 0;
			camera._42 = 0;
			camera._43 = 0;

			XMMATRIX rotX = XMMatrixRotationX(dy * rotSpd * delta_time);
			XMMATRIX rotY = XMMatrixRotationY(dx * rotSpd * delta_time);

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
					XMMATRIX translation = XMMatrixTranslation(0, 0, moveSpeed * delta_time);
					XMMATRIX tempCam = XMLoadFloat4x4(&camera);
					XMMATRIX result = XMMatrixMultiply(translation, tempCam);
					XMStoreFloat4x4(&camera, result);
				}
					break;
				case 'S':
				{
					XMMATRIX translation = XMMatrixTranslation(0, 0, -moveSpeed * delta_time);
					XMMATRIX tempCam = XMLoadFloat4x4(&camera);
					XMMATRIX result = XMMatrixMultiply(translation, tempCam);
					XMStoreFloat4x4(&camera, result);
				}
					break;
				case 'A':
				{
					XMMATRIX translation = XMMatrixTranslation(-moveSpeed * delta_time, 0, 0);
					XMMATRIX tempCam = XMLoadFloat4x4(&camera);
					XMMATRIX result = XMMatrixMultiply(translation, tempCam);
					XMStoreFloat4x4(&camera, result);
				}
					break;
				case 'D':
				{
					XMMATRIX translation = XMMatrixTranslation(moveSpeed * delta_time, 0, 0);
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
