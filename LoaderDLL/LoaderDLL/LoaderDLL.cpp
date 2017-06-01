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

	exportFile FBXLoader::saver()
	{
		unsigned int geometryCount = theScene->GetGeometryCount();

		for (unsigned int i = 0; i < geometryCount; i++)
		{
			FbxGeometry* object = theScene->GetGeometry(i);
			FbxNodeAttribute::EType type = object->GetAttributeType();
			if (type == FbxNodeAttribute::eMesh)
			{
				//theData.verticeCount = ((static_cast<FbxMesh*>(object))->GetPolygonCount()) * TRIANGLE_VERTEX_COUNT;
				theData.verticeCount = object->GetControlPointsCount();
				theData.myData = new exportFile::vertex[theData.verticeCount];
				
				const FbxVector4* controlPoints = object->GetControlPoints();
				FbxVector4 currentVertex;
				for (unsigned int j = 0; j < theData.verticeCount; j++)
				{

					currentVertex = controlPoints[j];
					theData.myData[j].position.x = static_cast<float>(currentVertex[0]);
					theData.myData[j].position.y = static_cast<float>(currentVertex[1]);
					theData.myData[j].position.z = static_cast<float>(currentVertex[2]);
					theData.myData[j].position.w = 1;
				
				}
			}
		}
		return theData;
	}
}
