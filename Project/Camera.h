#pragma once

#include "Input.h"
#include "Transform.h"

class Camera
{
private:
	// Rendering
	Transform transform;
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	float fov;
	float nearplane;
	float farplane;
	float aspectRatio;

	// Controls
	float moveSpeed;
	float lookSensitivity;

	// Other
	const char* name;
public:
	// Constructors
	Camera(float aspectRatio, XMFLOAT3 position);
	Camera(float aspectRatio, XMFLOAT3 position, float fov, float nearplane, float farplane, const char* name);

	// Getters
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();
	Transform* GetTransform();
	const char* GetName();
	float GetFOV();
	float GetNearplane();
	float GetFarplane();
	float GetMoveSpeed();
	float GetSensitivity();

	// Setters
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void SetFOV(float fov);
	void SetNearplane(float nearplane);
	void SetFarPlane(float farplane);
	void SetMoveSpeed(float speed);
	void SetSensitivity(float sensitivity);


	// Other
	void Update(float dt);
};

