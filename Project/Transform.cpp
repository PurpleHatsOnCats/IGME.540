#include "Transform.h"

Transform::Transform()
{
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
	UpdateDirections();
	isWorldDirty = false;
}

// ------------------------------------Setters------------------------------------ //

void Transform::SetPosition(float x, float y, float z)
{
	SetPosition(XMFLOAT3(x, y, z));
}

void Transform::SetPosition(XMFLOAT3 position)
{
	Transform::position = position;
	isWorldDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	SetRotation(XMFLOAT3(pitch, yaw, roll));
}

void Transform::SetRotation(XMFLOAT3 rotation)
{
	Transform::rotation = rotation;
	isWorldDirty = true;
	UpdateDirections();
}

void Transform::SetScale(float x, float y, float z)
{
	SetScale(XMFLOAT3(x, y, z));
}

void Transform::SetScale(XMFLOAT3 scale)
{
	Transform::scale = scale;
	isWorldDirty = true;
}

void Transform::UpdateDirections()
{
	// Set right, up, forward vectors
	XMStoreFloat3(&right, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
	XMStoreFloat3(&up, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
	XMStoreFloat3(&forward, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
}

// ------------------------------------Getters------------------------------------ //

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

XMFLOAT3 Transform::GetRight()
{
	return right;
}

XMFLOAT3 Transform::GetUp()
{
	return up;
}

XMFLOAT3 Transform::GetForward()
{
	return forward;
}

// ------------------------------------Transformers------------------------------------ //

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
	UpdateDirections();
}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	XMVECTOR rot = XMLoadFloat3(&(Transform::rotation));
	rot += XMLoadFloat3(&rotation);
	XMStoreFloat3(&(Transform::rotation), rot);
	isWorldDirty = true;
	UpdateDirections();
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

void Transform::MoveRelative(float x, float y, float z)
{
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMVector3Rotate(XMVectorSet(x,y,z,0), XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
}

void Transform::MoveRelative(XMFLOAT3 offset)
{
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMVector3Rotate(XMLoadFloat3(&offset), XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)));
}
