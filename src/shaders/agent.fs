#version 150


//in range 0 - 1
in float colorVal;
out vec4 out_color;
 
void main() {
	//constants 800 and 400 are purely experimental to add depth shading
	float originalZ = max(max((gl_FragCoord.z / gl_FragCoord.w - 800) / 400, 1), 0);

	float c = colorVal;
	float r = 1.2 - pow(c, 1);//1 - pow(colorVal, 0.1);
	float b = 1.5 - max(r - 0.5, r);//max(colorVal, 1.0 - colorVal);
	float g = pow(c, 0.35);
	/*float r = pow(c, 0.5);
	float g = 1 - max(r - 0.5, r);//max(colorVal, 1.0 - colorVal);
	float b = 1.0 - pow(c, 1);//1 - pow(colorVal, 0.1);*/
	out_color = vec4(vec3(r, g, b) / originalZ, 1.0);
}