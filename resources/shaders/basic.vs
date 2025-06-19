#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
// Note: vertexTangent may not be available in Raylib's default sphere mesh

// Input uniform values
uniform mat4 mvp2;
uniform mat4 matModel2;  // Model matrix for transforming vertices and normals

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragPosition;
out vec3 fragNormal;
out mat3 TBN;           // Tangent-Bitangent-Normal matrix for normal mapping

void main()
{
    // Transform vertex position and normal by model matrix
    vec4 worldPosition = matModel2 * vec4(vertexPosition, 1.0);
    
    // Normal matrix: transpose of the inverse of the upper-left 3x3 part of the model matrix
    mat3 normalMatrix = transpose(inverse(mat3(matModel2)));
    vec3 worldNormal = normalize(normalMatrix * vertexNormal);
    
    // Calculate tangent space for sphere
    // For a sphere, we can derive the tangent from the normal and texture coordinates
    vec3 c1 = cross(worldNormal, vec3(0.0, 0.0, 1.0));
    vec3 c2 = cross(worldNormal, vec3(0.0, 1.0, 0.0));
    
    // Use the non-zero cross product with largest magnitude
    vec3 worldTangent = normalize(length(c1) > length(c2) ? c1 : c2);
    vec3 worldBitangent = normalize(cross(worldNormal, worldTangent));
    
    // Create TBN matrix
    TBN = mat3(worldTangent, worldBitangent, worldNormal);
    
    // Send vertex attributes to fragment shader
    fragTexCoord = vec2(vertexTexCoord.x, vertexTexCoord.y);
    fragColor = vertexColor;
    fragPosition = worldPosition.xyz;
    fragNormal = worldNormal;
    
    // Calculate final vertex position
    gl_Position = mvp2 * vec4(worldPosition.xyz, 1.0);
}
