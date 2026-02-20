#pragma once
#include <DirectXMath.h>
using namespace DirectX;
class Transform
{
private:
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMFLOAT4X4 world;
	XMFLOAT4X4 worldInverseTranspose;
	bool isWorldDirty;
	XMFLOAT3 right;
	XMFLOAT3 up;
	XMFLOAT3 forward;
	void UpdateDirections();
public:
	Transform();

	// Setters 
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 position);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(XMFLOAT3 rotation);
	void SetScale(float x, float y, float z);
	void SetScale(XMFLOAT3 scale);

	// Getters 
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetPitchYawRoll();
	XMFLOAT3 GetScale();
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT4X4 GetWorldInverseTransposeMatrix();
	XMFLOAT3 GetRight();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetForward();

	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);
	void MoveRelative(float x, float y, float z);
	void MoveRelative(XMFLOAT3 offset);

};

