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
	DirectX::XMFLOAT4 defUp = { 0.0f, 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 defFwd = { 0.0f, 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT4 defRight = { 1.0f, 0.0f, 0.0f, 0.0f };

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
	DirectX::XMVECTOR Up = { up.x, up.y, up.z, 0.0f };
	DirectX::XMVECTOR Fwd = { target.x, target.y, target.z, 0.0f };
	DirectX::XMVECTOR Right = DirectX::XMVector3Cross(Up, Fwd);
	DirectX::XMStoreFloat4(&CameraControl::camRight, Right);

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
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		ImVec2 currentCursorPosition = ImGui::GetMousePos();
		ImVec2 posOffset = {
			MouseControl::previousCursorPosition.y - currentCursorPosition.y,
			MouseControl::previousCursorPosition.x - currentCursorPosition.x
		};
		float xRot = 0.0f;
		float yRot = 0.0f;
		if (abs(posOffset.x) > 15.0f)
		{
			MouseControl::previousCursorPosition = { currentCursorPosition.x, currentCursorPosition.y };
			xRot = CLAMP(posOffset.x, -1.0f, 1.0f) * -1.0f;
			xRot = xRot * rotSpeed * dt; // rotation around x axis
		}
		if (abs(posOffset.y) > 15.0f)
		{
			MouseControl::previousCursorPosition = { currentCursorPosition.x, currentCursorPosition.y };
			yRot = CLAMP(posOffset.y, -1.0f, 1.0f) * -1.0f;
			yRot = yRot * rotSpeed * dt; // rotation around y axis
		}
		//LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("X: " + STR(xRot) + " Y: " + STR(yRot)));
		

		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(xRot, yRot, 0.0f);

		DirectX::XMVECTOR cT = DirectX::XMLoadFloat4(&CameraControl::camTarget);
		cT = DirectX::XMVector3TransformCoord(cT, rotMat);
		
		DirectX::XMVECTOR cU = DirectX::XMLoadFloat4(&CameraControl::camUp);
		cU = DirectX::XMVector3TransformCoord(cU, rotMat);

		DirectX::XMVECTOR cF = DirectX::XMLoadFloat4(&CameraControl::camFwd);
		cF = DirectX::XMVector3TransformCoord(cF, rotMat);
		
		DirectX::XMVECTOR cR = DirectX::XMLoadFloat4(&CameraControl::camRight);
		cR = DirectX::XMVector3TransformCoord(cR, rotMat);
		
		DirectX::XMStoreFloat4(&CameraControl::camTarget, cT);
		DirectX::XMStoreFloat4(&CameraControl::camUp, cU);
		DirectX::XMStoreFloat4(&CameraControl::camFwd, cF);
		DirectX::XMStoreFloat4(&CameraControl::camRight, cR);

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
	
	DirectX::XMFLOAT3 xMovement =
	{
		xMov * CameraControl::camRight.x,
		xMov * CameraControl::camRight.y,
		xMov * CameraControl::camRight.z
	};

	DirectX::XMFLOAT3 yMovement =
	{
		yMov * CameraControl::camUp.x,
		yMov * CameraControl::camUp.y,
		yMov * CameraControl::camUp.z
	};

	DirectX::XMFLOAT3 zMovement =
	{
		zMov * CameraControl::camFwd.x,
		zMov * CameraControl::camFwd.y,
		zMov * CameraControl::camFwd.z
	};

	CameraControl::camPosition = {
		CameraControl::camPosition.x + xMovement.x + yMovement.x + zMovement.x,
		CameraControl::camPosition.y + xMovement.y + yMovement.y + zMovement.y,
		CameraControl::camPosition.z + xMovement.z + yMovement.z + zMovement.z,
		0.0f
	};
	
	CameraControl::camTarget = {
		CameraControl::camTarget.x + xMovement.x + yMovement.x + zMovement.x,
		CameraControl::camTarget.y + xMovement.y + yMovement.y + zMovement.y,
		CameraControl::camTarget.z + xMovement.z + yMovement.z + zMovement.z,
		0.0f
	};

	//camData->mView *= DirectX::XMMatrixTranslation(xMov, yMov, zMov);
	camData->mView = DirectX::XMMatrixLookAtLH(
		DirectX::XMLoadFloat4(&CameraControl::camPosition),
		DirectX::XMLoadFloat4(&CameraControl::camTarget),
		DirectX::XMLoadFloat4(&CameraControl::camUp)
	);
}
