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

// Texture samplers
uniform sampler2D diffuseMap;  // Earth day texture map
uniform sampler2D emissionMap;  // Earth night texture map (emissive)
uniform sampler2D specularMap;  // Earth specular map
uniform sampler2D normalMap;   // Earth normal map
uniform sampler2D cloudMap;    // Earth cloud map

// Texture availability flags
uniform bool hasDiffuseMap = false;
uniform bool hasEmissionMap = false;
uniform bool hasSpecularMap = false;
uniform bool hasNormalMap = false;
uniform bool hasCloudMap = false;

void main()
{
    // Define default values for missing textures
    vec4 defaultDiffuse = diffuseColor;
    vec4 defaultEmission = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 defaultSpecular = vec4(0.1, 0.1, 0.1, 1.0);
    vec3 defaultNormal = fragNormal;
    vec4 defaultCloud = vec4(0.0, 0.0, 0.0, 1.0);

    // Normal calculation - use normal map if available, otherwise use fragment normal
    vec3 normal;
    if (hasNormalMap) {
        // Sample normal map (convert from RGB to normal vector)
        vec3 normalMapValue = texture(normalMap, fragTexCoord).rgb;
        normalMapValue = normalMapValue * 2.0 - 1.0;
        
        // Adjust normal map strength (increase for more pronounced effect)
        float normalStrength = 2.0;
        normalMapValue.xy *= normalStrength;
        normalMapValue = normalize(normalMapValue);
        
        // Apply normal map by transforming the normal from tangent space to world space
        normal = normalize(TBN * normalMapValue);
    } else {
        normal = normalize(defaultNormal);
    }
    
    // Calculate the light direction and distance
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Sample texture color or use default if not available
    vec4 texColor = hasDiffuseMap ? texture(diffuseMap, fragTexCoord) : defaultDiffuse;
    
    // Sample cloud texture or use default if not available
    vec4 cloudColor = hasCloudMap ? texture(cloudMap, fragTexCoord) : defaultCloud;
    
    // Sample night texture (emissive) or use default if not available
    vec4 nightColor = hasEmissionMap ? texture(emissionMap, fragTexCoord) : defaultEmission;
    
    // Calculate ambient reflection (increased brightness)
    vec4 ambient = vec4(0.3, 0.3, 0.3, 1.0) * texColor * 0.1;
    
    // Calculate diffuse component
    vec4 diffuse = texColor * diff;
    
    // Calculate specular reflection (Phong)
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // Use the specular map to determine the specular intensity
    float specularIntensity;
    if (hasSpecularMap) {
        vec4 specularMapColor = texture(specularMap, fragTexCoord);
        specularIntensity = (specularMapColor.r + specularMapColor.g + specularMapColor.b) / 3.0;
    } else {
        specularIntensity = 0.0; // Default specular intensity
    }
    
    vec4 specular = vec4(0.5, 0.5, 0.5, 1.0) * spec * specularIntensity;
    
    // Reduce specular in cloudy areas if cloud map is available
    if (hasCloudMap) {
        specular = mix(specular, vec4(0.0, 0.0, 0.0, 1.0), cloudColor.r);
    }
    
    // Add emissive (night) component based on the reverse of the diffuse factor
    float nightFactor = 1.0 - diff;
    nightFactor = pow(nightFactor, 2.0); // Less sharp transition
    vec4 emissive = nightColor * nightFactor * 1.0; // Increased intensity

    // Calculate final color
    finalColor = ambient + diffuse + specular;
    
    // Add emission if available
    if (hasEmissionMap) {
        finalColor += emissive;
    }
    
    // Apply cloud effect if available
    if (hasCloudMap) {
        vec4 cloudAmbient = vec4(0.3, 0.3, 0.3, 1.0) * cloudColor * 0.2;
        finalColor = mix(finalColor, vec4(1.0, 1.0, 1.0, 1.0), cloudColor.r * diff + cloudAmbient.r);
    }
}