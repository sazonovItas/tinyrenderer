#include "camera.h"

Camera::Camera() {}

Camera::Camera(float radius) { this->radius = radius; }

Camera::Camera(glm::vec3 center) { this->center = center; }

Camera::Camera(glm::vec3 center, float radius) {
  this->center = center;
  this->radius = radius;
}

Camera::Camera(glm::vec3 center, float radius, glm::vec3 up) {
  this->center = center;
  this->radius = radius;
  this->up = up;
}

Camera::Camera(glm::vec3 center, float radius, float mnRadius, float mxRadius) {
  this->center = center;
  this->radius = radius;
  this->mnRadius = mnRadius;
  this->mxRadius = mxRadius;
}

Camera::Camera(glm::vec3 center, float radius, float mnRadius, float mxRadius,
               glm::vec3 up) {
  this->center = center;
  this->radius = radius;
  this->mnRadius = mnRadius;
  this->mxRadius = mxRadius;
  this->up = up;
}

void Camera::rotateYaw(float dAngle) {
  this->yaw += dAngle;
  if (this->yaw > 2 * M_PI) {
    this->yaw -= 2 * M_PI;
  }

  if (this->yaw < -2 * M_PI) {
    this->yaw += 2 * M_PI;
  }
}

void Camera::rotatePitch(float dAngle) {
  this->pitch += dAngle;
  this->pitch = std::min(this->pitch, float(M_PI / 2.0f - 0.001f));
  this->pitch = std::max(this->pitch, float(-M_PI / 2.0f + 0.001f));
}

void Camera::rotate(float yawAngle, float pitchAngle) {
  rotateYaw(yawAngle);
  rotatePitch(pitchAngle);
}

void Camera::zoom(float delta) {
  this->radius += delta;
  this->radius = std::min(this->radius, this->mxRadius);
  this->radius = std::max(this->radius, this->mnRadius);
}

glm::mat4x4 Camera::view() {
  glm::vec3 position(cos(pitch) * sin(yaw) * radius, sin(pitch) * radius,
                     cos(pitch) * cos(yaw) * radius);
  return glm::lookAt(position, center, up);
}
