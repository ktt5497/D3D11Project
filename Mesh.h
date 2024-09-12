#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include "Vertex.h"

class Mesh
{
public:
	Mesh(const char* name, Vertex* vert, unsigned int totalVerts, unsigned int* indices, unsigned int totalIndices);
	~Mesh();
	Mesh(const Mesh&) = delete; // Remove copy constructor
	Mesh& operator=(const Mesh&) = delete; // Remove copy-assignment operator

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	const char* GetName();
	unsigned int GetVertexCount();
	void DrawMesh();

private:
	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> inBuffer;
	unsigned int totalIndices;
	unsigned int totalVertices;
	const char* name;
};

