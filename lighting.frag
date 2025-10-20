//-----------------------------------------------------------------------------
// basic.frag 
// Fragment shader for Part 1 of this tutorial
//-----------------------------------------------------------------------------
#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D textureSampler;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float specularStrength;
uniform int useLighting;

void main()
{
    // Sample texture color
    vec4 objectColor = texture(textureSampler, TexCoord);
    
    // Skip lighting calculations if lighting is disabled
    if (useLighting == 0) {
        FragColor = objectColor;
        return;
    }
    
    // Ambient lighting - constant base illumination
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting - depends on surface normal and light direction
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting - shiny highlights
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Combine all lighting components with object color
    vec3 result = (ambient + diffuse + specular) * objectColor.rgb;
    FragColor = vec4(result, objectColor.a);
}