// LoaderDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#if 0
#include "TestHeader.h"

namespace MathLibrary
{
	double Functions::Add(double a, double b)
	{
		return a + b;
	}

	double Functions::Multiply(double a, double b)
	{
		return a * b;
	}

	double Functions::AddMultiply(double a, double b)
	{
		return a + (a * b);
	}
}
#endif
#include "Loader.h"

namespace functionLibrary
{
	FBXLoader::FBXLoader(char* newFileName)
	{
		iFileName = newFileName;
		theManager = FbxManager::Create();
		theScene = FbxScene::Create(theManager, "");
		theIOSettings = FbxIOSettings::Create(theManager, IOSROOT);
		theManager->SetIOSettings(theIOSettings);
	}
	FBXLoader::~FBXLoader()
	{
		theManager->Destroy();
	}
	void FBXLoader::importer()
	{
		FbxImporter* theImporter = FbxImporter::Create(theManager, "");
		if (!theImporter->Initialize(iFileName, -1, theManager->GetIOSettings()))
		{
			printf("Error importing");
		}

		theImporter->Import(theScene);
		theImporter->Destroy();
	}
	void FBXLoader::saver()
	{

	}
}
