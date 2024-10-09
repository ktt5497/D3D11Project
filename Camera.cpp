#include "Camera.h"
#include "Input.h"
#include <algorithm>

using namespace DirectX;

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 initialPosition, DirectX::XMFLOAT3 inOrientation,
    float moveSpeed, float mouseSpeed, float fov, float nearClip, float farClip, bool isOrtho)
    :moveSpeed(moveSpeed), mouseSpeed(mouseSpeed), fov(fov), nearClip(nearClip), farClip(farClip), isOrtho(isOrtho)
{
    transform.SetPosition(initialPosition.x, initialPosition.y, initialPosition.z);
    transform.SetRotation(XMConvertToRadians(inOrientation.x), XMConvertToRadians(inOrientation.y), XMConvertToRadians(inOrientation.z));

    UpdateProjectionMatrix(aspectRatio);
    UpdateViewMatrix();
}

Camera::~Camera()
{
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
    return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjMatrix()
{
    return projectionMatrix;
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(fov, aspectRatio, nearClip, farClip));
}

void Camera::UpdateViewMatrix()
{
    XMFLOAT3 pos = transform.GetPosition();
    XMVECTOR position = XMLoadFloat3(&pos);

    XMFLOAT3 fwd = transform.GetFoward();
    XMVECTOR forward = XMLoadFloat3(&fwd);

    XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);

    XMStoreFloat4x4(&viewMatrix, XMMatrixLookToLH(position, forward, worldUp));
}

void Camera::Update(float dt)
{
    //Keyboard input
    if (Input::KeyDown('W')) transform.MoveRelative(0.0f, 0.0f, moveSpeed * dt);
    if (Input::KeyDown('S')) transform.MoveRelative(0.0f, 0.0f, -moveSpeed * dt);
    if (Input::KeyDown('A')) transform.MoveRelative(-moveSpeed * dt, 0.0f, 0.0f);
    if (Input::KeyDown('D')) transform.MoveRelative(moveSpeed * dt, 0.0f, 0.0f);
    if (Input::KeyDown(VK_SPACE)) transform.MoveRelative(0.0f, moveSpeed * dt, 0.0f);
    if (Input::KeyDown('X')) transform.MoveRelative(0.0f, -moveSpeed * dt, 0.0f);

    //mouse input
    if (Input::MouseLeftDown()) 
    {
        float cursorMovementX = Input::GetMouseXDelta();
        float cursorMovementY = Input::GetMouseYDelta();

        float x = cursorMovementX * mouseSpeed * dt;
        float y = cursorMovementY * mouseSpeed * dt;
        transform.Rotate(y, x, 0);

        //Clamping
        float xRot = transform.GetPitchYawRoll().x;
        if (xRot > XM_PIDIV2) {
            xRot = XM_PIDIV2;
        }
        else if (xRot < -XM_PIDIV2) {
            xRot = -XM_PIDIV2;
        }
        transform.SetRotation(xRot, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
    }
    
    UpdateViewMatrix();
}
