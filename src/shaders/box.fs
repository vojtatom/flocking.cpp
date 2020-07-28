#version 150

uniform int darkMode;

out vec4 out_color;
 
void main() {
	if (bool(darkMode))
		out_color = vec4(0.8, 0.8, 0.8, 0.35);
	else
		out_color = vec4(0.2, 0.2, 0.2, 0.1);
}