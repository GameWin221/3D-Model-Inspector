#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

//const float offset = 1.0 / 200.0;
uniform float gamma = 2.0f;
uniform float exposure = 1.0f;


vec3 average(vec3 a, vec3 b)
{
    return vec3
    (
        (a.x+b.x)/2.0f,
        (a.y+b.y)/2.0f,
        (a.z+b.z)/2.0f
    );
}

void main()
{ 
    /*
    vec2 offsets[9] = vec2[]
    (
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    float kernel[9] = float[](
        1,1,1,
        1,-8,1,
        1,1,1
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
    {
        col += sampleTex[i] * kernel[i];
    }
    */

    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    
    // Reinhard tone mapping
    //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));

    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0f);
}