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
		bool mHasNormal = false;
		bool mHasUV = false;
		bool mAllByControlPoint = true;


#if 0
		struct myFbxJoint { FbxNode* node; int parentIndex; int myIndex; };
		std::vector<myFbxJoint> nodeArray;
		int nodeCount = 0;

		LOADERAPI void goThoughNodeArray(FbxNode* node, int parentIndex);

		LOADERAPI void changeNodeArrayToVertexArray(std::vector<myFbxJoint> theArray, exportFile* newFile);
#else
		struct myFbxJoint { FbxNode* node; int parentIndex; };
		std::vector<myFbxJoint> nodeArray;
		int nodeCount = 0;

		LOADERAPI void goThoughNodeArray(FbxNode* node, int parentIndex);

		LOADERAPI void changeNodeArrayToVertexArray(std::vector<myFbxJoint> theArray, exportFile* newFile);

#endif

		const int TRIANGLE_VERTEX_COUNT = 3;

		// Four floats for every position.
		const int VERTEX_STRIDE = 4;
		// Three floats for every normal.
		const int NORMAL_STRIDE = 3;
		// Two floats for every UV.
		const int UV_STRIDE = 2;

		//variables
		FbxManager* theManager;
		FbxScene* theScene;
		FbxIOSettings* theIOSettings;
		FbxMesh* oMesh; 
		FbxGeometry* object; 
		FbxImporter* theImporter;
		FbxNodeAttribute::EType type;
		char* iFileName;
		exportFile theData;

		//functions
		LOADERAPI FBXLoader(char* newFileName);
		LOADERAPI ~FBXLoader();
		LOADERAPI void importer();
		LOADERAPI void savePose(exportFile* temp);
		LOADERAPI void save();   
	};
}