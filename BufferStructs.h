#pragma once
#include <d3d11.h>
#include <DirectXMath.h>


struct VertexShaderData {
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
};