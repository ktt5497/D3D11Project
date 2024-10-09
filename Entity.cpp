#include "Entity.h"
#include "Graphics.h"

Entity::Entity(std::shared_ptr<Mesh> mesh) : mesh(mesh)
{
	this->mesh = mesh;
}

Entity::~Entity()
{
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

Transform* Entity::GetTransform()
{
	return &object;
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer, std::shared_ptr<Camera> camera)
{
	//preparing world matrix for entity
	DirectX::XMFLOAT4X4 worldMatrix = object.GetWorldMatrix();

	//filling in the constant buffer with world matrix'
	VertexShaderData vsData = {};
	vsData.colorTint = DirectX::XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.world = object.GetWorldMatrix();
	vsData.viewMatrix = camera->GetViewMatrix();
	vsData.projectionMatrix = camera->GetProjMatrix();

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	Graphics::Context->Unmap(constantBuffer.Get(), 0);

	//Binding Constant Buffer
	Graphics::Context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	mesh->DrawMesh();



}
