
void TSR_DrawGUI(DX11Data& dxData, IMData* imData, FrameStats& fStats)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGui::Begin("Entity details");
	ImGui::SliderFloat3("Rotation axis", imData->rot, -1.0f, 1.0f);
	ImGui::DragFloat("Rotation speed", &imData->rotSpeed, 60.0f, -1000.0f, 1000.0f);
	ImGui::End();

	ImGui::Begin("Frame statistics");
	ImGui::Text("fps: %f", fStats.fps);
	ImGui::Text("ms per frame: %f", fStats.ms_per_frame);
	ImGui::Separator();
	ImGui::Text("avg fps: %f", fStats.avgfps);
	ImGui::Text("avg ms per frame: %f", fStats.avgmspf);
	ImGui::End();

	//ImGui::ShowDemoWindow();
	ImGuiWindowFlags rtWindowFlags = 0;// = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
	ImVec2 rtSize{ 640.0f, 360.0f };
	//ImGui::SetNextWindowSize(rtSize);
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin("Viewport", 0, rtWindowFlags);
	{
		//https://github.com/ocornut/imgui/issues/1287 handling window resize
		//https://github.com/ocornut/imgui/issues/2987
		ImGui::Image(PTRCAST(void*, dxData.VP.ShaderResourceView), ImVec2{ 640.0f, 360.0f }, ImVec2{ 0,0 }, ImVec2{ 1,1 });
	}
	ImGui::End();
	ImGui::Render();

	ID3D11RenderTargetView* views[1];
	views[0] = dxData.renderTargetView;
	//views[1] = dxData.textureRTView;
	dxData.imDeviceContext->OMSetRenderTargets(1, views, dxData.depthStencilView);
	//dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.textureRTView, dxData.depthStencilView);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void InitializeCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up, float fov, float aspectRatio, CameraData* camData)
{
	// camera position, target and up vector in 3D world
	DirectX::XMVECTOR cEye = DirectX::XMVectorSet(position.x, position.y, position.z, 0.0f);
	DirectX::XMVECTOR cFocus = DirectX::XMVectorSet(target.x, target.y, target.z, 0.0f);
	DirectX::XMVECTOR cUp = DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f);

	// build view matrix
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtLH(cEye, cFocus, cUp);

	// frustum data
	float yFov = DirectX::XMConvertToRadians(fov);
	float nearZ = 0.1f;
	float farZ = 1000.0f;
	DirectX::XMMATRIX mProj = DirectX::XMMatrixPerspectiveFovLH(yFov, aspectRatio, nearZ, farZ);

	// world mat
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();

	camData->mWorld = mWorld;
	camData->mView = mView;
	camData->mProj = mProj;
}

void InitializeCBuffer(CameraData& camData, DX11Data* dxData, ConstantBuffer* cbuffer) {
	//Simple translation followed by the camera view and projection
	//Note(Fran): Camera world now is identity but maybe we should multiply it aswell for the future
	//transpose?
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(mWorld * camData.mWorld * camData.mView * camData.mProj);
	DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixIdentity();
	*cbuffer = {
		mWorld,
		mWVP,
		normalMatrix
	};

	D3D11_BUFFER_DESC cbdesc{ 0 };
	cbdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbdesc.Usage = D3D11_USAGE_DEFAULT;// D3D11_USAGE_DYNAMIC;
	cbdesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
	cbdesc.ByteWidth = sizeof(*cbuffer);
	D3D11_SUBRESOURCE_DATA csd{};
	csd.pSysMem = cbuffer;
	HRESULT hr = dxData->device->CreateBuffer(&cbdesc, &csd, &dxData->dx11_cbuffer);
}

