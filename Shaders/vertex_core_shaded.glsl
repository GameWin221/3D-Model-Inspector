#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;
  
#define MAXLIGHTS 3

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;
out vec4 fPosLightSpace[MAXLIGHTS];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix[MAXLIGHTS];

void main()
{
   fTexCoord = vTexCoord;
   fPos = vec3(model * vec4(vPos, 1.0));
   fNormal = mat3(transpose(inverse(model))) * vNormal;  

   for(int i = 0; i < MAXLIGHTS; i++)
   {
      fPosLightSpace[i] = lightSpaceMatrix[i] * vec4(fPos, 1.0);
   }

   gl_Position = projection * view * model * vec4(vPos.xyz, 1.0);
}