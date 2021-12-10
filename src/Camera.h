#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <glew.h>
#include <glfw3.h>

#include <cmath>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class Camera
{
public:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right;
    glm::vec3 globalUp;

    glm::mat4 projection;

    bool orbital = true;
    float orbitalDistance = 1.5f;
    float maxOrbitalDistance = 10.0f;

    float fov = 70.0f;
    float maxFov = 120.0f;
    float minFov = 5.0f;

    float farClipplingPlane = 100.0f;
    float nearClippingPlane = 0.002f;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float speed = 100.0f;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));

    // Move the camera locally
    void Move(glm::vec3 local_delta_position);

    // Rotate the camera by 'delta_yaw' angles in the global y axis and by 'delta_pitch' in the local x axis
    void Rotate(float delta_yaw, float delta_pitch);

    // Increase/decrease the camera's fov by 'delta_fov' angles
    void Zoom(float delta_fov);

    // Calculate the view matrix and return it
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix(int& window_width, int& window_height);

    void UpdateVectors();
};
#endif