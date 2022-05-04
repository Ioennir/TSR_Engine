enum class Primitive : ui32 { Triangle, Plane, Cube, Sphere, Icosphere, Pyramid, Cilinder, Cone, Torus, Capsule, Count };

void TSR_DrawGUI(DX11Data& dxData, IMData* imData, FrameStats& fStats)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGui::Begin("Cube data");
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
		ImGui::Image(reinterpret_cast<void*>(dxData.scnData.shaderResourceView), ImVec2{ 640.0f, 360.0f }, ImVec2{ 0,0 }, ImVec2{ 1,1 });
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
	*cbuffer = {
		mWorld,
		mWVP
	};

	D3D11_BUFFER_DESC cbdesc{ 0 };
	cbdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbdesc.Usage = D3D11_USAGE_DEFAULT;// D3D11_USAGE_DYNAMIC;
	cbdesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
	cbdesc.ByteWidth = sizeof(*cbuffer);
	D3D11_SUBRESOURCE_DATA csd{};
	csd.pSysMem = cbuffer;
	dxData->device->CreateBuffer(&cbdesc, &csd, &dxData->dx11_cbuffer);

}

void UpdateCBuffer(const CameraData& camData, float deltarot, float rotaxis[3], ConstantBuffer* cbuffer) {

	float anim = DirectX::XMConvertToRadians(deltarot);

	// triangle transformations/ world matrix basically rotate around arbitrary axis with arbitrary speed
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, -3.0f, 7.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis({ rotaxis[0], rotaxis[1], rotaxis[2], 0.0f }, -anim);

	DirectX::XMMATRIX currentWorld = rotationMatrix * translation;

	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(currentWorld * camData.mView * camData.mProj);

	*cbuffer = {
		currentWorld,
		mWVP
	};
}

void TestUpdate(const CameraData& camData, float deltarot, float rotaxis[3], ConstantBuffer* cbuffer)
{
	float anim = DirectX::XMConvertToRadians(deltarot);
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-3.0f, -3.0f, 4.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis({ rotaxis[0], rotaxis[1], rotaxis[2], 0.0f }, -anim);

	DirectX::XMMATRIX currentWorld = rotationMatrix * translation;

	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(currentWorld * camData.mView * camData.mProj);

	*cbuffer = {
		currentWorld,
		mWVP
	};
}

void TSR_Update(float dt)
{

}

void TSR_Draw(float rotVelocity, CameraData* camData, ConstantBuffer* cbuffer, IMData* imData, DX11Data& dxData, DX11VertexShaderData& vsData, DX11PixelShaderData& psData, BufferData& vb, BufferData& ib, RenderData* renderData, BufferData& pvb, BufferData& pib)
{
	//clear backbuffer
	DirectX::XMVECTORF32 clearColor_orange{ 1.0f, 0.5f, 0.0f, 1.0f };

	// VIEWPORT RENDERING
	dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.scnData.renderTargetView, dxData.scnData.depthStencilView);
	dxData.imDeviceContext->RSSetViewports(1, &dxData.scnData.viewport);

	dxData.imDeviceContext->ClearRenderTargetView(dxData.scnData.renderTargetView, reinterpret_cast<const float*>(&clearColor_orange));
	dxData.imDeviceContext->ClearDepthStencilView(dxData.scnData.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// CUBE
	// Bind shaders and buffers
	dxData.imDeviceContext->IASetInputLayout(vsData.inputLayout);
	dxData.imDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxData.imDeviceContext->VSSetShader(vsData.shader, 0, 0);
	dxData.imDeviceContext->PSSetShader(psData.shader, 0, 0);
	dxData.imDeviceContext->IASetVertexBuffers(0, 1, &vb.buffer, &vb.stride, &vb.offset);
	dxData.imDeviceContext->IASetIndexBuffer(ib.buffer, DXGI_FORMAT_R32_UINT, ib.offset);


	//CBUFFER
	// TODO(Fran): Move this to the update, check the issues 
	UpdateCBuffer(*camData, rotVelocity, imData->rot, cbuffer);
	dxData.imDeviceContext->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.imDeviceContext->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);

	dxData.imDeviceContext->DrawIndexed(static_cast<ui32>(renderData->totalIndices.size()), 0, 0);

	dxData.imDeviceContext->IASetVertexBuffers(0, 1, &pvb.buffer, &pvb.stride, &pvb.offset);
	dxData.imDeviceContext->IASetIndexBuffer(pib.buffer, DXGI_FORMAT_R32_UINT, pib.offset);
	
	TestUpdate(*camData, rotVelocity, imData->rot, cbuffer);
	dxData.imDeviceContext->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.imDeviceContext->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);
	
	//dxData.imDeviceContext->DrawIndexed(1080, 0, 0);
	//dxData.imDeviceContext->DrawIndexedInstanced(36, 10, 0, 0, 0);
	//dxData.imDeviceContext->DrawIndexed(36, 0, 0);
	dxData.imDeviceContext->DrawIndexed(240, 0, 0);


	// MAIN WINDOW RENDERING
	//dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.renderTargetView, dxData.depthStencilView);
	//dxData.imDeviceContext->RSSetViewports(1, &dxData.windowViewport);

	//dxData.imDeviceContext->ClearRenderTargetView(dxData.renderTargetView, reinterpret_cast<const float*>(&black));
	//dxData.imDeviceContext->ClearDepthStencilView(dxData.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//// TRIANGLE RENDERING

	//dxData.imDeviceContext->DrawIndexed(36, 0, 0);


}

