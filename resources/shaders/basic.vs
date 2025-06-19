#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;  // Model matrix for transforming vertices and normals

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragPosition;
out vec3 fragNormal;

void main()
{
    // Transform vertex position and normal by model matrix
    vec4 worldPosition = matModel * vec4(vertexPosition, 1.0);
    
    // Normal matrix: transpose of the inverse of the upper-left 3x3 part of the model matrix
    mat3 normalMatrix = transpose(inverse(mat3(matModel)));
    vec3 worldNormal = normalize(normalMatrix * vertexNormal);
    
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragPosition = worldPosition.xyz;
    fragNormal = worldNormal;
    
    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
