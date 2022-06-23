//These structs are as simple as possible for now to perform
// a basic 3D projection and get things going.
struct CameraData
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProj;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mVWP;
	DirectX::XMMATRIX normalMatrix;
};

//TODO(Fran): this will be surely changed
struct IMData
{
	r32 rot[3]{ 0.0f, 1.0f, 0.0f };
	r32 rotSpeed{ 60.0f };
};

void TSR_DrawGUI(DX11Data& dxData, IMData* imData, FrameStats& fStats)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGui::Begin("Entity details");
	//ImGui::SliderFloat3("Position", );
	ImGui::SliderFloat3("Rotation", imData->rot, 0.0f, 360.0f);
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

	
	//ImGui::Begin("texture", 0, rtWindowFlags);
	//{
	//	ImGui::Image(PTRCAST(void*, srviewtest), ImVec2{ 200.0f, 200.0f }, ImVec2{ 0,0 }, ImVec2{ 1,1 });
	//}
	//ImGui::End();

	ImGui::Render();

	ID3D11RenderTargetView* views[1];
	views[0] = dxData.renderTargetView;
	//views[1] = dxData.textureRTView;
	dxData.context->OMSetRenderTargets(1, views, dxData.depthStencilView);
	//dxData.context->OMSetRenderTargets(1, &dxData.textureRTView, dxData.depthStencilView);
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
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis({ rotaxis[0], rotaxis[1], rotaxis[2], 0.0f }, -anim);
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, -3.0f, 2.0f);
	DirectX::XMMATRIX currentWorld = scaleMatrix * rotationMatrix * translation;

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

//NOTE(I should think on grouping by material or something like that. (VIVI's ball hasnt got normals so it appears black)
//Generates the drawcalls for the model 
void TSR_RenderEntity(ID3D11DeviceContext * context, ModelBuffers * buffers, DrawComponent * drawable)
{
	// for now, one drawcall.
	context->IASetVertexBuffers(0, 1, &buffers->vertexBuffer.buffer, &buffers->vertexBuffer.stride, &buffers->vertexBuffer.offset);
	context->IASetIndexBuffer(buffers->indexBuffer.buffer, DXGI_FORMAT_R32_UINT, buffers->indexBuffer.offset);
	context->PSSetSamplers(0, 1, &DX11::dxData.samplerState);
	//context->DrawIndexed(drawable->model.indexCount, 0, 0);
	for (ui32 i = 0; i < drawable->model.submeshCount; ++i)
	{
		DX11::dxData.context->PSSetShaderResources(0, 1, &drawable->model.materials[drawable->model.submeshMaterialIndex[i]].diffuse);
		DX11::dxData.context->PSSetShaderResources(1, 1, &drawable->model.materials[drawable->model.submeshMaterialIndex[i]].normal);
		//also set the textures
		ui32 start = drawable->model.submeshStartIndex[i];
		ui32 indexcount = drawable->model.submeshEndIndex[i] - start;
		context->DrawIndexed(indexcount, start, 0);
	}
}

void TSR_Draw(float rotVelocity, CameraData* camData, ConstantBuffer* cbuffer, IMData* imData, DX11Data& dxData, DX11VertexShaderData& vsData, DX11PixelShaderData& psData, ModelBuffers * buffers, ModelBuffers * primitiveBuffers, DrawComponent * drawable)
{
	//clear backbuffer
	DirectX::XMVECTORF32 clearColor_orange{ 1.0f, 0.5f, 0.0f, 1.0f };

	// VIEWPORT RENDERING - 
	// NOTE(Fran): should this be called all the time?
	dxData.context->OMSetRenderTargets(1, &dxData.VP.RenderTargetView, dxData.VP.DepthStencilView);
	dxData.context->RSSetViewports(1, &dxData.VP.Viewport);

	dxData.context->ClearRenderTargetView(dxData.VP.RenderTargetView, PTRCAST(const float*, &clearColor_orange));
	dxData.context->ClearDepthStencilView(dxData.VP.DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Bind shaders and buffers
	dxData.context->IASetInputLayout(vsData.inputLayout);
	dxData.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxData.context->VSSetShader(vsData.shader, 0, 0);
	dxData.context->PSSetShader(psData.shader, 0, 0);
	
	//CBUFFER
	// TODO(Fran): Move this to the update, check the issues 
	UpdateCBuffer(*camData, rotVelocity, imData->rot, cbuffer);
	dxData.context->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.context->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	dxData.context->PSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	TSR_RenderEntity(dxData.context, buffers, drawable);

	//Primitive Rendering

	//TestUpdate(*camData, rotVelocity, imData->rot, cbuffer);
	//dxData.context->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	//dxData.context->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	

	//dxData.context->IASetVertexBuffers(0, 1, &primitiveBuffers->vertexBuffer.buffer, &primitiveBuffers->vertexBuffer.stride, &primitiveBuffers->vertexBuffer.offset);
	//dxData.context->IASetIndexBuffer(primitiveBuffers->indexBuffer.buffer, DXGI_FORMAT_R32_UINT, primitiveBuffers->indexBuffer.offset);
	//dxData.context->DrawIndexed(1080, 0, 0);
}

void TSR_UpdateTransformComponents()
{

}

void TSR_UpdateDrawComponentRelatedBuffers()
{

}

//Only draws the draw component of the entity
void TSR_DrawEntities()
{

}