#pragma once
#include <vector>

class GraphicsSystem
{
public:
	struct pipelineData
	{
		IDXGISwapChain* swapchain = NULL;
		ID3D11Device* dev = NULL;
		ID3D11DeviceContext *devcon = NULL;
		D3D11_VIEWPORT viewport;
		ID3D11InputLayout* inputLayout = NULL;
		ID3D11VertexShader* vertexShader = NULL;
		ID3D11PixelShader* pixelShader = NULL;
		ID3D11RenderTargetView* renderTarget = NULL;
		ID3D11Texture2D* depthStencilBuffer = NULL;
		ID3D11DepthStencilState* depthStencilState = NULL;
		ID3D11DepthStencilView* depthStencilView;
		ID3D11RasterizerState* rasterState = NULL;
		UINT stride;
		UINT offset;
	};

	struct vertex
	{
		XMFLOAT4 position = XMFLOAT4(0,0,0,0);
		XMFLOAT4 color;
	};

	struct fVertex
	{
		XMFLOAT4 position;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

	struct matriceData
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4X4 model;
	};

	struct keyframe { double time; std::vector<XMFLOAT4X4> joints; };

	struct animClip { double duration; std::vector<keyframe> frames; };

	struct object
	{
		vertex* theObject;
		fVertex* theFObject;
		bool fullVert = false;
		std::vector<vertex> bones;
		unsigned int actualBonesCount;
		matriceData theMatrix;
		unsigned int* indices;
		unsigned int vertexCount;
		unsigned int indexCount = 0;
		animClip theAnimation;
		int currentFrame = 0;
		XMFLOAT4X4 transformMat;
		double timePassed = 0;

		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		ID3D11Buffer* vertexBuffer = NULL;
		ID3D11Buffer* constantBuffer = NULL;
		ID3D11Buffer* indexBuffer = NULL;

		void translateObject(float xTranslate, float yTranslate, float zTranslate, float xScale, float yScale, float zScale)
		{
			XMMATRIX temp = XMMatrixMultiply(XMMatrixTranslation(xTranslate, yTranslate, zTranslate), XMMatrixScaling(xScale, yScale, zScale));

			XMStoreFloat4x4(&theMatrix.model, XMMatrixTranspose(temp));

			XMStoreFloat4x4(&transformMat, temp);

			for (unsigned int i = 0; i < bones.size(); i++)
			{
				XMVECTOR bonePos = XMLoadFloat4(&bones[i].position);
				XMVECTOR fin = XMVector3Transform(bonePos, temp);
				XMStoreFloat4(&bones[i].position, fin);
			}
		}
	};
	
	XMMATRIX perspectiveProjection(float width, float height);

	void initViewport(pipelineData* state, unsigned int width, unsigned int height);

	void setGeneralPipelineStages(pipelineData* state);

	void setObjectPipelineStages(pipelineData* state, object* theObject);

	void basicSetUpIndexBuffer(pipelineData* state, object* theObject);

	void basicSetUpInOrderBuffer(pipelineData * state, object* theObject);

	void initDepthBuffer(pipelineData* state, unsigned int width, unsigned int height);

	void initDepthState(pipelineData* state);

	void initDepthView(pipelineData* state);

	void initRasterizerState(pipelineData* state);

	void initShaders(pipelineData* state);

	void initOverall(pipelineData* state, HWND window, unsigned int width, unsigned int height);

	void drawIndex(pipelineData* state, object* theObject);

	void drawInOrder(pipelineData* state, object* theObject);

	void cleanUpPipeLine(pipelineData* state);

	void cleanUpObject(object* theObject);

	GraphicsSystem();
	~GraphicsSystem();
};

