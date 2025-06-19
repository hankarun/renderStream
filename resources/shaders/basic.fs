#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

// Output fragment color
out vec4 finalColor;

// Uniform inputs
uniform vec3 lightPos;
uniform vec4 diffuseColor;

void main()
{
    // Calculate the normal in world coordinates
    vec3 normal = normalize(fragNormal);
    
    // Calculate the light direction and distance
    vec3 lightDir = normalize(lightPos - fragPosition);
    
    // Calculate diffuse reflection (Lambert)
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Calculate ambient reflection
    vec4 ambient = vec4(0.1, 0.1, 0.1, 1.0) * diffuseColor;
    
    // Calculate final color
    vec4 diffuse = diffuseColor * diff;
    
    // Output final color
    finalColor = ambient + diffuse;
}
