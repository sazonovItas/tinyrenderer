#pragma once

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

class Camera {
  float pitch = glm::radians(45.0f);
  float yaw = glm::radians(45.0f);
  float radius = 10.0f;

  glm::vec3 center;

  float mnRadius = 1.0f;
  float mxRadius = 100.0f;
  glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

  void rotateYaw(float dAngle);
  void rotatePitch(float dAngle);

public:
  Camera();
  Camera(float radius);
  Camera(glm::vec3 center);
  Camera(glm::vec3 center, float radius);
  Camera(glm::vec3 center, float radius, glm::vec3 up);
  Camera(glm::vec3 center, float radius, float mnRadius, float mxRadius);
  Camera(glm::vec3 center, float radius, float mnRadius, float mxRadius,
         glm::vec3 up);

  void rotate(float yawAngle, float pitchAngle);
  void zoom(float delta);

  glm::mat4x4 view();
};

#endif
