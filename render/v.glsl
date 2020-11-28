//modified from assignment 4
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec3 a_Normal;
uniform mat4 u_MvpMat;
uniform mat4 u_ModelMat;
uniform mat4 u_NormalMat;
out vec3 v_Position;
out vec3 v_Color;
out vec3 v_Normal;
void main() {
	gl_Position = u_MvpMat * vec4(a_Position, 1);
	v_Position = (u_ModelMat * vec4(a_Position, 1)).xyz;
	v_Color = a_Color;
	v_Normal = (u_NormalMat * vec4(a_Normal, 1)).xyz;
}

