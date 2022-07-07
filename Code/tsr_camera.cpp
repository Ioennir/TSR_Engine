struct CameraData
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProj;
};

namespace MouseControl
{
	DirectX::XMFLOAT2 previousCursorPosition = { 0.0f, 0.0f };

}

namespace CameraControl
{
	CameraData camData = {};
	DirectX::XMFLOAT4 camPosition;
	DirectX::XMFLOAT4 camTarget;
	DirectX::XMFLOAT4 camUp;
	DirectX::XMFLOAT4 camFwd;
	DirectX::XMFLOAT4 camRight;
}

void InitializeCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up, float fov, float aspectRatio)
{
	CameraControl::camPosition = { position.x, position.y, position.z, 0.0f };
	CameraControl::camUp = { up.x, up.y, up.z, 0.0f };
	CameraControl::camFwd = { target.x, target.y, target.z, 0.0f };
	CameraControl::camTarget = { target.x, target.y, target.z, 0.0f };
	CameraControl::camRight = { 1.0f, 0.0f, 0.0f, 0.0f };

	// camera position, target and up vector in 3D world
	DirectX::XMVECTOR cEye = DirectX::XMLoadFloat4(&CameraControl::camPosition);
	DirectX::XMVECTOR cFocus = DirectX::XMLoadFloat4(&CameraControl::camTarget);
	DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&CameraControl::camUp);

	// build view matrix
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtLH(cEye, cFocus, cUp);

	// frustum data
	float yFov = DirectX::XMConvertToRadians(fov);
	float nearZ = 0.1f;
	float farZ = 1000.0f;
	DirectX::XMMATRIX mProj = DirectX::XMMatrixPerspectiveFovLH(yFov, aspectRatio, nearZ, farZ);

	// world mat
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();

	CameraControl::camData.mWorld = mWorld;
	CameraControl::camData.mView = mView;
	CameraControl::camData.mProj = mProj;
}

// Fix the camera rotation
void UpdateCamera(float dt, CameraData * camData)
{
	const float rotSpeed = 15.0f;
	const float movSpeed = 15.0f;
	// Fetch input from keyboard
	
	//rotation is broken.
	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		ImVec2 currentCursorPosition = ImGui::GetMousePos();
		ImVec2 posOffset = {
			currentCursorPosition.y - MouseControl::previousCursorPosition.y,
			currentCursorPosition.x - MouseControl::previousCursorPosition.x
		};
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
		MouseControl::previousCursorPosition = { currentCursorPosition.x, currentCursorPosition.y };
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		float xRot = CLAMP(posOffset.x, -1.0f, 1.0f) * -1.0f;
		float yRot = CLAMP(posOffset.y, -1.0f, 1.0f) * -1.0f;

		LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("X: " + STR(xRot) + " Y: " + STR(yRot)));

		xRot = xRot * rotSpeed * dt;
		yRot = yRot * rotSpeed * dt;
		
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(xRot, yRot, 0.0f);
		DirectX::XMVECTOR cT = DirectX::XMLoadFloat4(&CameraControl::camTarget);
		DirectX::XMVECTOR cR = DirectX::XMLoadFloat4(&CameraControl::camRight);
		DirectX::XMVECTOR cF = DirectX::XMLoadFloat4(&CameraControl::camFwd);
		DirectX::XMVECTOR cU = DirectX::XMLoadFloat4(&CameraControl::camUp);

		cT = DirectX::XMVector3TransformCoord(cT, rotMat);
		cR = DirectX::XMVector3TransformCoord(cR, rotMat);
		cF = DirectX::XMVector3TransformCoord(cF, rotMat);
		cU = DirectX::XMVector3Cross(cF, cR);

		//cR = DirectX::XMVector4Normalize(cR);
		//cF = DirectX::XMVector4Normalize(cF);
		//cU = DirectX::XMVector4Normalize(cU);
		
		DirectX::XMStoreFloat4(&CameraControl::camTarget, cT);
		DirectX::XMStoreFloat4(&CameraControl::camRight, cR);
		DirectX::XMStoreFloat4(&CameraControl::camFwd, cF);
		DirectX::XMStoreFloat4(&CameraControl::camUp, cU);
	}

	// TODO(Fran): Check the disable obsolete keyIO
	bool forward = ImGui::IsKeyDown(ImGuiKey_W);
	bool back = ImGui::IsKeyDown(ImGuiKey_S);
	bool right = ImGui::IsKeyDown(ImGuiKey_D);
	bool left = ImGui::IsKeyDown(ImGuiKey_A);
	bool up = ImGui::IsKeyDown(ImGuiKey_R);
	bool down = ImGui::IsKeyDown(ImGuiKey_F);

	// convert input to axis value
	float fwdmove = (forward ? 1.0f : 0.0f) + (back ? -1.0f : 0.0f);
	float hormove = (right ? 1.0f : 0.0f) + (left ? -1.0f : 0.0f);
	float vermove = (up ? 1.0f : 0.0f) + (down ? -1.0f : 0.0f);

	//rebuild mView
	float xMov = hormove * movSpeed * dt;
	float zMov = fwdmove * movSpeed * dt;
	float yMov = vermove * movSpeed * dt;
	CameraControl::camPosition = {
		CameraControl::camPosition.x + xMov,
		CameraControl::camPosition.y + yMov,
		CameraControl::camPosition.z + zMov,
		0.0f
	};
	CameraControl::camTarget = {
		CameraControl::camTarget.x + xMov,
		CameraControl::camTarget.y + yMov,
		CameraControl::camTarget.z + zMov,
		0.0f
	};
	//camData->mView *= DirectX::XMMatrixTranslation(xMov, yMov, zMov);
	camData->mView = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat4(&CameraControl::camPosition),
		DirectX::XMLoadFloat4(&CameraControl::camTarget),
		DirectX::XMLoadFloat4(&CameraControl::camUp)
	);
}
