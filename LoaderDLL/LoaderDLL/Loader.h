#pragma once
#include <fbxsdk.h>

#ifdef LOADERDEFINE
#define LOADERAPI __declspec(dllexport)
#else
#define LOADERAPI __declspec(dllimport)
#endif // LOADERDEFINE

namespace functionLibrary
{
	class FBXLoader
	{
	public:
		//variables
		FbxManager* theManager;
		FbxScene* theScene;
		FbxIOSettings* theIOSettings;
		FbxMesh* oMesh;
		char* iFileName;

		//functions
		LOADERAPI FBXLoader(char* newFileName);
		LOADERAPI ~FBXLoader();
		LOADERAPI void importer();
		LOADERAPI void saver();
	};
}