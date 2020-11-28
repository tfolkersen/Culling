//modified from assignment 4
#version 330 core
in vec3 v_Position;
in vec3 v_Color;
in vec3 v_Normal;
uniform vec3 u_LightPos;
uniform vec3 u_LightColor;
uniform vec3 u_AmbientLight;
out vec3 color;
void main() {
	vec3 normal = normalize(v_Normal);
	vec3 lightDir = normalize(u_LightPos - v_Position);
	float nDotL = max(0.0, dot(normal, lightDir));
	vec3 diffuse = u_LightColor * v_Color.rgb * nDotL;
	vec3 ambient = u_AmbientLight * v_Color.rgb;
	color = diffuse + ambient;
}
