#pragma once

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
	struct matriceData
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
		XMFLOAT4X4 model;
	};

	struct object
	{
		vertex* theObject;
		matriceData theMatrix;
		unsigned int* indices;
		unsigned int vertexCount;
		unsigned int indexCount;

		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		ID3D11Buffer* vertexBuffer = NULL;
		ID3D11Buffer* constantBuffer = NULL;
		ID3D11Buffer* indexBuffer = NULL;
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

