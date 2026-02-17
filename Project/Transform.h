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
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);

};

