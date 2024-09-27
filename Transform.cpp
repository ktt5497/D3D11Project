#include "Transform.h"

Transform::Transform()
{
	position = DirectX::XMFLOAT3(0, 0, 0);
	rotation = DirectX::XMFLOAT3(0, 0, 0);
	scale = DirectX::XMFLOAT3(1, 1, 1);

	DirectX::XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&worldInverseTranspose, DirectX::XMMatrixIdentity());
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
	position = DirectX::XMFLOAT3(x, y, z);
	UpdateWorld();
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
	UpdateWorld();
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
	UpdateWorld();
}

void Transform::SetScale(float x, float y, float z)
{
	scale = DirectX::XMFLOAT3(x, y, z);
	UpdateWorld();
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
	UpdateWorld();
}

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
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	return worldInverseTranspose;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	UpdateWorld();
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	this->position.x = offset.x;
	this->position.y = offset.y;
	this->position.z = offset.z;
	UpdateWorld();
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	DirectX::XMVECTOR rot = XMLoadFloat3(&rotation);
	rot = DirectX::XMVectorAdd(rot, DirectX::XMVectorSet(pitch, yaw, roll, 0.0f));
	XMStoreFloat3(&rotation, rot);
	UpdateWorld();
}

void Transform::Rotate(DirectX::XMFLOAT3 rotate)
{
	DirectX::XMVECTOR rot = XMLoadFloat3(&rotation);
	rot = DirectX::XMVectorAdd(rot, DirectX::XMVectorSet(rotate.x, rotate.y, rotate.z, 0.0f));
	XMStoreFloat3(&rotation, rot);
	UpdateWorld();
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	UpdateWorld();

}

void Transform::Scale(DirectX::XMFLOAT3 scaling)
{
	this->scale.x *= scaling.x;
	this->scale.y *= scaling.y;
	this->scale.z *= scaling.z;
	UpdateWorld();
}

void Transform::UpdateWorld()
{
	DirectX::XMMATRIX tr = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX rt = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation));
	DirectX::XMMATRIX sc = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

	DirectX::XMMATRIX world = sc * rt * tr;

	DirectX::XMStoreFloat4x4(&worldMatrix, world);
	DirectX::XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(world)));
}
