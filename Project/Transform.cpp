#include "Transform.h"

Transform::Transform()
{
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
	isWorldDirty = false;
}

// ------------------------------------Setters------------------------------------

void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	isWorldDirty = true;
}

void Transform::SetPosition(XMFLOAT3 position)
{
	Transform::position = position;
	isWorldDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
	isWorldDirty = true;
}

void Transform::SetRotation(XMFLOAT3 rotation)
{
	Transform::rotation = rotation;
	isWorldDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	isWorldDirty = true;
}

void Transform::SetScale(XMFLOAT3 scale)
{
	Transform::scale = scale;
	isWorldDirty = true;
}

// ------------------------------------Getters------------------------------------

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (isWorldDirty) {
		XMMATRIX translationMatrix = XMMatrixTranslation(position.x, position.y, position.z);
		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
		XMMATRIX scalingMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;

		XMStoreFloat4x4(&world, worldMatrix);
		XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(worldMatrix)));
		isWorldDirty = false;
	}

	return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	GetWorldMatrix();
	return worldInverseTranspose;
}

// ------------------------------------Transformers------------------------------------

void Transform::MoveAbsolute(float x, float y, float z)
{
	XMVECTOR pos = XMLoadFloat3(&position);
	pos += XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&position, pos);
	isWorldDirty = true;
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	XMVECTOR pos = XMLoadFloat3(&position);
	pos += XMLoadFloat3(&offset);
	XMStoreFloat3(&position, pos);
	isWorldDirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR rot = XMLoadFloat3(&(Transform::rotation));
	rot += XMVectorSet(pitch, yaw, roll, 0);
	XMStoreFloat3(&(Transform::rotation), rot);
	isWorldDirty = true;
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	XMVECTOR rot = XMLoadFloat3(&(Transform::rotation));
	rot += XMLoadFloat3(&rotation);
	XMStoreFloat3(&(Transform::rotation), rot);
	isWorldDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	XMVECTOR scl = XMLoadFloat3(&(Transform::scale));
	scl *= XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&(Transform::scale), scl);
	isWorldDirty = true;
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{
	XMVECTOR scl = XMLoadFloat3(&(Transform::scale));
	scl *= XMLoadFloat3(&scale);
	XMStoreFloat3(&(Transform::scale), scl);
	isWorldDirty = true;
}
