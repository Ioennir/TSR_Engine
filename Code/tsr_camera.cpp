struct CameraData
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProj;
	float AspectRatio;
	float Fov;
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
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtLH(cEye, cFocus, cUp);

	// frustum data
	float yFov = DirectX::XMConvertToRadians(fov);
	float nearZ = 0.1f;
	float farZ = 1000.0f;
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

		float xRot = dir.x * rotSpeed * dt;
		float yRot = dir.y * rotSpeed * dt;

		
		//LOGDEBUG(LOGSYSTEM_TSR, TEXTMESSAGE("X: " + STR(xRot) + " Y: " + STR(yRot)));
		//NOTE(Fran): rotation is funky sometimes, doing rombo movement shows the issue (diagonals)
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(yRot, xRot, 0.0f);
		CamData->mView *= rotMat;

		// store them
		DirectX::XMVECTOR f = DirectX::XMVector4Transform({ 0.0f, 0.0f, 1.0f, 0.0f }, CamData->mView);
		DirectX::XMStoreFloat4(&CameraControl::camFwd, f);
		DirectX::XMVECTOR r = DirectX::XMVector4Transform({ 1.0f, 0.0f, 0.0f, 0.0f }, CamData->mView);
		DirectX::XMStoreFloat4(&CameraControl::camRight, r);
		DirectX::XMVECTOR u = DirectX::XMVector4Transform({ 0.0f, 1.0f, 0.0f, 0.0f }, CamData->mView);
		DirectX::XMStoreFloat4(&CameraControl::camUp, u);
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
	float xMov = -hormove * movSpeed * dt;
	float zMov = -fwdmove * movSpeed * dt;
	float yMov = -vermove * movSpeed * dt;
	
	CamData->mView *= DirectX::XMMatrixTranslation(xMov, yMov, zMov);
}
