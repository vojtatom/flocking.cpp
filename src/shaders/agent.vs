#version 150

in vec3 position;
in vec3 velocity;
in vec3 vert;
in float colorValue;

uniform mat4 view;
uniform mat4 projection;
uniform float maxColorValue;
uniform float minColorValue;

out float colorVal;

/**
 * Create rotation matrix from field vector.
 * The returned matrix can rotate vector (1, 0, 0)
 * into the desired setup. Used to rotate glyphs according
 * to vecotr field
 */
mat4 getRotationMat(vec3 vector)
{
	vec3 unit = vec3(1, 0, 0);
	vec3 f = normalize(vector);
	vec3 cross = cross(f, unit);
	vec3 a = normalize(cross);
	float s = length(cross);
	float c = dot(f, unit);
	float oc = 1.0 - c;

	return mat4(oc * a.x * a.x + c,        oc * a.x * a.y - a.z * s,  oc * a.z * a.x + a.y * s,  0.0,
                oc * a.x * a.y + a.z * s,  oc * a.y * a.y + c,        oc * a.y * a.z - a.x * s,  0.0,
                oc * a.z * a.x - a.y * s,  oc * a.y * a.z + a.x * s,  oc * a.z * a.z + c,        0.0,
                0.0,                       0.0,                       0.0,                       1.0);

}

void main() {
	mat4 rot = getRotationMat(velocity);
	vec3 rvert = vec3(rot * vec4(vert, 1.0f));
 	gl_Position = projection * view * vec4(rvert + position, 1.0);


	colorVal = (clamp(colorValue, minColorValue, maxColorValue) - minColorValue) / max(1, (maxColorValue - minColorValue));
}