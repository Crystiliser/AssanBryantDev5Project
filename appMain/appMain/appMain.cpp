// appMain.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "fbxsdk.h"
#include "appMain.h"
#include "Loader.h"
#include "GraphicsSystem.h"
#include <chrono>
#include <vector>


#define MAX_LOADSTRING 100
#define BACKBUFFER_WIDTH 500
#define BACKBUFFER_HEIGHT 500

//My Global Variables
functionLibrary::FBXLoader* theMageLoader = new functionLibrary::FBXLoader("Idle.fbx");
exportFile* mageMeshFile = new exportFile;
exportFile* magePoseFile = new exportFile;
GraphicsSystem::object* mageMesh = new GraphicsSystem::object;

functionLibrary::FBXLoader* theTeddyLoader = new functionLibrary::FBXLoader("Teddy_Run.fbx");
exportFile* teddyMeshFile = new exportFile;
exportFile* teddyPoseFile = new exportFile;
GraphicsSystem::object* teddyMesh = new GraphicsSystem::object;

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
float moveSpeed = 5.0f;
float rotSpd = 0.5f;

//timer stuff

using time_point_t = decltype(std::chrono::high_resolution_clock::now());

time_point_t last_time;
time_point_t curr_time;
double accum_time{ 0.0 };

//animatino stuff
bool playingAnim = false;
bool playingTween = false;
double timeMultiplier = 100;


//Functions

