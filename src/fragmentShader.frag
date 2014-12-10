#version 150

in vec3 outColour;
out vec4 frag_colour;

void main () {
  frag_colour = vec4 (outColour, 1.0);
}