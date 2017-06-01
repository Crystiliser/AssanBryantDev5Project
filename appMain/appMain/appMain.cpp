// appMain.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "fbxsdk.h"
#include "appMain.h"
#include "Loader.h"
#include "GraphicsSystem.h"

#define MAX_LOADSTRING 100
#define BACKBUFFER_WIDTH 500
#define BACKBUFFER_HEIGHT 500

//My Global Variables
functionLibrary::FBXLoader theLoader("BattleMagewithRigandtextures.fbx");
exportFile* theData = new exportFile;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd;

GraphicsSystem graphicsStuff;
GraphicsSystem::pipelineData pipeData;
GraphicsSystem::vertex* lines; 
GraphicsSystem::vertex* object;
XMMATRIX perspective;

float timer = 0;

float moveSpeed = 3;
XMFLOAT4X4 camera;


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
#ifndef NDEBUG
	theLoader.importer();
	theData = &theLoader.saver();
	AllocConsole();

	FILE* new_std_in_out;
	freopen_s(&new_std_in_out, "CONOUT$", "w", stdout);
	freopen_s(&new_std_in_out, "CONIN$", "r", stdin);

//	std::cout << "Vertice Count is: " << theData->verticeCount << "\n";
//	for (unsigned int i = 0; i < theData->verticeCount; i++)
//	{
//		std::cout << "Vertice: " << i << "\n";
//		std::cout << "x: " << (theData->myData[i].position.x) << " "
//			<< "y: " << (theData->myData[i].position.y) << " "
//			<< "z: " << (theData->myData[i].position.z) << " "
//			<< "w: " << (theData->myData[i].position.w) << "\n";
//	}
//
//	std::cout << "End of vertices";
#endif

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APPMAIN));

	MSG msg;
	bool running = true;

#pragma region SetupDrawData
	pipeData.topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	perspective = graphicsStuff.perspectiveProjection(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);
	object = new GraphicsSystem::vertex[theData->verticeCount];
	for (int i = 0; i < theData->verticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(Red);

		temp.position.x = theData->myData[i].position.x;
		temp.position.y = theData->myData[i].position.y;
		temp.position.z = theData->myData[i].position.z;
		temp.position.w = theData->myData[i].position.w;


		object[i] = temp;
	}

	graphicsStuff.initOverall(&pipeData, hWnd, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, object, theData->verticeCount);


	pipeData.topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	perspective = graphicsStuff.perspectiveProjection(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);
	lines = new GraphicsSystem::vertex[theData->verticeCount];
	for (int i = 0; i < theData->verticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(Red);

		temp.position.x = 0;
		temp.position.y = 0;
		temp.position.z = 0;
		temp.position.w = 0;

		XMStoreFloat4(&temp.position, XMVector4Transform(XMLoadFloat4(
			&temp.position), perspective));


		lines[i] = temp;
	}
#pragma endregion

	while (running)
	{

			graphicsStuff.setPipelinesStages(&pipeData);
			graphicsStuff.draw(&pipeData, lines, sizeof(GraphicsSystem::vertex) 
				* theData->verticeCount, theData->verticeCount);

		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_KEYDOWN)
			{
				XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, -moveSpeed * timer);
				XMMATRIX tempCam = XMLoadFloat4x4(&camera);
				XMMATRIX result = XMMatrixMultiply(translation, tempCam);
				XMStoreFloat4x4(&camera, result);
			}
			if (msg.message == WM_QUIT)
			{
				running = false;
#pragma region pipeline cleanup
				pipeData.constantBuffer->Release();
				pipeData.depthStencilBuffer->Release();
				pipeData.depthStencilState->Release();
				pipeData.depthStencilView->Release();
				pipeData.dev->Release();
				pipeData.devcon->Release();
				pipeData.inputLayout->Release();
				pipeData.pixelShader->Release();
			//	pipeDatate.rasterState->Release();
				pipeData.renderTarget->Release();
				pipeData.swapchain->Release();
				pipeData.vertexBuffer->Release();
				pipeData.vertexShader->Release();
#pragma endregion

			}
		}

		timer += 0.01f;
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
