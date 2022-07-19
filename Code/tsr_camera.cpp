struct CameraData
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProj;
	float AspectRatio;
	float Fov;
	float NearClipPlane;
	float FarClipPlane;
};

namespace MouseControl
{
	DirectX::XMFLOAT2 prevDrag = { 0.0f, 0.0f };

}

namespace CameraControl
{
	DirectX::XMFLOAT4 camPosition;
	DirectX::XMFLOAT4 camUp;
	DirectX::XMFLOAT4 camFwd;
	DirectX::XMFLOAT4 camTarget;
	DirectX::XMFLOAT4 camRight;
	CameraData CamData = {};

	DirectX::XMFLOAT3 camRotation;
}

void TSR_RecalculateCameraVectors(DirectX::XMFLOAT3 UpDir, DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Target)
{
	// Store Up vector
	DirectX::XMFLOAT3 Up = TSR_DX_NormalizeFLOAT3(UpDir);
	CameraControl::camUp = {Up.x, Up.y, Up.z, 0.0f };

	// Calculate Forward vector which is equal to target vector.
	CameraControl::camTarget = { Target.x, Target.y, Target.z, 1.0f };
	DirectX::XMFLOAT3 Forward = Target - Position;
	Forward = TSR_DX_NormalizeFLOAT3(Forward);
	CameraControl::camFwd = { Forward.x, Forward.y, Forward.z, 0.0f };

	// Calculate Right vector which is equal to Up cross Forward

	DirectX::XMFLOAT3 Right;
	DirectX::XMStoreFloat3(&Right, DirectX::XMVector3Cross({ Up.x, Up.y, Up.z }, { Forward.x, Forward.y, Forward.z }));
	CameraControl::camRight = { Right.x, Right.y, Right.z, 0.0f };

}

void TSR_InitCamera(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Target, DirectX::XMFLOAT3 UpDir, float Fov, float AspectRatio)
{
	// Store AspectRatio and Fov and camrotation
	CameraControl::CamData.AspectRatio = AspectRatio;
	CameraControl::CamData.Fov = Fov;
	CameraControl::camRotation = { 0.0f, 0.0f, 0.0f };

	// Store Position, w=1 point, w=0 vector
	CameraControl::camPosition = { Position.x, Position.y, Position.z, 1.0f };

	TSR_RecalculateCameraVectors(UpDir, Position, Target);

	DirectX::XMVECTOR cEye = DirectX::XMLoadFloat4(&CameraControl::camPosition);
	DirectX::XMVECTOR cFocus = DirectX::XMLoadFloat4(&CameraControl::camTarget);
	DirectX::XMVECTOR cUp = DirectX::XMLoadFloat4(&CameraControl::camUp);
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtLH(cEye, cFocus, cUp);

	float yFov = DirectX::XMConvertToRadians(Fov);
	float nearZ = 0.1f;
	float farZ = 1000.0f;
	CameraControl::CamData.NearClipPlane = nearZ;
	CameraControl::CamData.FarClipPlane = farZ;
	DirectX::XMMATRIX mProj = DirectX::XMMatrixPerspectiveFovLH(yFov, CameraControl::CamData.AspectRatio, nearZ, farZ);

	DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();

	CameraControl::CamData.mWorld = mWorld;
	CameraControl::CamData.mView = mView;
	CameraControl::CamData.mProj = mProj;
}

void UpdateCamera(float dt, CameraData * CamData)
{
	const float rotSpeed = 30.0f;
	const float movSpeed = 15.0f;
	// Fetch input from keyboard

	DirectX::XMMATRIX mTranslation = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX mRotation = DirectX::XMMatrixIdentity();;
	
	// Fetch and process mouse input
	ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	ImVec2 dir = { 0.0f, 0.0f };
	ImVec2 mouseDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
	ImVec2 prev = { MouseControl::prevDrag.x, MouseControl::prevDrag.y };
	if (mouseDrag.x > prev.x)
	{
		dir.x = 1.0f;
	}
	else if (mouseDrag.x < prev.x)
	{
		dir.x = -1.0f;
	}

	if (mouseDrag.y > prev.y)
	{
		dir.y = 1.0f;
	}
	else if (mouseDrag.y < prev.y)
	{
		dir.y = -1.0f;
	}
	MouseControl::prevDrag = { mouseDrag.x, mouseDrag.y};

	float UpAxisRot = dir.x * rotSpeed * dt;
	float RightAxisRot = dir.y * rotSpeed * dt;

	CameraControl::camRotation.x = (CameraControl::camRotation.x + RightAxisRot);
	CameraControl::camRotation.y = (CameraControl::camRotation.y + UpAxisRot) > 175.0f ? CameraControl::camRotation.y : (CameraControl::camRotation.y + UpAxisRot);

	float RightRot = CameraControl::camRotation.x;
	float UpRot = CameraControl::camRotation.y;

	mRotation *= DirectX::XMMatrixRotationRollPitchYaw(RightRot, UpRot, 0.0f);

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

	DirectX::XMFLOAT4 MovOffset = (CameraControl::camRight * xMov) + (CameraControl::camUp * yMov) + (CameraControl::camFwd * zMov);
	DirectX::XMFLOAT4 Movement = CameraControl::camPosition + MovOffset;

	mTranslation *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat4(&Movement));
	DirectX::XMMATRIX mRT = mRotation * mTranslation;

	DirectX::XMVECTOR cEye = DirectX::XMVector4Transform({0.0f, 0.0f, 0.0f, 1.0f}, mRT);
	DirectX::XMStoreFloat4(&CameraControl::camPosition, cEye);
	DirectX::XMVECTOR cTarget = DirectX::XMVector4Transform({ 0.0f, 0.0f, 1.0f, 1.0f }, mRT);
	DirectX::XMStoreFloat4(&CameraControl::camTarget, cTarget);
	DirectX::XMVECTOR cUp = DirectX::XMVector4Transform({ 0.0f, 1.0f, 0.0f, 0.0f }, mRT);
	DirectX::XMStoreFloat4(&CameraControl::camUp, cUp);

	TSR_RecalculateCameraVectors(
		{ CameraControl::camUp.x, CameraControl::camUp.y, CameraControl::camUp.z },
		{ CameraControl::camPosition.x, CameraControl::camPosition.y, CameraControl::camPosition.z },
		{ CameraControl::camTarget.x, CameraControl::camTarget.y, CameraControl::camTarget.z }
	);
	
	CamData->mView = DirectX::XMMatrixLookAtLH(
		cEye,
		cTarget,
		cUp
	);
}
