#version 150

in vec3 vert;

in vec3 low;
in vec3 high;

uniform mat4 view;
uniform mat4 projection;

void main() {

    vec3 rvert = (vert * (high - low)) + low; 

 	gl_Position = projection * view * vec4(rvert, 1.0);
}