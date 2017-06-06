#pragma once
class GraphicsSystem
{
public:
	struct pipelineData
	{
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
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		UINT stride;
		UINT offset;
		ID3D11Buffer* vertexBuffer = NULL;
		ID3D11Buffer* constantBuffer = NULL;
		IDXGISwapChain* swapchain = NULL;
	};
	struct vertex
	{
		XMFLOAT4 position;
		XMFLOAT4 color;
	};
	struct matriceData
	{
		XMFLOAT4X4 view;
		XMFLOAT4X4 projection;
	};

	struct object
	{
		vertex* theObject;
		matriceData theMatrix;
		unsigned int count;
	};

	XMMATRIX perspectiveProjection(float width, float height);

	void initViewport(pipelineData* state, unsigned int width, unsigned int height);

	void setPipelinesStages(pipelineData* state);

	void basicSetUpBuffer(pipelineData* state, vertex* points, unsigned int size);

	void initDepthBuffer(pipelineData* state, unsigned int width, unsigned int height);

	void initDepthState(pipelineData* state);

	void initDepthView(pipelineData* state);

	void initRasterizerState(pipelineData* state);

	void initShaders(pipelineData* state);

	void initOverall(pipelineData* state, HWND window, unsigned int width,
		unsigned int height, vertex* points, unsigned int size);

	void draw(pipelineData* state, void* data, size_t size, int vertexCount);

	void cleanUpPipeLine(pipelineData* state);

	GraphicsSystem();
	~GraphicsSystem();
};

