#include "Camera.h"

Camera::Camera(float aspectRatio, XMFLOAT3 position) : Camera(aspectRatio, position, 70.0f, 0.01f, 70.0f, "Default Name") {}

Camera::Camera(float aspectRatio, XMFLOAT3 position, float fov, float nearplane, float farplane, const char* name)
{
	transform = Transform();
	transform.SetPosition(position);
	Camera::fov = fov;
	Camera::nearplane = nearplane;
	Camera::farplane = farplane;
	UpdateProjectionMatrix(aspectRatio);
	UpdateViewMatrix();

	moveSpeed = 2.0f;
	lookSensitivity = 0.005f;

	Camera::name = name;
}

Transform* Camera::GetTransform() 
{
	return &transform;
}

XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}
const char* Camera::GetName() 
{
	return name;
}

float Camera::GetFOV()
{
	return fov;
}

float Camera::GetNearplane()
{
	return nearplane;
}

float Camera::GetFarplane()
{
	return farplane;
}

float Camera::GetMoveSpeed()
{
	return moveSpeed;
}

float Camera::GetSensitivity()
{
	return lookSensitivity;
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(fov, aspectRatio, nearplane, farplane));
	Camera::aspectRatio = aspectRatio;
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	XMStoreFloat4x4(&viewMatrix, XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&forward), XMVectorSet(0, 1, 0, 0)));
}

void Camera::SetFOV(float fov)
{
	if (fov < 0.001f)
		Camera::fov = 0.001f;
	else
		Camera::fov = fov;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetNearplane(float nearplane)
{
	if (nearplane < 0.001f)
		Camera::nearplane = 0.001f;
	else if (nearplane >= farplane)
		nearplane = farplane - 0.01f;
	else
		Camera::nearplane = nearplane;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetFarPlane(float farplane)
{
	if (farplane <= nearplane)
		Camera::farplane = nearplane + 0.01f;
	else
		Camera::farplane = farplane;
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::SetMoveSpeed(float speed)
{
	if (speed < 0.001f)
		moveSpeed = 0.001f;
	else if (speed > 100.0f)
		moveSpeed = 100.0f;
	else
		moveSpeed = speed;
}

void Camera::SetSensitivity(float sensitivity)
{
	if (sensitivity < 0.001f)
		lookSensitivity = 0.001f;
	else if (sensitivity > 100.0f)
		lookSensitivity = 100.0f;
	else
		lookSensitivity = sensitivity;
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
			if (rotation.x > XM_PI/2.0f-0.001f) { // small offset or else camera flips (?idk why)
				transform.SetRotation(XM_PI/2.0f - 0.001f,rotation.y,rotation.z);
			}
			else if (rotation.x < -XM_PI/2.0f + 0.001f) {
				transform.SetRotation(-XM_PI/2.0f + 0.001f, rotation.y, rotation.z);
			}
		}
	}
	// Speed Input
	{
		if (Input::KeyDown(VK_LCONTROL)) {
			SetMoveSpeed(moveSpeed + Input::GetMouseWheel() * 0.2f * moveSpeed);
		}
	}
	UpdateViewMatrix();
}
