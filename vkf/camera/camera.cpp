#include "camera.hpp"

#include "../framework/framework.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace vkf;

PerspectiveCamera::PerspectiveCamera(
    Framework *framework,
    glm::vec3 position,
    float fov,
    glm::vec3 up,
    float yaw,
    float pitch)
    : framework(framework) {
  this->pos = position;
  this->fov = fov;
  this->world_up = up;
  this->yaw = yaw;
  this->pitch = pitch;

  this->updateDirections();
  this->updateProjection();
}

glm::mat4 PerspectiveCamera::getViewMatrix() {
  // return glm::lookAt(pos, pos + front, up);
  return glm::lookAt(this->pos, this->pos + this->front, this->up);
}

glm::mat4 PerspectiveCamera::getProjectionMatrix() {
  return this->projection;
}

void PerspectiveCamera::update() {
  this->updateDirections();
  this->updateProjection();
}

void PerspectiveCamera::setPos(glm::vec3 pos) {
  this->pos = pos;
}

glm::vec3 PerspectiveCamera::getPos() const {
  return this->pos;
}

void PerspectiveCamera::setYaw(float yaw) {
  this->yaw = yaw;
}

float PerspectiveCamera::getYaw() const {
  return yaw;
}

void PerspectiveCamera::setPitch(float pitch) {
  if (pitch > 89.0f)
    this->pitch = 89.0f;
  else if (pitch < -89.0f)
    this->pitch = -89.0f;
  else
    this->pitch = pitch;
}

float PerspectiveCamera::getPitch() const {
  return this->pitch;
}

void PerspectiveCamera::setFov(float fov) {
  this->fov = fov;
}

float PerspectiveCamera::getFov() const {
  return this->fov;
}

glm::vec3 PerspectiveCamera::getFront() const {
  return this->front;
}

glm::vec3 PerspectiveCamera::getRight() const {
  return this->right;
}

void PerspectiveCamera::updateProjection() {
  auto width = (float)this->framework->getWindow()->getWidth();
  auto height = (float)this->framework->getWindow()->getHeight();
  this->projection = glm::perspective(fov, width / height, near, far);
}

void PerspectiveCamera::updateDirections() {
  glm::vec3 front;
  front.x = cos(yaw) * cos(pitch);
  front.y = sin(pitch);
  front.z = sin(yaw) * cos(pitch);
  this->front = glm::normalize(front);

  this->right = glm::normalize(glm::cross(front, world_up));
  this->up = glm::normalize(glm::cross(right, front));
}
