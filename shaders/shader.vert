#version 450

layout(set = 0, binding = 1) uniform uniformBuffer {
  mat4 model;
  mat4 view;
  mat4 proj;
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out vec3 color0;
layout(location = 1) out vec2 texCoord0;

void main() {
  gl_Position = proj * view * model * vec4(pos, 1.0);
  color0 = color;
  texCoord0 = texCoord;
}
