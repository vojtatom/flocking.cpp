#version 150

in vec3 position;
in vec3 direction;
in float velocity;

uniform mat4 view;
uniform mat4 projection;

 
void main() {
 	gl_Position = projection * view * vec4(position + direction * velocity, 1.0);
}