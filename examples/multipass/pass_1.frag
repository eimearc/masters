#version 450 core

layout (input_attachment_index = 0, set=1, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set=1, binding = 1) uniform subpassInput inputDepth;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 light;

layout(location = 0) out vec4 outColor;

const vec3 ka = vec3(0.1);
const vec3 kd = vec3(1.0);
const vec3 ks = vec3(1.0);
const float specular_exponent = 1.0;
const vec3 la = vec3(1.0);
const vec3 ld = vec3(1.0);
const vec3 ls = vec3(1.0);

// From Jon Macey's lectures.
vec3 phongModel()
{
    vec3 normal = subpassLoad(inputColor).rgb;
	vec3 s = normalize(light - position);
	vec3 v = normalize(-position);
	vec3 r = reflect(-s, normal);
	vec3 ambient = vec3(la*ka);
	float sDotN = max(dot(s,normal), 0.0);
	vec3 diffuse = vec3(ld*kd*sDotN);
	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
	{
		spec = vec3(la * ka * pow(max(dot(r,v), 0.0), specular_exponent));
	}
	return ambient + diffuse + spec;
}

void main() {
    vec3 color = subpassLoad(inputColor).rgb;
    float depth = subpassLoad(inputDepth).r;

    // vec4 clip = vec4(gl_FragCoord.xy, subpassLoad(inputDepth).x, 1.0);

    outColor=vec4(phongModel(),0);
}