void setupMeshData(exportFile* theFile, GraphicsSystem::object* theMesh, XMMATRIX* perspectiveMatrix, XMVECTOR eye, XMVECTOR at, XMVECTOR up)
{
	theMesh->topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//move vertex Data over to object for drawing
	theMesh->theObject = new GraphicsSystem::vertex[theFile->uniqueVerticeCount];
	theMesh->vertexCount = theFile->uniqueVerticeCount;
	for (unsigned int i = 0; i < theFile->uniqueVerticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(Purple);

		temp.position.x = theFile->myData[i].position.x;
		temp.position.y = theFile->myData[i].position.y;
		temp.position.z = theFile->myData[i].position.z;
		temp.position.w = theFile->myData[i].position.w;


		theMesh->theObject[i] = temp;
	}

	//move index data over to object to draw
	theMesh->indexCount = theFile->indexCount;
	theMesh->indices = new unsigned int[theMesh->indexCount];
	for (unsigned int i = 0; i < theMesh->indexCount; i++)
	{
		theMesh->indices[i] = theFile->indicies[i];
	}

	//initialize drawing stuff in pipeline
	XMStoreFloat4x4(&theMesh->theMatrix.projection, XMMatrixTranspose(*perspectiveMatrix));
	XMStoreFloat4x4(&theMesh->theMatrix.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
}

void setupDebugPoseData(exportFile* theFile, GraphicsSystem::object* theMesh)
{
	theMesh->actualBonesCount = theFile->uniqueVerticeCount;

	for (unsigned int i = 0; i < theFile->uniqueVerticeCount; i++)
	{
		GraphicsSystem::vertex temp;
		temp.color = XMFLOAT4(White);

		temp.position.x = theFile->myData[i].position.x;
		temp.position.y = theFile->myData[i].position.y;
		temp.position.z = theFile->myData[i].position.z;
		temp.position.w = theFile->myData[i].position.w;

		theMesh->bones.push_back(temp);
	}


#if 1
	unsigned int endPoint = theFile->uniqueVerticeCount;
	if (theFile->uniqueVerticeCount % 2 != 0)
	{
		GraphicsSystem::vertex bufferVert;
		bufferVert.position = theMesh->bones[theFile->uniqueVerticeCount - 1].position;
		bufferVert.color = XMFLOAT4(White);
		theMesh->bones.push_back(bufferVert);
		endPoint++;
	}

	for (unsigned int i = 0; i < (endPoint - 1); i++)
	{

		GraphicsSystem::vertex xAxis1;
		xAxis1.color = XMFLOAT4(Red);
		xAxis1.position.x = theMesh->bones[i].position.x;
		xAxis1.position.y = theMesh->bones[i].position.y;
		xAxis1.position.z = theMesh->bones[i].position.z;
		xAxis1.position.w = theMesh->bones[i].position.w;
		theMesh->bones.push_back(xAxis1);

		GraphicsSystem::vertex xAxis2;
		xAxis2.color = XMFLOAT4(Red);
		xAxis2.position.x = theMesh->bones[i].position.x + 0.1f;
		xAxis2.position.y = theMesh->bones[i].position.y;
		xAxis2.position.z = theMesh->bones[i].position.z;
		xAxis2.position.w = theMesh->bones[i].position.w;
		theMesh->bones.push_back(xAxis2);

		GraphicsSystem::vertex yAxis1;
		yAxis1.color = XMFLOAT4(Green);
		yAxis1.position.x = theMesh->bones[i].position.x;
		yAxis1.position.y = theMesh->bones[i].position.y;
		yAxis1.position.z = theMesh->bones[i].position.z;
		yAxis1.position.w = theMesh->bones[i].position.w;
		theMesh->bones.push_back(yAxis1);

		GraphicsSystem::vertex yAxis2;
		yAxis2.color = XMFLOAT4(Green);
		yAxis2.position.x = theMesh->bones[i].position.x;
		yAxis2.position.y = theMesh->bones[i].position.y + 0.1f;
		yAxis2.position.z = theMesh->bones[i].position.z;
		yAxis2.position.w = theMesh->bones[i].position.w;
		theMesh->bones.push_back(yAxis2);

		GraphicsSystem::vertex zAxis1;
		zAxis1.color = XMFLOAT4(Blue);
		zAxis1.position.x = theMesh->bones[i].position.x;
		zAxis1.position.y = theMesh->bones[i].position.y;
		zAxis1.position.z = theMesh->bones[i].position.z;
		zAxis1.position.w = theMesh->bones[i].position.w;
		theMesh->bones.push_back(zAxis1);

		GraphicsSystem::vertex zAxis2;
		zAxis2.color = XMFLOAT4(Blue);
		zAxis2.position.x = theMesh->bones[i].position.x;
		zAxis2.position.y = theMesh->bones[i].position.y;
		zAxis2.position.z = theMesh->bones[i].position.z + 0.1f;
		zAxis2.position.w = theMesh->bones[i].position.w;
		theMesh->bones.push_back(zAxis2);
	}
	debugVertCount += theMesh->bones.size() - 1;
#endif

}

void setupAnimData(exportFile* theFile, GraphicsSystem::object* theMesh)
{
	theMesh->theAnimation.duration = theFile->theAnimation.duration;
	for (unsigned int i = 0; i < theFile->theAnimation.frames.size(); i++)
	{
		GraphicsSystem::keyframe newFrame;
		newFrame.time = theFile->theAnimation.frames[i].time;
		for (unsigned int j = 0; j < theFile->theAnimation.frames[i].joints.size(); j++)
		{
			XMFLOAT4X4 newJoint;
			float* temp = theFile->theAnimation.frames[i].joints[j];

			newJoint._11 = temp[0];
			newJoint._12 = temp[1];
			newJoint._13 = temp[2];
			newJoint._14 = temp[3];

			newJoint._21 = temp[4];
			newJoint._22 = temp[5];
			newJoint._23 = temp[6];
			newJoint._24 = temp[7];

			newJoint._31 = temp[8];
			newJoint._32 = temp[9];
			newJoint._33 = temp[10];
			newJoint._34 = temp[11];

			newJoint._41 = temp[12];
			newJoint._42 = temp[13];
			newJoint._43 = temp[14];
			newJoint._44 = temp[15];
			newFrame.joints.push_back(newJoint);
		}
		theMesh->theAnimation.frames.push_back(newFrame);
	}
}

void playFrame(GraphicsSystem::object* theMesh)
{
	for (unsigned int i = 0; i < theMesh->actualBonesCount; i++)
	{
		XMFLOAT4 newPos;
		XMFLOAT4X4 posMat = theMesh->theAnimation.frames[theMesh->currentFrame].joints[i];

		newPos.x = posMat._41;
		newPos.y = posMat._42;
		newPos.z = posMat._43;
		newPos.w = 1;
		XMVECTOR currPos = XMLoadFloat4(&newPos);

		XMMATRIX temp = XMLoadFloat4x4(&theMesh->transformMat);

		XMStoreFloat4(&newPos, XMVector3Transform(currPos, temp));

		theMesh->bones[i].position = newPos;
	}
	if (theMesh->actualBonesCount % 2 != 0)
	{
		XMFLOAT4 newPos;
		XMFLOAT4X4 posMat = theMesh->theAnimation.frames[theMesh->currentFrame].joints[theMesh->actualBonesCount - 1];

		newPos.x = posMat._41;
		newPos.y = posMat._42;
		newPos.z = posMat._43;
		newPos.w = 1;
		XMVECTOR currPos = XMLoadFloat4(&newPos);

		XMMATRIX temp = XMLoadFloat4x4(&theMesh->transformMat);

		XMStoreFloat4(&newPos, XMVector3Transform(currPos, temp));

		theMesh->bones[theMesh->actualBonesCount].position = newPos;
	}
}

void playTween(GraphicsSystem::object* theMesh)
{
	GraphicsSystem::keyframe* prevFrame = &theMesh->theAnimation.frames[theMesh->currentFrame];
	GraphicsSystem::keyframe* nextFrame;
	double ratio;

	if (theMesh->currentFrame + 1 < (int)theMesh->theAnimation.frames.size())
	{
		nextFrame = &theMesh->theAnimation.frames[theMesh->currentFrame + 1];
		ratio = theMesh->timePassed / nextFrame->time;
	}
	else
	{
		nextFrame = &theMesh->theAnimation.frames[0];
		ratio = nextFrame->time / theMesh->timePassed;
	}

	
	for (unsigned int i = 0; i < theMesh->actualBonesCount; i++)
	{
		XMFLOAT4 newPos;
		XMFLOAT4X4 prevPosMat = prevFrame->joints[i];
		XMFLOAT4X4 nextPosMat = nextFrame->joints[i];

		newPos.x = ((nextPosMat._41 - prevPosMat._41) * ratio) + prevPosMat._41;
		newPos.y = ((nextPosMat._42 - prevPosMat._42) * ratio) + prevPosMat._42;
		newPos.z = ((nextPosMat._43 - prevPosMat._43) * ratio) + prevPosMat._43;
		newPos.w = 1;

		XMVECTOR currPos = XMLoadFloat4(&newPos);

		XMMATRIX temp = XMLoadFloat4x4(&theMesh->transformMat);

		XMStoreFloat4(&newPos, XMVector3Transform(currPos, temp));

		theMesh->bones[i].position = newPos;
	}
	if (theMesh->actualBonesCount % 2 != 0)
	{
		XMFLOAT4 newPos;
		XMFLOAT4X4 prevPosMat = prevFrame->joints[theMesh->actualBonesCount - 1];
		XMFLOAT4X4 nextPosMat = nextFrame->joints[theMesh->actualBonesCount - 1];

		newPos.x = ((nextPosMat._41 - prevPosMat._41) * ratio) + prevPosMat._41;
		newPos.y = ((nextPosMat._42 - prevPosMat._42) * ratio) + prevPosMat._42;
		newPos.z = ((nextPosMat._43 - prevPosMat._43) * ratio) + prevPosMat._43;
		newPos.w = 1;

		XMVECTOR currPos = XMLoadFloat4(&newPos);

		XMMATRIX temp = XMLoadFloat4x4(&theMesh->transformMat);

		XMStoreFloat4(&newPos, XMVector3Transform(currPos, temp));

		theMesh->bones[theMesh->actualBonesCount].position = newPos;
	}

}

void updatePoseData(GraphicsSystem::object* theMesh, double time, bool tween)
{
	theMesh->timePassed += time * timeMultiplier;

	if (tween)
	{
		playTween(theMesh);

		theMesh->currentFrame++;
	}
	else
	{
		if (theMesh->timePassed >= theMesh->theAnimation.frames[theMesh->currentFrame].time)
		{
			playFrame(theMesh);

			theMesh->currentFrame++;
		}
	}

	if (theMesh->timePassed >= theMesh->theAnimation.duration || theMesh->currentFrame >= (int)theMesh->theAnimation.frames.size())
	{
		theMesh->currentFrame = 0;
		theMesh->timePassed = 0;
	}
}

void moveFrame(GraphicsSystem::object* theMesh, bool forward)
{
	if (forward)
	{
		theMesh->currentFrame++;
		if (theMesh->currentFrame >= (int)theMesh->theAnimation.frames.size())
		{
			theMesh->currentFrame = 0;
		}
		playFrame(theMesh);
	}
	else
	{
		theMesh->currentFrame--;
		if (theMesh->currentFrame < 0)
		{
			theMesh->currentFrame = theMesh->theAnimation.frames.size() - 1;
		}
		playFrame(theMesh);
	}
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
	theMageLoader->save(mageMeshFile);
	theMageLoader->savePose(magePoseFile);
	theMageLoader->saveAnimationStack(magePoseFile);
	delete theMageLoader;


	//import and save Teddy stuff
	theTeddyLoader->importer();
	theTeddyLoader->save(teddyMeshFile);
	theTeddyLoader->savePose(teddyPoseFile);
	theTeddyLoader->saveAnimationStack(teddyPoseFile);
	delete theTeddyLoader;


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

#pragma region InitObjectData
	//setup object data
	setupMeshData(mageMeshFile, mageMesh, &perspectiveMatrix, eye, at, up);
	delete mageMeshFile;
	setupDebugPoseData(magePoseFile, mageMesh);
	setupAnimData(magePoseFile, mageMesh);
	delete magePoseFile;

	setupMeshData(teddyMeshFile, teddyMesh, &perspectiveMatrix, eye, at, up);
	delete teddyMeshFile;
	setupDebugPoseData(teddyPoseFile, teddyMesh);
	setupAnimData(teddyPoseFile, teddyMesh);
	delete teddyPoseFile;
#pragma endregion

#pragma region Translations
	//Mage
	mageMesh->translateObject(-10.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	//Teddy
	teddyMesh->translateObject(0.0f, 0.0f, 0.0f, 0.02f, 0.02f, 0.02f);
#pragma endregion



	//finish inits for buffers
	graphicsStuff.initOverall(&pipeData, hWnd, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);
	graphicsStuff.basicSetUpIndexBuffer(&pipeData, mageMesh);
	graphicsStuff.basicSetUpIndexBuffer(&pipeData, teddyMesh);



	while (running)
	{
		//make sure to do any object movment before this
#pragma region InitDebugRenderStuff 
		debugVerts.clear();
		//Init debug render
		for (unsigned int i = 0; i < mageMesh->bones.size(); i++)
		{
			debugVerts.push_back(mageMesh->bones[i]);
		}
		for (unsigned int i = 0; i < teddyMesh->bones.size(); i++)
		{
			debugVerts.push_back(teddyMesh->bones[i]);
		}


		debugObject->vertexCount = debugVertCount;
		debugObject->theObject = &debugVerts[0];

		debugObject->topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

		XMStoreFloat4x4(&debugObject->theMatrix.projection, XMMatrixTranspose(perspectiveMatrix));
		XMStoreFloat4x4(&debugObject->theMatrix.view, XMMatrixTranspose(XMMatrixLookAtLH(eye, at, up)));
		XMStoreFloat4x4(&debugObject->theMatrix.model, XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, 0.0f)));

		graphicsStuff.basicSetUpInOrderBuffer(&pipeData, debugObject);
#pragma endregion

#pragma region TimerStuff

		curr_time = std::chrono::high_resolution_clock::now();

		double delta_time = std::chrono::duration<double>(curr_time - last_time).count();

		last_time = curr_time;

		accum_time += delta_time;
#pragma endregion


		XMStoreFloat4x4(&mageMesh->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));
		XMStoreFloat4x4(&teddyMesh->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));
		XMStoreFloat4x4(&debugObject->theMatrix.view, XMMatrixTranspose(XMMatrixInverse(nullptr, XMLoadFloat4x4(&camera))));



		if (playingAnim)
		{
			updatePoseData(mageMesh, delta_time, playingTween);
			updatePoseData(teddyMesh, delta_time, playingTween);
		}



#pragma region Drawing Stuff
		graphicsStuff.setGeneralPipelineStages(&pipeData);

		//mageMesh draw 
		graphicsStuff.setObjectPipelineStages(&pipeData, mageMesh);
		graphicsStuff.drawIndex(&pipeData, mageMesh);

		//teddyMesh draw
		graphicsStuff.setObjectPipelineStages(&pipeData, teddyMesh);
		graphicsStuff.drawIndex(&pipeData, teddyMesh);

		//debugobject draw
		graphicsStuff.setObjectPipelineStages(&pipeData, debugObject);
		graphicsStuff.drawInOrder(&pipeData, debugObject);


		pipeData.swapchain->Present(1, 0);
#pragma endregion


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
				case 'P':
					playingAnim = !playingAnim;
					break;
				case 'J':
					moveFrame(mageMesh, false);
					moveFrame(teddyMesh, false);
					break;
				case 'K':
					moveFrame(mageMesh, true);
					moveFrame(teddyMesh, true);
					break;
				case 'T':
					playingTween = !playingTween;
					break;
				default:
					break;
				}
				break;
			case WM_QUIT:
				running = false;
				graphicsStuff.cleanUpObject(mageMesh);
				graphicsStuff.cleanUpObject(teddyMesh);
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
