#pragma once
#include "Transform.h"
#include "Mesh.h"
#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "BufferStructs.h"


class Entity
{

public:
	Entity(std::shared_ptr<Mesh> mesh);
	~Entity();
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform();

	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer);

private:
	Transform object;
	std::shared_ptr<Mesh> mesh;


};

