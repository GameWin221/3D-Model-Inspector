#include "Camera.h"

Camera::Camera(glm::vec3 _position)
{
    position = _position;
    globalUp = up;
    UpdateVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix(int& window_width, int& window_height)
{
    glm::mat4 a = glm::perspective(glm::radians(fov), (float)window_width / (float)window_height, nearClippingPlane, farClipplingPlane);
    return a;
}

void Camera::Move(glm::vec3 local_delta_position)
{
    if (!orbital)   // If the camera is in free mode
    {
        float velocity = speed;
        position += ((local_delta_position.x * right) +
            (local_delta_position.y * up) +
            (local_delta_position.z * front)) / 100.0f * velocity;
    }
}
void Camera::Rotate(float delta_yaw, float delta_pitch)
{
    yaw += delta_yaw;
    pitch += delta_pitch;
    
    if (pitch > 89.99f)  pitch = 89.99f;
    if (pitch < -89.99f) pitch = -89.99f;
    
    UpdateVectors();
}
void Camera::Zoom(float delta_fov)
{
    fov -= delta_fov;
    if (fov < minFov) fov = minFov;
    if (fov > maxFov) fov = maxFov;
}

void Camera::UpdateVectors()
{
    // Front vector is the direction the camera is looking at
    glm::vec3 temp_front;
    temp_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    temp_front.y = sin(glm::radians(pitch));
    temp_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(temp_front);

    right = glm::normalize(glm::cross(front, globalUp));
    up = glm::normalize(glm::cross(right, front));

    if(orbital)
        this->position = -this->front * this->orbitalDistance;
}