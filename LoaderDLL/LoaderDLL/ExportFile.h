#pragma once

#include "fbxsdk.h"
#include <vector>

class exportFile
{
public:
	~exportFile()
	{
		delete[] myData;
		if (indexed)
		{
			delete[] indicies;
		}
		if (animated)
		{
			for (unsigned int i = 1; i < theAnimation.frames.size() - 1; i++)
			{
				for (unsigned int j = 0; j < theAnimation.frames[i].joints.size() - 1; j++)
				{
					delete[] theAnimation.frames[i].joints[j];
				}
			}
		}
 		int x = 0;
	}
	struct FLOAT4
	{
		//Based off of directx XMFLOAT4
		float x;
		float y;
		float z;
		float w;

		FLOAT4()
		{

		}

		FLOAT4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
		{

		}

		explicit FLOAT4(_In_reads_(4) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3])
		{

		}

		FLOAT4& operator= (const FLOAT4& Float4)
		{
			x = Float4.x;
			y = Float4.y;
			z = Float4.z;
			w = Float4.w;
			return *this;
		}
	};

	struct vertex
	{
		FLOAT4 position;
		FLOAT4 normal;
		FLOAT4 UV;

		bool operator==(const vertex a)
		{
			bool returnVal = true;
			if (position.x != a.position.x) 
			{ 
				returnVal = false;
			}
			else if (position.y != a.position.y) 
			{ 
				returnVal = false;
			}
			else if (position.z != a.position.z) 
			{ 
				returnVal = false;
			}
			else if (position.w != a.position.w) 
			{ 
				returnVal = false;
			}
			return returnVal;
		}
	};

	bool indexed = false;
	bool animated = false;

	unsigned int uniqueVerticeCount = 0;
	unsigned int indexCount = 0;
	vertex* myData;
	unsigned int* indicies;


	struct keyframe { double time; std::vector<float*> joints; };
	struct animClip { double duration; std::vector<keyframe> frames; };
	animClip theAnimation;
};