// stacks: vertical quads, slices: horizontal quads ex: stacks: 1 = cilinder 
void GenerateSphereGeometry(eastl::vector<Vertex>& vertices, eastl::vector<ui32>& indices, r32 radius, ui32 slices, ui32 stacks)
{
	const r32 Pi = DirectX::XM_PI;
	r32 sliceStep = 2.0f * Pi / static_cast<r32>(slices);
	r32 stackStep = Pi / static_cast<r32>(stacks);
	ui32 vertexCount = slices * (stacks - 1) + 2;
	ui32 trisCount = slices * (stacks - 1) * 2;
	ui32 indexCount = trisCount * 3;

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);

	//Vertices
	vertices.push_back({ {0.0f, -radius, 0.0f}, white });
	r32 stackAngle = Pi - stackStep;
	for (ui32 v = 0; v < stacks - 1; ++v)
	{
		r32 sliceAngle = 0.0f;
		for (ui32 h = 0; h < slices; ++h) {
			r32 x = radius*sin(stackAngle)*cos(sliceAngle);
			r32 y = radius*cos(stackAngle);
			r32 z = radius*sin(stackAngle)*sin(sliceAngle);
			vertices.push_back({ {x, y, z}, white });
			sliceAngle += sliceStep;
		}
		stackAngle -= stackStep;
	}
	vertices.push_back({ {0.0f, radius, 0.0f}, white });

	//TODO(Fran): check this out
	//Indices
	for (ui32 i = 1; i <= slices; ++i)
	{
		indices.push_back(i);
		indices.push_back(0);
		indices.push_back((i - 1 == 0 ? i + slices - 1 : i - 1));
	}

	for (ui32 i = 1; i < vertexCount - slices - 1; ++i)
	{
		indices.push_back(i + slices);
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));
		
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1: i - 1));
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));
	}

	for (ui32 i = vertexCount - slices - 1; i < vertexCount - 1; ++i)
	{
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1 : i - 1));
		indices.push_back(vertexCount - 1);
	}

}

void GenerateCilinderGeometry(eastl::vector<Vertex>& vertices, eastl::vector<ui32>& indices, r32 radius, ui32 slices)
{
	const r32 Pi = DirectX::XM_PI;
	r32 sliceStep = 2.0f * Pi / static_cast<r32>(slices);
	ui32 vertexCount = (slices * 2) + 2;
	ui32 trisCount = (slices * 4);
	ui32 indexCount = trisCount * 3;

	vertices.reserve(vertexCount);
	indices.reserve(indexCount);

	//bot
	vertices.push_back({ {0.0f, -radius, 0.0f}, white });

	//middle
	r32 sliceAngle = 0.0f;
	for (ui32 i = 0; i < slices; ++i)
	{
		r32 x = 0.0f;
		r32 z = 0.0f;
		vertices.push_back({ {x, -radius, z},white });
		sliceAngle += sliceStep;
	}

	sliceAngle = 0.0f;
	for (ui32 i = 0; i < slices; ++i)
	{
		r32 x = radius*cos(sliceAngle);
		r32 z = radius*sin(sliceAngle);
		vertices.push_back({ {x, radius, z},white });
		sliceAngle += sliceStep;
	}

	//top
	vertices.push_back({{ 0.0f, radius, 0.0f }, white});
	

	//Indices
	for (ui32 i = 1; i <= slices; ++i)
	{
		indices.push_back(i);
		indices.push_back(0);
		indices.push_back((i - 1 == 0 ? i + slices - 1 : i - 1));
	}

	for (ui32 i = 1; i < vertexCount - slices - 1; ++i)
	{
		indices.push_back(i + slices);
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));

		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1 : i - 1));
		indices.push_back(((i - 1) % slices == 0 ? i + slices + slices - 1 : i + slices - 1));
	}

	for (ui32 i = vertexCount - slices - 1; i < vertexCount - 1; ++i)
	{
		indices.push_back(i);
		indices.push_back(((i - 1) % slices == 0 ? i + slices - 1 : i - 1));
		indices.push_back(vertexCount - 1);
	}

}

