#pragma once

#include <glm/glm.hpp>

namespace vkf {
class Framework;

class PerspectiveCamera {
public:
  PerspectiveCamera(
      Framework *framework,
      glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
      float fov = glm::radians(70.0f),
      glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f),
      float yaw = glm::radians(90.0f),
      float pitch = glm::radians(0.0f));
  virtual ~PerspectiveCamera(){};

  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix();
  void update();

  void setPos(glm::vec3 pos);
  glm::vec3 getPos() const;

  void setYaw(float yaw);
  float getYaw() const;

  void setPitch(float pitch);
  float getPitch() const;

  void setFov(float fov);
  float getFov() const;

  glm::vec3 getFront() const;
  glm::vec3 getRight() const;

private:
  Framework* framework;

  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 world_up;

  glm::mat4 projection;

  float yaw;
  float pitch;
  float fov;

  float near = 0.001f;
  float far = 300.0f;

  void updateProjection();
  void updateDirections();
};
} // namespace vkf
