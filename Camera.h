#pragma once
#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>

class Camera
{
public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 initialPosition, DirectX::XMFLOAT3 inOrientation, float moveSpeed, float mouseSpeed, float fov, float nearClip, float farClip, bool isOrtho );
	~Camera();

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjMatrix();

	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);


private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	float aspectRatio;
	float fov;
	float nearClip;
	float farClip;
	float moveSpeed;
	float mouseSpeed;
	bool isOrtho;

};

