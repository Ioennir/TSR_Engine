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

void TSR_DrawGUI(IMData* imData)
{
	DX11Data& dxData = DX11::dxData;
	FrameStats& fStats = Profiling::frameStats;
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

	ImGui::Begin("Camera details");
		//ImGui::SliderFloat3("Position", );
		float fp[] = { CameraControl::camPosition.x, CameraControl::camPosition.y, CameraControl::camPosition.z };
		ImGui::SliderFloat3("Position", fp, 0.0f, 1000.0f);
		float fo[] = { CameraControl::camRotation.x, CameraControl::camRotation.y, CameraControl::camRotation.z };
		ImGui::SliderFloat3("Orientation", fo, 0.0f, 1000.0f);
		float ft[] = { CameraControl::camTarget.x, CameraControl::camTarget.y, CameraControl::camTarget.z };
		ImGui::SliderFloat3("Target", ft, 0.0f, 1000.0f);

		float fu[] = { CameraControl::camUp.x, CameraControl::camUp.y, CameraControl::camUp.z };
		ImGui::SliderFloat3("Up", fu, 0.0f, 1000.0f);
		float fr[] = { CameraControl::camRight.x, CameraControl::camRight.y, CameraControl::camRight.z };
		ImGui::SliderFloat3("Right", fr, 0.0f, 1000.0f);
		float ff[] = { CameraControl::camFwd.x, CameraControl::camFwd.y, CameraControl::camFwd.z };
		ImGui::SliderFloat3("Forward", ff, 0.0f, 1000.0f);
		
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

void InitializeCBuffer(CameraData& CamData, DX11Data* dxData, ConstantBuffer* cbuffer) {
	//Simple translation followed by the camera view and projection
	//Note(Fran): Camera world now is identity but maybe we should multiply it aswell for the future
	//transpose?
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(mWorld * CamData.mWorld * CamData.mView * CamData.mProj);
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

void UpdateCBuffer(const CameraData& CamData, float deltarot, float rotaxis[3], ConstantBuffer* cbuffer) {

	float anim = 180.0f;// DirectX::XMConvertToRadians(deltarot);

	// triangle transformations/ world matrix basically rotate around arbitrary axis with arbitrary speed
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis({ rotaxis[0], rotaxis[1], rotaxis[2], 0.0f }, -anim);
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, -3.0f, 5.0f);
	DirectX::XMMATRIX currentWorld = scaleMatrix * rotationMatrix * translation;

	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(currentWorld * CamData.mView * CamData.mProj);

	DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(currentWorld.r, currentWorld));

	*cbuffer = {
		currentWorld,
		mWVP,
		normalMatrix
	};
}

void UpdatePlane(const CameraData& CamData, ConstantBuffer* cbuffer)
{
	DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, -4.0f, 3.5f);
	DirectX::XMMATRIX currentWorld = scaleMatrix * rotationMatrix * translation;
	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(currentWorld * CamData.mView * CamData.mProj);
	DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(currentWorld.r, currentWorld));
	*cbuffer = {
		currentWorld,
		mWVP,
		normalMatrix
	};
}

void TSR_Update(float dt)
{
	UpdateCamera(dt, &CameraControl::CamData);
}

//NOTE(I should think on grouping by material or something like that. (VIVI's ball hasnt got normals so it appears black)
//Generates the drawcalls for the model 
void TSR_RenderEntity(ID3D11DeviceContext * context, ModelBuffers * buffers, DrawComponent * drawable)
{
	// for now, one drawcall.
	context->IASetVertexBuffers(0, 1, &buffers->vertexBuffer.buffer, &buffers->vertexBuffer.stride, &buffers->vertexBuffer.offset);
	context->IASetIndexBuffer(buffers->indexBuffer.buffer, DXGI_FORMAT_R32_UINT, buffers->indexBuffer.offset);
	context->PSSetSamplers(0, 1, &DX11::dxData.samplerState);
	context->PSSetShaderResources(3, 1, &Lighting::LightBufferView);
	context->PSSetShaderResources(4, 1, &Lighting::PointLightBufferView);
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

//redo this
void TSR_RenderPrimitive(ID3D11DeviceContext * context, ModelBuffers * primitiveBuffers)
{
	context->IASetVertexBuffers(0, 1, &primitiveBuffers->vertexBuffer.buffer, &primitiveBuffers->vertexBuffer.stride, &primitiveBuffers->vertexBuffer.offset);
	context->IASetIndexBuffer(primitiveBuffers->indexBuffer.buffer, DXGI_FORMAT_R32_UINT, primitiveBuffers->indexBuffer.offset);
	//plane
	context->DrawIndexed(6, 0, 0);
}

void TSR_Draw(float rotVelocity, ConstantBuffer* cbuffer, IMData* imData, DX11VertexShaderData& vsData, DX11PixelShaderData& psData, ModelBuffers * buffers, DX11VertexShaderData& primVS, DX11PixelShaderData& primPS, ModelBuffers * primitiveBuffers, DrawComponent * drawable)
{
	DX11Data& dxData = DX11::dxData;
	CameraData* CamData = &CameraControl::CamData;
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
	UpdateCBuffer(*CamData, rotVelocity, imData->rot, cbuffer);
	dxData.context->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.context->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	dxData.context->PSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	TSR_RenderEntity(dxData.context, buffers, drawable);


	dxData.context->IASetInputLayout(primVS.inputLayout);
	dxData.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxData.context->VSSetShader(primVS.shader, 0, 0);
	dxData.context->PSSetShader(primPS.shader, 0, 0);
	UpdatePlane(*CamData, cbuffer);
	dxData.context->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.context->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	dxData.context->PSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	dxData.context->PSSetShaderResources(3, 1, &Lighting::LightBufferView);
	dxData.context->PSSetShaderResources(4, 1, &Lighting::PointLightBufferView);
	TSR_RenderPrimitive(dxData.context, primitiveBuffers);
}

