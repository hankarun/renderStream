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
uniform vec3 viewPos;  // Camera position for specular calculation
uniform sampler2D diffuseMap;  // Earth day texture map
uniform sampler2D emissionMap;  // Earth night texture map (emissive)

void main()
{
    // Calculate the normal in world coordinates
    vec3 normal = normalize(fragNormal);
    
    // Calculate the light direction and distance
    vec3 lightDir = normalize(lightPos - fragPosition);
    
    // Calculate diffuse reflection (Lambert)
    float diff = max(dot(normal, lightDir), 0.0);
      // Sample texture color
    vec4 texColor = texture(diffuseMap, fragTexCoord);
    
    // Sample night texture (emissive)
    vec4 nightColor = texture(emissionMap, fragTexCoord);
    
    // Calculate ambient reflection
    vec4 ambient = vec4(0.1, 0.1, 0.1, 1.0) * texColor;
    
    // Calculate diffuse component
    vec4 diffuse = texColor * diff;
    
    // Calculate specular reflection (Phong)
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    
    // Shininess factor (higher = smaller, sharper highlight)
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec4 specular = vec4(0.5, 0.5, 0.5, 1.0) * spec;
      // Output final color (ambient + diffuse + specular)
    // Add emissive (night) component based on the reverse of the diffuse factor
    // This makes the night lights visible on the dark side of the earth
    float nightFactor = 1.0 - diff;
    vec4 emissive = nightColor * nightFactor * 0.0; // Boost the night lights a bit
    
    finalColor = ambient + diffuse + specular + emissive;
}
