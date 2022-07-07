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

namespace MouseControl
{
	DirectX::XMFLOAT2 previousCursorPosition = {0.0f, 0.0f};
}

// Fix the camera rotation
void UpdateCamera(float dt, CameraData * camData)
{
	const float rotSpeed = 15.0f;
	const float movSpeed = 15.0f;
	// Fetch input from keyboard
	
	// TODO(Fran): Check the disable obsolete keyIO
	bool forward = ImGui::IsKeyDown(ImGuiKey_W);
	bool back = ImGui::IsKeyDown(ImGuiKey_S);
	bool right = ImGui::IsKeyDown(ImGuiKey_D);
	bool left = ImGui::IsKeyDown(ImGuiKey_A);
	// convert input to axis value
	float fwdmove = (forward ? -1.0f : 0.0f) + (back ? 1.0f : 0.0f);
	float hormove = (right ? 1.0f : 0.0f) + (left ? -1.0f : 0.0f);
	
	//rebuild mView
	float xMov = hormove * movSpeed * dt;
	float zMov = fwdmove * movSpeed * dt;
	camData->mView *= DirectX::XMMatrixTranslation(xMov, 0.0f, zMov);

	//rotation
	ImVec2 posOffset = { 0.0f, 0.0f };
	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		ImVec2 currentCursorPosition = ImGui::GetMousePos();
		posOffset = {
			currentCursorPosition.y - MouseControl::previousCursorPosition.y,
			currentCursorPosition.x - MouseControl::previousCursorPosition.x
		};
		MouseControl::previousCursorPosition = { currentCursorPosition.x, currentCursorPosition.y };
	}
	float xRot = CLAMP(posOffset.x, -1.0f, 1.0f);
	float yRot = CLAMP(posOffset.y, -1.0f, 1.0f);
	

	LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("X: " + STR(xRot) + " Y: " + STR(yRot)));

	xRot = xRot * rotSpeed * -dt;
	yRot = yRot * rotSpeed * -dt;

	camData->mView *= DirectX::XMMatrixRotationX(xRot);
	camData->mView *= DirectX::XMMatrixRotationY(yRot);
}
