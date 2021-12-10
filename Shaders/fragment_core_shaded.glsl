#version 330 core

out vec4 FragColor;

in vec3 fPos;
in vec3 fNormal;
in vec2 fTexCoord;
in vec4 fPosLightSpace;

struct Material {
    sampler2D diffuseTex;
    sampler2D specularTex;

    float shininess;
    float specular;

	bool wireframe;

	vec3 color;
};

struct PointLight {
    vec3 position;

	float intensity;

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
uniform PointLight pointLight;

#define MAXLIGHTS 3
uniform DirLight dirLights[MAXLIGHTS];

uniform int shadows = 1;

uniform sampler2D shadowMap;
float ShadowCalculation(vec4 fragPosLightSpace, float shadowBias)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
		    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
		    shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;       
		}    
	}
	shadow /= 9.0;

    return shadow;
}  

vec3 CalculateLight(DirLight dirLight, vec3 fNormal, vec3 viewDir)
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

	float bias = max(0.05 * (1.0 - dot(fNormal, lightDir)), 0.005);  

	float shadow = ShadowCalculation(fPosLightSpace, bias);       

	return (diffuse + specular) * (1.0 - (shadow*shadows));
}

void main()
{
	
	vec3 result;
	if(!material.wireframe)
	{
		// ambient
		vec3 ambient = ambientColor * texture(material.diffuseTex, fTexCoord).rgb;

		// Diffuse Lighting
		vec3 norm = normalize(fNormal);
		vec3 viewDir = normalize(viewPos - fPos);

		result = ambient;

		// Multishadow support trzeba dodac
		//for(int i = 0; i<MAXLIGHTS; i++)
			result += CalculateLight(dirLights[0], norm, viewDir);


	}
	else
	{
		result = vec3(0.0, 0.0, 1.0);
	}

	FragColor = vec4(result, 1.0);
}