void BuildPrimitiveBuffers(Primitive primitive,ID3D11Device& device, BufferData* vBuffer, BufferData* iBuffer)
{
	HRESULT hr;
	eastl::vector<Vertex> vertices;
	eastl::vector<ui32> indices;
	switch (primitive)
	{
	case Primitive::Triangle:
	{
		// Vertices of a triangle with center in 0,0,0
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), white},
				{DirectX::XMFLOAT3(0.5f, 0.0f, 0.0f), white},
				{DirectX::XMFLOAT3(-0.5f, 0.0f, 0.0f), white}
			});
		indices.insert(indices.end(),
			{
				0, 1, 2
			});
	}break;
	case Primitive::Plane:
	{
		// Vertices of a plane with center in 0,0,0
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f), white},
				{DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.0f, -0.5f), white}
			});
		indices.insert(indices.end(),
			{
				0, 1, 2,
				1, 2, 3
			});
	}break;
	case Primitive::Cube:
	{
		// Vertices of a cube with center in 0,0,0
		vertices.insert(vertices.end(),
			{
				{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), white},
				{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), white},
				{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), white},
				
				{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), white},
				{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), white},
				{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), white},
				{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), white},
			});
		indices.insert(indices.end(),
			{
				2, 0, 3, 3, 0, 1,
				0, 4, 1, 1, 4, 6,
				0, 2, 4, 4, 2, 5,
				4, 5, 6, 6, 5, 7,
				5, 2, 7, 7, 2, 3
			});
	}break;
	case Primitive::Sphere:
	{
		// has 1080 indices
		GenerateSphereGeometry(vertices, indices, 0.5f, 20, 10);
	}break;
	case Primitive::Icosphere: {}break;
	case Primitive::Pyramid:
	{
		// vertices for a pyramid with center 0,0,0
		vertices.insert(vertices.end(),
			{
				// base
				{{-0.5f, 0.0f, -0.5f}, white},
				{{0.5f, 0.0f, -0.5f}, white},
				{{-0.5f, 0.0f, 0.5f}, white},
				{{0.5f, 0.0f, 0.5f}, white},
				// pinacle
				{{0.0f, 1.0f, 0.0f}, white}
			});
		indices.insert(indices.end(),
			{
				//base
				0, 1, 2,
				1, 3, 2,
				//sides
				1, 4, 3,
				1, 0, 4,
				0, 2, 4,
				2, 3, 4
			});
	}break;
	case Primitive::Cilinder: 
	{
		GenerateCilinderGeometry(vertices, indices, 0.5f, 20);
	}break;
	case Primitive::Cone: {}break;
	case Primitive::Torus: {}break;
	case Primitive::Capsule: {}break;
	default:
	{
		printf("Wrong primitive value.\n");
	}
	}

	// Build the vertex buffer
	vBuffer->stride = sizeof(Vertex);
	vBuffer->offset = 0;
	ui32 vertMemberCount = static_cast<ui32>(vertices.size());
	D3D11_BUFFER_DESC vBuffDesc{ 0 };
	vBuffDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vBuffDesc.ByteWidth = vertMemberCount * vBuffer->stride;
	vBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vBuffData{ 0 };
	vBuffData.pSysMem = vertices.data();

	// Build the index buffer
	iBuffer->stride = sizeof(ui32);
	iBuffer->offset = 0;
	ui32 indMemberCount = static_cast<ui32>(indices.size());
	D3D11_BUFFER_DESC iBuffDesc{ 0 };
	iBuffDesc.Usage = D3D11_USAGE_IMMUTABLE;
	iBuffDesc.ByteWidth = indMemberCount * iBuffer->stride;
	iBuffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA iBuffData{ 0 };
	iBuffData.pSysMem = indices.data();

	// Create the buffers
	hr = device.CreateBuffer(
		&vBuffDesc, &vBuffData, &vBuffer->buffer
	);

	if (FAILED(hr)) 
	{
		printf("Failed to create vertex buffer for a primitive.\n");
	}

	hr = device.CreateBuffer(
		&iBuffDesc, &iBuffData, &iBuffer->buffer
	);

	if (FAILED(hr))
	{
		printf("Failed to create indexbuffer for a primitive.\n");
	}
}