void UpdateCBuffer(const CameraData& camData, float deltarot, float rotaxis[3], ConstantBuffer* cbuffer) {

	float anim = DirectX::XMConvertToRadians(deltarot);

	// triangle transformations/ world matrix basically rotate around arbitrary axis with arbitrary speed
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, -3.0f, 7.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis({ rotaxis[0], rotaxis[1], rotaxis[2], 0.0f }, -anim);

	DirectX::XMMATRIX currentWorld = rotationMatrix * translation;

	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(currentWorld * camData.mView * camData.mProj);

	DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(currentWorld.r, currentWorld));

	*cbuffer = {
		currentWorld,
		mWVP,
		normalMatrix
	};
}

void TestUpdate(const CameraData& camData, float deltarot, float rotaxis[3], ConstantBuffer* cbuffer)
{
	float anim = DirectX::XMConvertToRadians(deltarot);
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-3.0f, -3.0f, 4.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis({ rotaxis[0], rotaxis[1], rotaxis[2], 0.0f }, -anim);

	DirectX::XMMATRIX currentWorld = rotationMatrix * translation;

	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(currentWorld * camData.mView * camData.mProj);

	DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(currentWorld.r, currentWorld));

	*cbuffer = {
		currentWorld,
		mWVP,
		normalMatrix
	};
}

void TSR_Update(float dt)
{

}

//Generates the drawcalls for the model
void TSR_RenderEntity(ID3D11DeviceContext * context, DrawComponent * drawable)
{
	// for now, one drawcall.
	context->DrawIndexed(drawable->model.indexCount, 0, 0);
}

void TSR_Draw(float rotVelocity, CameraData* camData, ConstantBuffer* cbuffer, IMData* imData, DX11Data& dxData, DX11VertexShaderData& vsData, DX11PixelShaderData& psData, ModelBuffers * buffers, ModelBuffers * primitiveBuffers, DrawComponent * drawable)
{
	//clear backbuffer
	DirectX::XMVECTORF32 clearColor_orange{ 1.0f, 0.5f, 0.0f, 1.0f };

	// VIEWPORT RENDERING
	dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.VP.RenderTargetView, dxData.VP.DepthStencilView);
	dxData.imDeviceContext->RSSetViewports(1, &dxData.VP.Viewport);

	dxData.imDeviceContext->ClearRenderTargetView(dxData.VP.RenderTargetView, reinterpret_cast<const float*>(&clearColor_orange));
	dxData.imDeviceContext->ClearDepthStencilView(dxData.VP.DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Bind shaders and buffers
	dxData.imDeviceContext->IASetInputLayout(vsData.inputLayout);
	dxData.imDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxData.imDeviceContext->VSSetShader(vsData.shader, 0, 0);
	dxData.imDeviceContext->PSSetShader(psData.shader, 0, 0);

	dxData.imDeviceContext->IASetVertexBuffers(0, 1, &buffers->vertexBuffer.buffer, &buffers->vertexBuffer.stride, &buffers->vertexBuffer.offset);
	dxData.imDeviceContext->IASetIndexBuffer(buffers->indexBuffer.buffer, DXGI_FORMAT_R32_UINT, buffers->indexBuffer.offset);

	//CBUFFER
	// TODO(Fran): Move this to the update, check the issues 
	UpdateCBuffer(*camData, rotVelocity, imData->rot, cbuffer);
	dxData.imDeviceContext->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.imDeviceContext->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);

	TSR_RenderEntity(dxData.imDeviceContext, drawable);

	//Primitive Rendering

	dxData.imDeviceContext->IASetVertexBuffers(0, 1, &primitiveBuffers->vertexBuffer.buffer, &primitiveBuffers->vertexBuffer.stride, &primitiveBuffers->vertexBuffer.offset);
	dxData.imDeviceContext->IASetIndexBuffer(primitiveBuffers->indexBuffer.buffer, DXGI_FORMAT_R32_UINT, primitiveBuffers->indexBuffer.offset);
	
	TestUpdate(*camData, rotVelocity, imData->rot, cbuffer);
	dxData.imDeviceContext->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.imDeviceContext->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	
	dxData.imDeviceContext->DrawIndexed(1080, 0, 0);

}
