#version 330 core

out vec4 FragColor;

#define MAXLIGHTS 3

in vec3 fPos;
in vec3 fNormal;
in vec2 fTexCoord;
in vec4 fPosLightSpace[MAXLIGHTS];

struct Material {
    sampler2D diffuseTex;
    sampler2D specularTex;

    float shininess;
    float specular;

	bool wireframe;

	vec3 color;
};
struct DirLight {
    vec3 direction;

	float intensity;

    vec3 color;
};

uniform vec3 ambientColor;

uniform vec3 viewPos;
uniform Material material;

uniform DirLight dirLights[MAXLIGHTS];

uniform int shadows = 1;
uniform sampler2D shadowMap[MAXLIGHTS];

float ShadowCalculation(vec4 fragPosLightSpace, int lightIndex, float shadowBias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap[lightIndex], projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	float shadow = 0.0;

	vec2 texelSize = 2.5 / textureSize(shadowMap[lightIndex], 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
		    float pcfDepth = texture(shadowMap[lightIndex], projCoords.xy + vec2(x, y) * texelSize).r; 
		    shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;       
		}    
	}
	shadow /= 9.0;

    return shadow;
}  

vec3 CalculateLight(DirLight dirLight, int index, vec3 fNormal, vec3 viewDir)
{
	// Directions
	vec3 lightDir = normalize(dirLight.direction);  
	
	// Diffuse Light
	float diff = max(dot(fNormal, lightDir), 0.0);
	vec3 diffuse = material.color * (dirLight.color * dirLight.intensity) * diff * texture(material.diffuseTex, fTexCoord).rgb;

	// Specular Light
	vec3 reflectDir = reflect(-lightDir, fNormal);  

	// Phong Shading
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess) * dirLight.intensity;
	vec3 specular = dirLight.color * spec * material.specular * texture(material.specularTex, fTexCoord).rgb;  

	float bias = max(0.01 * (1.0 - dot(fNormal, lightDir)), 0.002); 
	float shadow = ShadowCalculation(fPosLightSpace[index], index, bias);       

	return (diffuse + specular) * (1.0 - (shadow*shadows));
}

void main()
{
	vec3 result;
	if(!material.wireframe)
	{
		// Ambient Light
		vec3 ambient = material.color * ambientColor * texture(material.diffuseTex, fTexCoord).rgb;

		vec3 norm = normalize(fNormal);
		vec3 viewDir = normalize(viewPos - fPos);

		result = ambient;

		// Diffuse, Specular light and Shadowmapping
		for(int i = 0; i<MAXLIGHTS; i++)
			result += CalculateLight(dirLights[i], i, norm, viewDir);
	}
	else
	{
		//result = vec3(0.0, 0.9, 1.0);
		result = vec3(0.0);
	}

	FragColor = vec4(result, 1.0);
}