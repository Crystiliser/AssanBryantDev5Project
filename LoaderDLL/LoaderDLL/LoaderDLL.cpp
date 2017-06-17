// LoaderDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
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
		theScene->Destroy();
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

	void FBXLoader::savePose(exportFile* returnVal)
	{
		returnVal->indexed = false;

		unsigned int poseCount = theScene->GetPoseCount();

		for (unsigned int i = 0; i < poseCount; i++)
		{
			FbxPose* object = theScene->GetPose(i);
			if (object->IsBindPose())
			{
				unsigned int nodeCount = object->GetCount();
				for (unsigned int j = 0; j < nodeCount; j++)
				{
					FbxNode* node = object->GetNode(j);
					FbxSkeleton* skeleton = node->GetSkeleton();
					if (skeleton != nullptr)
					{
						if (skeleton->IsSkeletonRoot())
						{
							goThoughNodeArray(skeleton->GetNode(), -1);
							changeNodeArrayToVertexArray(nodeArray, returnVal);
							break;
						}
					}
				}
			}
		}
	}

	void FBXLoader::save(exportFile* theData)
	{
		theData->indexed = true;

		unsigned int geometryCount = theScene->GetGeometryCount();

		for (unsigned int i = 0; i < geometryCount; i++)
		{
			FbxGeometry* object = theScene->GetGeometry(i);
			FbxNodeAttribute::EType type = object->GetAttributeType();
			if (type == FbxNodeAttribute::eMesh)
			{
				if (!object->GetNode())
					break;

				const int lPolygonCount = ((FbxMesh*)object)->GetPolygonCount();

				// Count the polygon count of each material
				FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
				FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
				if (object->GetElementMaterial())
				{
					lMaterialIndice = &object->GetElementMaterial()->GetIndexArray();
					lMaterialMappingMode = object->GetElementMaterial()->GetMappingMode();
					if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
					{
						FBX_ASSERT(lMaterialIndice->GetCount() == lPolygonCount);
						if (lMaterialIndice->GetCount() == lPolygonCount)
						{
							// Count the faces of each material
							for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
							{
								const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
								if (mSubMeshes.GetCount() < lMaterialIndex + 1)
								{
									mSubMeshes.Resize(lMaterialIndex + 1);
								}
								if (mSubMeshes[lMaterialIndex] == NULL)
								{
									mSubMeshes[lMaterialIndex] = new SubMesh;
								}
								mSubMeshes[lMaterialIndex]->TriangleCount += 1;
							}

							// Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
							// if, in the loop above, we resized the mSubMeshes by more than one slot.
							for (int i = 0; i < mSubMeshes.GetCount(); i++)
							{
								if (mSubMeshes[i] == NULL)
									mSubMeshes[i] = new SubMesh;
							}

							// Record the offset (how many vertex)
							const int lMaterialCount = mSubMeshes.GetCount();
							int lOffset = 0;
							for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
							{
								mSubMeshes[lIndex]->IndexOffset = lOffset;
								lOffset += mSubMeshes[lIndex]->TriangleCount * 3;
								// This will be used as counter in the following procedures, reset to zero
								mSubMeshes[lIndex]->TriangleCount = 0;
							}
							FBX_ASSERT(lOffset == lPolygonCount * 3);
						}
					}
				}

				// All faces will use the same material.
				if (mSubMeshes.GetCount() == 0)
				{
					mSubMeshes.Resize(1);
					mSubMeshes[0] = new SubMesh();
				}

				// Congregate all the data of a mesh to be cached in VBOs.
				// If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
				mHasNormal = object->GetElementNormalCount() > 0;
				mHasUV = object->GetElementUVCount() > 0;
				FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
				FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
				if (mHasNormal)
				{
					lNormalMappingMode = object->GetElementNormal(0)->GetMappingMode();
					if (lNormalMappingMode == FbxGeometryElement::eNone)
					{
						mHasNormal = false;
					}
					if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
					{
						mAllByControlPoint = false;
					}
				}
				if (mHasUV)
				{
					lUVMappingMode = object->GetElementUV(0)->GetMappingMode();
					if (lUVMappingMode == FbxGeometryElement::eNone)
					{
						mHasUV = false;
					}
					if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
					{
						mAllByControlPoint = false;
					}
				}

				// Allocate the array memory, by control point or by polygon vertex.
				int lPolygonVertexCount = object->GetControlPointsCount();
				if (!mAllByControlPoint)
				{
					lPolygonVertexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
				}
				theData->myData = new exportFile::vertex[lPolygonVertexCount];
				theData->uniqueVerticeCount = lPolygonVertexCount;
				theData->indicies = new unsigned int[lPolygonCount * TRIANGLE_VERTEX_COUNT];
				theData->indexCount = lPolygonCount * TRIANGLE_VERTEX_COUNT;
				
				FbxStringList lUVNames;
				object->GetUVSetNames(lUVNames);
				const char * lUVName = NULL;
				if (mHasUV && lUVNames.GetCount())
				{
					lUVName = lUVNames[0];
				}

				// Populate the array with vertex attribute, if by control point.
				const FbxVector4 * lControlPoints = object->GetControlPoints();
				FbxVector4 lCurrentVertex;
				FbxVector4 lCurrentNormal;
				FbxVector2 lCurrentUV;
				if (mAllByControlPoint)
				{
					const FbxGeometryElementNormal * lNormalElement = NULL;
					const FbxGeometryElementUV * lUVElement = NULL;
					if (mHasNormal)
					{
						lNormalElement = object->GetElementNormal(0);
					}
					if (mHasUV)
					{
						lUVElement = object->GetElementUV(0);
					}
					for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
					{
						// Save the vertex position.
						lCurrentVertex = lControlPoints[lIndex];
						theData->myData[lIndex].position.x = static_cast<float>(lCurrentVertex[0]);
						theData->myData[lIndex].position.y = static_cast<float>(lCurrentVertex[1]);
						theData->myData[lIndex].position.z = static_cast<float>(lCurrentVertex[2]);
						theData->myData[lIndex].position.w = 1;

						// Save the normal.
						if (mHasNormal)
						{
							int lNormalIndex = lIndex;
							if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
							{
								lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
							}
							lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
							theData->myData[lIndex].normal.x = static_cast<float>(lCurrentNormal[0]);
							theData->myData[lIndex].normal.y = static_cast<float>(lCurrentNormal[1]);
							theData->myData[lIndex].normal.z = static_cast<float>(lCurrentNormal[2]);
						}

						// Save the UV.
						if (mHasUV)
						{
							int lUVIndex = lIndex;
							if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
							{
								lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
							}
							lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
							theData->myData[lIndex].UV.x = static_cast<float>(lCurrentUV[0]);
							theData->myData[lIndex].UV.y = static_cast<float>(lCurrentUV[1]);
						}
					}

				}

				int lVertexCount = 0;
				for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
				{
					// The material for current face.
					int lMaterialIndex = 0;
					if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon)
					{
						lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
					}

					// Where should I save the vertex attribute index, according to the material
					const int lIndexOffset = mSubMeshes[lMaterialIndex]->IndexOffset +
						mSubMeshes[lMaterialIndex]->TriangleCount * 3;
					for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
					{
						const int lControlPointIndex = ((FbxMesh*)object)->GetPolygonVertex(lPolygonIndex, lVerticeIndex);

						if (mAllByControlPoint)
						{
							theData->indicies[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
						}
						// Populate the array with vertex attribute, if by polygon vertex.
						else
						{
							theData->indicies[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);

							lCurrentVertex = lControlPoints[lControlPointIndex];
							theData->myData[lVertexCount].position.x = static_cast<float>(lCurrentVertex[0]);
							theData->myData[lVertexCount].position.y = static_cast<float>(lCurrentVertex[1]);
							theData->myData[lVertexCount].position.z = static_cast<float>(lCurrentVertex[2]);
							theData->myData[lVertexCount].position.w = 1;

							if (mHasNormal)
							{
								((FbxMesh*)object)->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
								theData->myData[lVertexCount].normal.x = static_cast<float>(lCurrentNormal[0]);
								theData->myData[lVertexCount].normal.y = static_cast<float>(lCurrentNormal[1]);
								theData->myData[lVertexCount].normal.z = static_cast<float>(lCurrentNormal[2]);
							}

							if (mHasUV)
							{
								bool lUnmappedUV;
								((FbxMesh*)object)->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
								theData->myData[lVertexCount].UV.x = static_cast<float>(lCurrentUV[0]);
								theData->myData[lVertexCount].UV.y = static_cast<float>(lCurrentUV[1]);
							}
						}
						++lVertexCount;
					}
					mSubMeshes[lMaterialIndex]->TriangleCount += 1;
				}

				break;
			}
		}
	}

	void FBXLoader::goThoughNodeArray(FbxNode* node, int parentIndex)
	{
		myFbxJoint newNode;
		newNode.node = node;
		newNode.parentIndex = parentIndex;
		nodeArray.push_back(newNode);
		nodeCount++;

		unsigned int childCount = node->GetChildCount();
		if (childCount > 0)
		{
			for (unsigned int i = 0; i < childCount; i++)
			{
				FbxNode* childNode = node->GetChild(i);

				goThoughNodeArray(childNode, nodeCount);

				newNode.node = node;
				newNode.parentIndex = parentIndex;
				nodeArray.push_back(newNode);
				nodeCount++;
			}
	}

	}

	void FBXLoader::changeNodeArrayToVertexArray(std::vector<myFbxJoint> theArray, exportFile* newFile)
	{
		newFile->myData = new exportFile::vertex[theArray.size()];
		newFile->uniqueVerticeCount = theArray.size();
		for (unsigned int i = 0; i < theArray.size(); i++)
		{
			
			FbxAMatrix tempMatrix;
			tempMatrix = theArray[i].node->EvaluateGlobalTransform();

			exportFile::vertex fin;
			FbxVector4 translation = tempMatrix.GetT();
			FbxVector4 mul = translation;

			fin.position.x = static_cast<float>(translation[0]);
			fin.position.y = static_cast<float>(translation[1]);
			fin.position.z = static_cast<float>(translation[2]);
			fin.position.w = 1;

			newFile->myData[i] = fin;

			FbxNode* child = theArray[i].node->GetParent();
			for (unsigned int j = 0; j < theArray.size(); j++)
			{
				if (child == theArray[j].node)
				{
					theArray[i].parentIndex = j;
				}
			}
		}
	}

	void FBXLoader::saveAnimationStack(exportFile* theData)
	{
		theData->animated = true;

		FbxAnimStack* theAnimStack = theScene->GetCurrentAnimationStack();

		FbxTimeSpan theTimeSpan = theAnimStack->GetLocalTimeSpan();

		FbxTime theTime = theTimeSpan.GetDuration();

		FbxLongLong frameCount = theTime.GetFrameCount(FbxTime::EMode::eFrames24);

		theData->theAnimation.duration = static_cast<double>(theTime.GetMilliSeconds());
		
		for (FbxLongLong i = 1; i < frameCount; i++)
		{
			exportFile::keyframe currentFrame;
			FbxTime newTime;
			newTime.SetFrame(i, FbxTime::EMode::eFrames24);
			currentFrame.time = static_cast<double>(newTime.GetMilliSeconds());

			for (unsigned int j = 0; j < nodeArray.size(); j++)
			{
				FbxMatrix newMatrix;
				newMatrix = nodeArray[j].node->EvaluateGlobalTransform(newTime);
				FbxVector4 firstRow = newMatrix.GetRow(0);
				FbxVector4 secondRow = newMatrix.GetRow(1);
				FbxVector4 thirdRow = newMatrix.GetRow(2);
				FbxVector4 fourthRow = newMatrix.GetRow(3);
				float* matrixArray = new float[16];
				matrixArray[0] = static_cast<float>(firstRow[0]);
				matrixArray[1] = static_cast<float>(firstRow[1]);
				matrixArray[2] = static_cast<float>(firstRow[2]);
				matrixArray[3] = static_cast<float>(firstRow[3]);

				matrixArray[4] = static_cast<float>(secondRow[0]);
				matrixArray[5] = static_cast<float>(secondRow[1]);
				matrixArray[6] = static_cast<float>(secondRow[2]);
				matrixArray[7] = static_cast<float>(secondRow[3]);

				matrixArray[8] = static_cast<float>(thirdRow[0]);
				matrixArray[9] = static_cast<float>(thirdRow[1]);
				matrixArray[10] = static_cast<float>(thirdRow[2]);
				matrixArray[11] = static_cast<float>(thirdRow[3]);

				matrixArray[12] = static_cast<float>(fourthRow[0]);
				matrixArray[13] = static_cast<float>(fourthRow[1]);
				matrixArray[14] = static_cast<float>(fourthRow[2]);
				matrixArray[15] = static_cast<float>(fourthRow[3]);

				currentFrame.joints.push_back(matrixArray);
			}
			theData->theAnimation.frames.push_back(currentFrame);
		}
	}
}
