
#include "stdafx.h"
#include "GraphicsSystem.h"
#include "Trivial_PS.csh"
#include "Trivial_VS.csh"
#include "TexturedPS.csh"
#include "TexturedVS.csh"


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
	state->viewport.Width = (float)width;
	state->viewport.Height = (float)height;
}

void GraphicsSystem::setGeneralPipelineStages(pipelineData * state)
{
	state->devcon->OMSetDepthStencilState(state->depthStencilState, 0);

	state->devcon->OMSetRenderTargets(1, &state->renderTarget, state->depthStencilView);

	state->devcon->RSSetViewports(1, &state->viewport);

	state->devcon->RSSetState(state->rasterState);

	state->devcon->ClearRenderTargetView(state->renderTarget, Black);

	state->devcon->ClearDepthStencilView(state->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

}

void GraphicsSystem::setObjectPipelineStages(pipelineData* state, object* theObject, bool debug)
{
	if (debug)
	{
		state->devcon->IASetInputLayout(state->debugInputLayout);

		state->devcon->VSSetShader(state->debugVertexShader, nullptr, 0);

		state->devcon->PSSetShader(state->debugPixelShader, nullptr, 0);
		
		state->devcon->IASetVertexBuffers(0, 1, &theObject->vertexBuffer, &state->debugStride, &state->debugOffset);
	}
	else
	{
		ID3D11ShaderResourceView* resourceArray[] = { theObject->textureView };

		state->devcon->PSSetSamplers(0, 1, &theObject->textureSampler);

		state->devcon->PSSetShaderResources(0, 1, resourceArray);

		state->devcon->IASetInputLayout(state->normalInputLayout);

		state->devcon->VSSetShader(state->normalVertexShader, nullptr, 0);

		state->devcon->PSSetShader(state->normalPixelShader, nullptr, 0);
		
		state->devcon->IASetVertexBuffers(0, 1, &theObject->vertexBuffer, &state->normalStride, &state->normalOffset);
	}

	state->devcon->IASetIndexBuffer(theObject->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	state->devcon->VSSetConstantBuffers(0, 1, &theObject->constantBuffer);

	state->devcon->PSSetConstantBuffers(0, 1, &theObject->constantBuffer);

	state->devcon->IASetPrimitiveTopology(theObject->topology);
}

void GraphicsSystem::setUpIndexBuffer(pipelineData * state, object * theObject)
{
	//vertex buffer setup
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(vertex) * theObject->vertexCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initial = { 0 };
	initial.pSysMem = theObject->theObject;
	initial.SysMemPitch = 0;
	initial.SysMemSlicePitch = 0;

	state->normalStride = sizeof(vertex);
	state->normalOffset = 0;

	state->dev->CreateBuffer(&bufferDesc, &initial, &theObject->vertexBuffer);

	//index buffer setup
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = theObject->indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	CD3D11_BUFFER_DESC indexBufferDesc(sizeof(unsigned int) * theObject->indexCount, D3D11_BIND_INDEX_BUFFER);
	state->dev->CreateBuffer(&indexBufferDesc, &indexBufferData, &theObject->indexBuffer);


	//constant buffer setup
	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.ByteWidth = sizeof(matriceData);
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.MiscFlags = 0;

	state->dev->CreateBuffer(&cBufferDesc, NULL, &theObject->constantBuffer);
}

void GraphicsSystem::basicSetUpInOrderBuffer(pipelineData * state, object* theObject)
{
	//vertex buffer setup
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(vertex) * theObject->vertexCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initial = { 0 };
	initial.pSysMem = theObject->theObject;
	initial.SysMemPitch = 0;
	initial.SysMemSlicePitch = 0;

	state->normalStride = sizeof(vertex);
	state->normalOffset = 0;

	state->dev->CreateBuffer(&bufferDesc, &initial, &theObject->vertexBuffer);

	//constant buffer setup
	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.ByteWidth = sizeof(matriceData);
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.MiscFlags = 0;

	state->dev->CreateBuffer(&cBufferDesc, NULL, &theObject->constantBuffer);
}

void GraphicsSystem::debugSetUpInOrderBuffer(pipelineData * state, object * theObject)
{
	//vertex buffer setup
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(debugVert) * theObject->vertexCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initial = { 0 };
	initial.pSysMem = theObject->debugObject;
	initial.SysMemPitch = 0;
	initial.SysMemSlicePitch = 0;

	state->debugStride = sizeof(debugVert);
	state->debugOffset = 0;

	state->dev->CreateBuffer(&bufferDesc, &initial, &theObject->vertexBuffer);

	//constant buffer setup
	D3D11_BUFFER_DESC cBufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	cBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cBufferDesc.ByteWidth = sizeof(matriceData);
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cBufferDesc.MiscFlags = 0;

	state->dev->CreateBuffer(&cBufferDesc, NULL, &theObject->constantBuffer);
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
	D3D11_RASTERIZER_DESC rasterDesc;

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	//rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	state->dev->CreateRasterizerState(&rasterDesc, &state->rasterState);

}

void GraphicsSystem::initShaders(pipelineData * state)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	state->dev->CreateVertexShader(TexturedVS, sizeof(TexturedVS), NULL, &state->normalVertexShader);
	state->dev->CreatePixelShader(TexturedPS, sizeof(TexturedPS), NULL, &state->normalPixelShader);

	state->dev->CreateInputLayout(inputDesc, 3, TexturedVS, sizeof(TexturedVS), &state->normalInputLayout);


	D3D11_INPUT_ELEMENT_DESC inputDesc2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	state->dev->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &state->debugVertexShader);
	state->dev->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &state->debugPixelShader);

	state->dev->CreateInputLayout(inputDesc2, 2, Trivial_VS, sizeof(Trivial_VS), &state->debugInputLayout);
}

