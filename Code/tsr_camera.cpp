struct CameraData
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProj;
};

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

void UpdateCamera(float dt)
{
	const float rotSpeed = 40.0f;
	// Fetch input from keyboard
	bool forward = ImGui::IsKeyPressed(ImGuiKey_W);
	bool back = ImGui::IsKeyPressed(ImGuiKey_S);
	bool right = ImGui::IsKeyPressed(ImGuiKey_D);
	bool left = ImGui::IsKeyPressed(ImGuiKey_A);
	// convert input to axis value
	float vertical = (forward ? 1.0f : 0.0f) + (back ? -1.0f : 0.0f);
	float horizontal = (right ? 1.0f : 0.0f) + (left ? -1.0f : 0.0f);
	
	// angle
	bool rotate = ImGui::IsMouseDragging(ImGuiMouseButton_Right);
	ImVec2 ddelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
	//eastl::string s = eastl::to_string(ddelta.x);
	//LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("Y: "));
	bool test = true;
	//float amount = (rotate ? )
}
