
#include "stdafx.h"
#include "GraphicsSystem.h"
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"


XMMATRIX GraphicsSystem::perspectiveProjection(float width, float height)
{
	float aspectRatio = (width / height);
	float fovAngleY = 70.0f * XM_PI / 180.0f;


	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);

	return perspectiveMatrix;
}

void GraphicsSystem::initViewport(pipelineData * state, unsigned int width, unsigned int height)
{
	ZeroMemory(&state->viewport, sizeof(D3D11_VIEWPORT));

	state->viewport.TopLeftX = 0;
	state->viewport.TopLeftY = 0;
	state->viewport.MaxDepth = 1;
	state->viewport.MinDepth = 0;
	state->viewport.Width = width;
	state->viewport.Height = height;
}

void GraphicsSystem::setPipelinesStages(pipelineData * state)
{
	state->devcon->OMSetDepthStencilState(state->depthStencilState, 0);

	state->devcon->OMSetRenderTargets(1, &state->renderTarget, state->depthStencilView);

	state->devcon->ClearRenderTargetView(state->renderTarget, Black);

	state->devcon->ClearDepthStencilView(state->depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);

	state->devcon->IASetVertexBuffers(0, 1, &state->vertexBuffer, &state->stride, &state->offset);

	state->devcon->IASetInputLayout(state->inputLayout);

	state->devcon->IASetPrimitiveTopology(state->topology);

	state->devcon->VSSetShader(state->vertexShader, 0, 0);

	state->devcon->PSSetShader(state->pixelShader, 0, 0);

	state->devcon->RSSetViewports(1, &state->viewport);

	state->devcon->VSSetConstantBuffers(0, 1, &state->constantBuffer);

	state->devcon->PSSetConstantBuffers(0, 1, &state->constantBuffer);
}

void GraphicsSystem::basicSetUpBuffer(pipelineData * state, vertex* points, unsigned int size)
{

	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(vertex) * size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initial;
	initial.pSysMem = points;
	initial.SysMemPitch = 0;
	initial.SysMemSlicePitch = 0;

	state->stride = sizeof(vertex);
	state->offset = 0;

	state->dev->CreateBuffer(&bufferDesc, &initial, &state->vertexBuffer);

	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.ByteWidth = sizeof(matriceData);
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.MiscFlags = 0;

	state->dev->CreateBuffer(&cBufferDesc, NULL, &state->constantBuffer);
}

void GraphicsSystem::initDepthBuffer(pipelineData * state, unsigned int width, unsigned int height)
{
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	state->dev->CreateTexture2D(&descDepth, NULL, &state->depthStencilBuffer);
}

void GraphicsSystem::initDepthState(pipelineData * state)
{

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	state->dev->CreateDepthStencilState(&dsDesc, &state->depthStencilState);
}

void GraphicsSystem::initDepthView(pipelineData * state)
{
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	descDSV.Flags = 0;

	// Create the depth stencil view
	state->dev->CreateDepthStencilView(state->depthStencilBuffer, // Depth stencil texture
		&descDSV, // Depth stencil desc
		&state->depthStencilView);  // [out] Depth stencil view

}

void GraphicsSystem::initRasterizerState(pipelineData * state)
{
	ID3D11RasterizerState * g_pRasterState;

	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_FRONT;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = true;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;
	state->dev->CreateRasterizerState(&rasterizerState, &g_pRasterState);

}

void GraphicsSystem::initShaders(pipelineData * state)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	state->dev->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &state->vertexShader);
	state->dev->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &state->pixelShader);

	state->dev->CreateInputLayout(inputDesc, 2, Trivial_VS, sizeof(Trivial_VS), &state->inputLayout);

}

void GraphicsSystem::initOverall(pipelineData* state, HWND window, 
	unsigned int width, unsigned int height, vertex* points, unsigned int size)
{
	DXGI_SWAP_CHAIN_DESC scd;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 2;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = window;
	scd.SampleDesc.Count = 1;
	scd.Windowed = true;

#ifdef _DEBUG
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&state->swapchain,
		&state->dev,
		NULL,
		&state->devcon);

#else
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&state->swapchain,
		&state->dev,
		NULL,
		&state->devcon);
#endif

	ID3D11Texture2D *pBackBuffer;
	state->swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

	state->dev->CreateRenderTargetView(pBackBuffer, NULL, &state->renderTarget);
	pBackBuffer->Release();
	initViewport(state, width, height);
	initDepthBuffer(state, width, height);
	initDepthState(state);
	initDepthView(state);
	basicSetUpBuffer(state, points, size);
	initShaders(state);

}

void GraphicsSystem::draw(pipelineData * state, void* data, size_t size, int vertexCount)
{

	D3D11_MAPPED_SUBRESOURCE gridSource;
	ZeroMemory(&gridSource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	state->devcon->Map(state->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gridSource);
	memcpy(gridSource.pData, data, size);
	state->devcon->Unmap(state->constantBuffer, 0);
	//state->devcon->UpdateSubresource(state->constantBuffer, 0, NULL, gridSource.pData, 500, 0);

	state->devcon->Draw(vertexCount, 0);

	state->swapchain->Present(1, 0);
}


GraphicsSystem::GraphicsSystem()
{
}


GraphicsSystem::~GraphicsSystem()
{
}