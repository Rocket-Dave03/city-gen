#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
  
uniform mat4 transform;
uniform float aspect_ratio;

void main()
{
	vec4 oPos = vec4(aPos, 1.0f);
    gl_Position = transform * oPos;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
} 

