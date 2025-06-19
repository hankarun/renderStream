#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in mat3 TBN;           // Tangent-Bitangent-Normal matrix for normal mapping

// Output fragment color
out vec4 finalColor;

// Uniform inputs
uniform vec3 lightPos;
uniform vec4 diffuseColor;
uniform vec3 viewPos;  // Camera position for specular calculation
uniform sampler2D diffuseMap;  // Earth day texture map
uniform sampler2D emissionMap;  // Earth night texture map (emissive)
uniform sampler2D specularMap;  // Earth specular map
uniform sampler2D normalMap;   // Earth normal map
uniform sampler2D cloudMap;    // Earth cloud map

void main()
{
    // Sample normal map (convert from RGB to normal vector)
    vec3 normalMap = texture(normalMap, fragTexCoord).rgb;
    // Transform from [0,1] range to [-1,1] range
    normalMap = normalMap * 2.0 - 1.0;
    
    // Adjust normal map strength (reduce for less pronounced effect)
    float normalStrength = 0.5;
    normalMap.xy *= normalStrength;
    normalMap = normalize(normalMap);
    
    // Apply normal map by transforming the normal from tangent space to world space
    vec3 normal = normalize(TBN * normalMap);
    
    // Calculate the light direction and distance
    vec3 lightDir = normalize(lightPos - fragPosition);    // Calculate diffuse reflection (Lambert) with a minimum ambient value
    float diff = max(dot(normal, lightDir), 0.0);
      // Sample texture color
    vec4 texColor = texture(diffuseMap, fragTexCoord);
      // Sample cloud texture
    vec4 cloudColor = texture(cloudMap, fragTexCoord);
    
    // Blend earth and clouds based on cloud texture brightness
    // Since clouds are white (or grayscale), we can use the red channel as the cloud density
    float cloudDensity = cloudColor.r * 0.8; // Adjust cloud intensity to 50% opacity
    
    // Blend the earth texture with white clouds
    texColor = mix(texColor, vec4(1.0, 1.0, 1.0, 1.0), cloudDensity);
    
    // Sample night texture (emissive)
    vec4 nightColor = texture(emissionMap, fragTexCoord);
      // Calculate ambient reflection (increased brightness)
    vec4 ambient = vec4(0.3, 0.3, 0.3, 1.0) * texColor;
    
    // Calculate diffuse component
    vec4 diffuse = texColor * diff;
    
    // Calculate specular reflection (Phong)
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
      // Shininess factor (higher = smaller, sharper highlight)
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // Use the specular map to determine the specular intensity
    vec4 specularMapColor = texture(specularMap, fragTexCoord);
    // Use the grayscale value from the specular map to modulate the specular intensity
    float specularIntensity = (specularMapColor.r + specularMapColor.g + specularMapColor.b) / 3.0;
    vec4 specular = vec4(0.5, 0.5, 0.5, 1.0) * spec * specularIntensity;      
    // Output final color (ambient + diffuse + specular)    
    // Add emissive (night) component based on the reverse of the diffuse factor
    // This makes the night lights visible on the dark side of the earth
    float nightFactor = 1.0 - diff;
    // Apply a stronger threshold to only show lights in very dark areas
    nightFactor = pow(nightFactor, 2.0); // Less sharp transition
    vec4 emissive = nightColor * nightFactor * 1.0; // Increased intensity
    
    finalColor = ambient + diffuse + specular + emissive;
}
