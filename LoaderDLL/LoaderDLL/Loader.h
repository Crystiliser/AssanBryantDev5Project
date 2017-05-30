#pragma once
#include <fbxsdk.h>
#include "ExportFile.h"

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
		exportFile theData;

		//functions
		LOADERAPI FBXLoader(char* newFileName);
		LOADERAPI ~FBXLoader();
		LOADERAPI void importer();
		LOADERAPI exportFile saver();
	};
}