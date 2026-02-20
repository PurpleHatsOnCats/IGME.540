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

	// Controls
	float moveSpeed;
	float lookSensitivity;
public:
	// Constructors
	Camera(float aspectRatio, XMFLOAT3 position);
	Camera(float aspectRatio, XMFLOAT3 position, float fov, float nearplane, float farplane);

	// Getters
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();

	// Setters
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();

	// Other
	void Update(float dt);
};

