#version 150

uniform mat4 projectionView;



in vec3 position;
in vec3 colour;

out vec3 outColour;

void main () {
  outColour = colour;
  gl_Position = projectionView * vec4(position, 1.0);
}