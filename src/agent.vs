#version 150

in vec3 position;
in vec3 velocity;

uniform mat4 view;
uniform mat4 projection;

 
void main() {
 	gl_Position = projection * view * vec4(position + velocity, 1.0);
}