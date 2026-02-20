#include "Camera.h"

Camera::Camera(float aspectRatio, XMFLOAT3 position) : Camera(aspectRatio, position, 70.0f, 0.01f, 70.0f) {}

Camera::Camera(float aspectRatio, XMFLOAT3 position, float fov, float nearplane, float farplane)
{
	transform = Transform();
	transform.SetPosition(position);
	Camera::fov = fov;
	Camera::nearplane = nearplane;
	Camera::farplane = farplane;
	UpdateProjectionMatrix(aspectRatio);
	UpdateViewMatrix();

	moveSpeed = 1.0f;
	lookSensitivity = 1.0f;
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	return XMFLOAT4X4();
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return XMFLOAT4X4();
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(fov, aspectRatio, nearplane, farplane));
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	XMStoreFloat4x4(&viewMatrix, XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&forward), XMVectorSet(0, 1, 0, 0)));
}

void Camera::Update(float dt)
{
	// Movement Input
	{
		// Main Directions
		if (Input::KeyDown('W')) { transform.MoveRelative(0, 0, moveSpeed * dt); }
		if (Input::KeyDown('S')) { transform.MoveRelative(0, 0, -moveSpeed * dt); }
		if (Input::KeyDown('A')) { transform.MoveRelative(-moveSpeed * dt, 0, 0); }
		if (Input::KeyDown('D')) { transform.MoveRelative(moveSpeed * dt, 0, 0); }

		// Up and Down
		if (Input::KeyDown(VK_SPACE)) { transform.MoveAbsolute(0, moveSpeed * dt, 0); }
		if (Input::KeyDown('C')) { transform.MoveAbsolute(0, -moveSpeed * dt, 0); }
	}
	// Look Input
	{
		if (Input::MouseRightDown()) 
		{ 
			int cursorMovementX = Input::GetMouseXDelta();
			int cursorMovementY = Input::GetMouseYDelta();

			transform.Rotate(cursorMovementY * lookSensitivity,cursorMovementX * lookSensitivity, 0);
			
			// Clamp pitch
			XMFLOAT3 rotation = transform.GetPitchYawRoll();
			if (rotation.x > XM_PI/2.0f) {
				transform.SetRotation(XM_PI/2.0f,rotation.y,rotation.z);
			}
			else if (rotation.x < -XM_PI/2.0f) {
				transform.SetRotation(-XM_PI/2.0f, rotation.y, rotation.z);
			}
		}
	}
	UpdateViewMatrix();
}