void GraphicsSystem::initOverall(pipelineData* state, HWND window, 
	unsigned int width, unsigned int height)
{
	DXGI_SWAP_CHAIN_DESC scd;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 3;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = window;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = true;

#ifdef _DEBUG
	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		0,
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

	initDepthBuffer(state, width, height);
	initDepthView(state);
	initDepthState(state);
	initRasterizerState(state);
	initViewport(state, width, height);
	initShaders(state);
}

void GraphicsSystem::drawIndex(pipelineData * state, object* theObject)
{
	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	state->devcon->Map(theObject->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &theObject->theMatrix, sizeof(matriceData));
	state->devcon->Unmap(theObject->constantBuffer, 0);

	state->devcon->DrawIndexed(theObject->indexCount, 0, 0);
}

void GraphicsSystem::drawInOrder(pipelineData* state, object* theObject)
{

	D3D11_MAPPED_SUBRESOURCE resource;
	ZeroMemory(&resource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	state->devcon->Map(theObject->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &theObject->theMatrix, sizeof(matriceData));
	state->devcon->Unmap(theObject->constantBuffer, 0);

	state->devcon->Draw(theObject->vertexCount, 0);
}

void GraphicsSystem::cleanUpPipeLine(pipelineData * state)
{
	state->depthStencilBuffer->Release();
	state->depthStencilState->Release();
	state->depthStencilView->Release();
	state->dev->Release();
	state->devcon->Release();
	state->rasterState->Release();
	state->renderTarget->Release();
	state->swapchain->Release();
	
	state->debugInputLayout->Release();
	state->debugPixelShader->Release();
	state->debugVertexShader->Release();
	
	state->normalInputLayout->Release();
	state->normalPixelShader->Release();
	state->normalVertexShader->Release();
	
}

void GraphicsSystem::cleanUpObject(object * theObject)
{
	theObject->constantBuffer->Release();
	delete[] theObject->theObject;
	theObject->vertexBuffer->Release();
	if (theObject->indexCount > 0)
	{
		theObject->indexBuffer->Release();
		delete[] theObject->indices;
	}
	if (theObject->theTexture != nullptr)
	{
		theObject->theTexture->Release();
		theObject->textureSampler->Release();
		theObject->textureView->Release();
	}
}


GraphicsSystem::GraphicsSystem()
{
}


GraphicsSystem::~GraphicsSystem()
{
}

