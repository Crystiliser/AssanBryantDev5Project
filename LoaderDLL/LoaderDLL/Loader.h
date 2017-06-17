#pragma once
#include <fbxsdk.h>
#include "ExportFile.h"
#include <vector>

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
		
		struct SubMesh
		{
			SubMesh() : IndexOffset(0), TriangleCount(0) {}

			int IndexOffset;
			int TriangleCount;
		};
		FbxArray<SubMesh*> mSubMeshes;
		bool mHasNormal = true;
		bool mHasUV = true;
		bool mAllByControlPoint = true;


		struct myFbxJoint { FbxNode* node; int parentIndex; };
		std::vector<myFbxJoint> nodeArray;
		int nodeCount = 0;

		LOADERAPI void goThoughNodeArray(FbxNode* node, int parentIndex);
		LOADERAPI void changeNodeArrayToVertexArray(std::vector<myFbxJoint> theArray, exportFile* newFile);


		LOADERAPI void saveAnimationStack(exportFile* theData);




		const int TRIANGLE_VERTEX_COUNT = 3;

		//variables
		FbxManager* theManager;
		FbxScene* theScene;
		FbxIOSettings* theIOSettings;
		FbxMesh* oMesh; 
		FbxGeometry* object; 
		FbxImporter* theImporter;
		FbxNodeAttribute::EType type;
		char* iFileName;

		//functions
		LOADERAPI FBXLoader(char* newFileName);
		LOADERAPI ~FBXLoader();
		LOADERAPI void importer();
		LOADERAPI void savePose(exportFile* temp);
		LOADERAPI void save(exportFile* theData);
	};
}