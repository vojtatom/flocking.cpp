#version 150


//in range 0 - 1
in float colorVal;
out vec4 out_color;
 
void main() {
	float c = colorVal;
	float r = pow(c, 0.5);
	float g = 1 - max(r - 0.5, r);//max(colorVal, 1.0 - colorVal);
	float b = 1.0 - pow(c, 1);//1 - pow(colorVal, 0.1);
	out_color = vec4(r, g, b, 1.0);
}