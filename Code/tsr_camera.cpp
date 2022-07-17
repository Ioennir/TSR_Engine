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
}

void TSR_RecalculateCameraVectors(DirectX::XMFLOAT3 UpDir, DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Target)
{
	// Store Up vector
	DirectX::XMFLOAT3 Up = TSR_DX_NormalizeFLOAT3(UpDir);
	CameraControl::camUp = { Up.x, Up.y, Up.z, 0.0f };
	LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("UP ::: X: " + STR(Up.x) + " Y: " + STR(Up.y) + " Z: " + STR(Up.z)));

	// Calculate Forward vector which is equal to target vector.
	CameraControl::camTarget = { Target.x, Target.y, Target.z, 1.0f };
	DirectX::XMFLOAT3 Forward = Target - Position;
	Forward = TSR_DX_NormalizeFLOAT3(Forward);
	CameraControl::camFwd = { Forward.x, Forward.y, Forward.z, 0.0f };

	LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("FW ::: X: " + STR(Forward.x) + " Y: " + STR(Forward.y) + " Z: " + STR(Forward.z)));

	// Calculate Right vector which is equal to Up cross Forward

	DirectX::XMFLOAT3 Right;
	DirectX::XMStoreFloat3(&Right, DirectX::XMVector3Cross({ Up.x, Up.y, Up.z }, { Forward.x, Forward.y, Forward.z }));
	CameraControl::camRight = { Right.x, Right.y, Right.z, 0.0f };

	LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("RI ::: X: " + STR(Right.x) + " Y: " + STR(Right.y) + " Z: " + STR(Right.z)));

}

void TSR_InitCamera(DirectX::XMFLOAT3 Position, DirectX::XMFLOAT3 Target, DirectX::XMFLOAT3 UpDir, float Fov, float AspectRatio)
{
	// Store AspectRatio and Fov
	CameraControl::CamData.AspectRatio = AspectRatio;
	CameraControl::CamData.Fov = Fov;

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

void InitializeCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up, float fov, float aspectRatio)
{
	CameraControl::CamData.AspectRatio = aspectRatio;
	CameraControl::CamData.Fov = fov;
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
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookToLH(cEye, cFocus, cUp);//DirectX::XMMatrixLookAtLH(cEye, cFocus, cUp);

	// frustum data
	float yFov = DirectX::XMConvertToRadians(fov);
	float nearZ = 0.1f;
	float farZ = 1000.0f;
	CameraControl::CamData.NearClipPlane = nearZ;
	CameraControl::CamData.FarClipPlane = farZ;
	DirectX::XMMATRIX mProj = DirectX::XMMatrixPerspectiveFovLH(yFov, aspectRatio, nearZ, farZ);

	// world mat
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();

	CameraControl::CamData.mWorld = mWorld;
	CameraControl::CamData.mView = mView;
	CameraControl::CamData.mProj = mProj;
}

//https://stackoverflow.com/questions/36779161/trap-cursor-in-window
// Fix the camera rotation
void UpdateCamera(float dt, CameraData * CamData)
{
	const float rotSpeed = 30.0f;
	const float movSpeed = 15.0f;
	// Fetch input from keyboard
	
	//rotation is broken.
	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		// Fetch and process mouse input
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		ImVec2 dir = { 0.0f, 0.0f };
		ImVec2 mouseDrag = ImGui::GetMousePos();//ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
		ImVec2 prev = { MouseControl::prevDrag.x, MouseControl::prevDrag.y };
		if (mouseDrag.x > prev.x)
		{
			dir.x = -1.0f;
		}
		else if (mouseDrag.x < prev.x)
		{
			dir.x = 1.0f;
		}

		if (mouseDrag.y > prev.y)
		{
			dir.y = -1.0f;
		}
		else if (mouseDrag.y < prev.y)
		{
			dir.y = 1.0f;
		}
		MouseControl::prevDrag = { mouseDrag.x, mouseDrag.y};

		float UpAxisRot = dir.x * rotSpeed * dt;
		float RightAxisRot = dir.y * rotSpeed * dt;
		//UpAxisRot = 0.0f;
		
		LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("X: " + STR(UpAxisRot) + " Y: " + STR(RightAxisRot)));
		//NOTE(Fran): rotation is funky sometimes, doing rombo movement shows the issue (diagonals)
		
		//DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(yRot, xRot, 0.0f);
		//CamData->mView *= rotMat;

		// Get the local rotation axis
		DirectX::XMVECTOR f = DirectX::XMVector4Normalize(DirectX::XMVector4Transform({ 0.0f, 0.0f, 1.0f, 0.0f }, CamData->mView));
		DirectX::XMStoreFloat4(&CameraControl::camFwd, f);
		DirectX::XMVECTOR r = DirectX::XMVector4Normalize(DirectX::XMVector4Transform({ 1.0f, 0.0f, 0.0f, 0.0f }, CamData->mView));
		DirectX::XMStoreFloat4(&CameraControl::camRight, r);
		DirectX::XMVECTOR u = DirectX::XMVector4Normalize(DirectX::XMVector4Transform({ 0.0f, 1.0f, 0.0f, 0.0f }, CamData->mView));
		DirectX::XMStoreFloat4(&CameraControl::camUp, u);
		
		// y rotations will always be around world y axis XD
		DirectX::XMMATRIX RotY = DirectX::XMMatrixRotationAxis(u, UpAxisRot);
		DirectX::XMMATRIX RotX = DirectX::XMMatrixRotationAxis(r, RightAxisRot);

		CamData->mView *= RotX;
		//CamData->mView *= RotY;

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

	DirectX::XMMATRIX mTranslation = DirectX::XMMatrixTranslation(xMov, yMov, zMov);
	DirectX::XMVECTOR cEye = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&CameraControl::camPosition), mTranslation);
	DirectX::XMVECTOR cTarget = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&CameraControl::camTarget), mTranslation);
	DirectX::XMVECTOR cUp = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&CameraControl::camUp), mTranslation);

	DirectX::XMStoreFloat4(&CameraControl::camPosition, cEye);
	DirectX::XMStoreFloat4(&CameraControl::camTarget, cTarget);
	DirectX::XMStoreFloat4(&CameraControl::camUp, cUp);

	CamData->mView = DirectX::XMMatrixLookAtLH(cEye, cTarget, cUp);

	//CamData->mView *= DirectX::XMMatrixTranslation(xMov, yMov, zMov);

	

	//CamData->mView = DirectX::XMMatrixLookAtLH();
}
