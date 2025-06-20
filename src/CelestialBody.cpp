#include "CelestialBody.h"

CelestialBody::CelestialBody()
    : name("Unnamed"), 
      radius(1.0f), 
      rotationSpeed(0.0f),
      rotationAngle(0.0f),
      scale(1.0f),
      position({0.0f, 0.0f, 0.0f}),
      rotationAxis({0.0f, 1.0f, 0.0f}), // Default rotation around Y axis
      orbitParent(nullptr),
      orbitDistance(0.0f),
      orbitSpeed(0.0f),
      orbitAngle(0.0f),
      orbitTilt(0.0f),
      hasCustomShader(false)
{
    // Initialize all textures to empty
    diffuseTexture = { 0 };
    normalTexture = { 0 };
    specularTexture = { 0 };
    emissionTexture = { 0 };
    cloudTexture = { 0 };
}

CelestialBody::CelestialBody(const std::string& name, float radius, float rotationSpeed)
    : name(name), 
      radius(radius), 
      rotationSpeed(rotationSpeed),
      rotationAngle(0.0f),
      scale(1.0f),
      position({0.0f, 0.0f, 0.0f}),
      rotationAxis({0.0f, 1.0f, 0.0f}), // Default rotation around Y axis
      orbitParent(nullptr),
      orbitDistance(0.0f),
      orbitSpeed(0.0f),
      orbitAngle(0.0f),
      orbitTilt(0.0f),
      hasCustomShader(false)
{
    // Initialize all textures to empty
    diffuseTexture = {0};
    normalTexture = {0};
    specularTexture = {0};
    emissionTexture = {0};
    cloudTexture = {0};
}

CelestialBody::~CelestialBody() {
    // Unload textures
    UnloadTextures();
    
    // Unload model and shader if we have a custom one
    if (model.meshCount > 0) {
        UnloadModel(model);
    }

    if (hasCustomShader) {
        UnloadShader(shader);
    }
}

void CelestialBody::Initialize(const char* modelPath,
                             const char* diffuseMapPath,
                             const char* normalMapPath,
                             const char* specularMapPath,
                             const char* emissionMapPath,
                             const char* cloudMapPath) {
    // Load model
    model = LoadModel(modelPath);
    
    // Load textures if paths are provided
    if (diffuseMapPath) {
        diffuseTexture = LoadTexture(diffuseMapPath);
        GenTextureMipmaps(&diffuseTexture);
    }
    
    if (normalMapPath) {
        normalTexture = LoadTexture(normalMapPath);
        GenTextureMipmaps(&normalTexture);
    }
    
    if (specularMapPath) {
        specularTexture = LoadTexture(specularMapPath);
        GenTextureMipmaps(&specularTexture);
    }
    
    if (emissionMapPath) {
        emissionTexture = LoadTexture(emissionMapPath);
        GenTextureMipmaps(&emissionTexture);
    }
    
    if (cloudMapPath) {
        cloudTexture = LoadTexture(cloudMapPath);
        GenTextureMipmaps(&cloudTexture);
    }

    // Initialize shader
    
    // Load default shader if custom shader isn't set
    if (!hasCustomShader) {
        shader = LoadShader("resources/shaders/basic.vs", "resources/shaders/basic.fs");
        SetupShaderLocations();
    }
    
    // Assign textures to model material
    if (diffuseTexture.id > 0) {
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = diffuseTexture;
    }
    
    if (normalTexture.id > 0) {
        model.materials[0].maps[MATERIAL_MAP_NORMAL].texture = normalTexture;
    }
    
    if (specularTexture.id > 0) {
        model.materials[0].maps[MATERIAL_MAP_SPECULAR].texture = specularTexture;
    }
    
    if (emissionTexture.id > 0) {
        model.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emissionTexture;
    }
    
    if (cloudTexture.id > 0) {
        // Using index 10 which is beyond the standard material map indices (same as main.cpp)
        model.materials[0].maps[10].texture = cloudTexture;
    }
    
    // Set shader to model
    model.materials[0].shader = shader;

    bool hasDiffuseMap = (diffuseTexture.id > 0);
    bool hasNormalMap = (normalTexture.id > 0);
    bool hasSpecularMap = (specularTexture.id > 0);
    bool hasEmissionMap = (emissionTexture.id > 0);
    bool hasCloudMap = (cloudTexture.id > 0);

    hasDiffuseMapLoc = GetShaderLocation(shader, "hasDiffuseMap");
    hasNormalMapLoc = GetShaderLocation(shader, "hasNormalMap");
    hasSpecularMapLoc = GetShaderLocation(shader, "hasSpecularMap");
    hasEmissionMapLoc = GetShaderLocation(shader, "hasEmissionMap");
    hasCloudMapLoc = GetShaderLocation(shader, "hasCloudMap");

    SetShaderValue(shader, hasDiffuseMapLoc, &hasDiffuseMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasNormalMapLoc, &hasNormalMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasSpecularMapLoc, &hasSpecularMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasEmissionMapLoc, &hasEmissionMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasCloudMapLoc, &hasCloudMap, SHADER_UNIFORM_INT);
}

void CelestialBody::SetCustomShader(Shader customShader) {
    // If we already had a custom shader, unload it
    if (hasCustomShader) {
        UnloadShader(shader);
    }
    
    shader = customShader;
    hasCustomShader = true;
    
    // Setup shader locations
    SetupShaderLocations();
    
    // Assign shader to model
    model.materials[0].shader = shader;
}

void CelestialBody::SetupShaderLocations() {
    // Get shader uniform locations
    mvpLoc = GetShaderLocation(shader, "mvp2");
    modelLoc = GetShaderLocation(shader, "matModel2");
    lightPosLoc = GetShaderLocation(shader, "lightPos");
    viewPosLoc = GetShaderLocation(shader, "viewPos");
    cloudMapLoc = GetShaderLocation(shader, "cloudMap");
    
    // Set up standard shader locations
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "diffuseMap");
    shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "emissionMap");
    shader.locs[SHADER_LOC_MAP_SPECULAR] = GetShaderLocation(shader, "specularMap");
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    
    // Set cloud map texture location explicitly
    shader.locs[SHADER_LOC_MAP_DIFFUSE + 10] = cloudMapLoc; // Using a custom slot
}

void CelestialBody::Update(float deltaTime) {
    // Update rotation
    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) rotationAngle -= 360.0f;
    
    // Update orbit if we have a parent
    if (orbitParent != nullptr) {
        UpdateOrbit(deltaTime);
    }
}

void CelestialBody::UpdateOrbit(float deltaTime) {
    if (orbitParent == nullptr) return;
    
    // Update orbit angle
    orbitAngle += orbitSpeed * deltaTime;
    if (orbitAngle > 360.0f) orbitAngle -= 360.0f;
    
    // Calculate new position based on orbit
    float x = orbitParent->GetPosition().x + orbitDistance * cosf(orbitAngle * DEG2RAD);
    float z = orbitParent->GetPosition().z + orbitDistance * sinf(orbitAngle * DEG2RAD);
    
    // Apply orbit tilt if needed
    if (orbitTilt != 0.0f) {
        // This is a simplified tilt calculation
        float tiltRadians = orbitTilt * DEG2RAD;
        float y = orbitParent->GetPosition().y + orbitDistance * sinf(orbitAngle * DEG2RAD) * sinf(tiltRadians);
        position = Vector3{ x, y, z };
    } else {
        position = Vector3{ x, orbitParent->GetPosition().y, z };
    }
}

void CelestialBody::Draw(const Camera3D& camera) {
    // Create rotation matrix for model
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
    
    // Create translation matrix
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);
    
    // Create scale matrix
    Matrix matScale = MatrixScale(scale, scale, scale);
    
    // Create model matrix by combining rotation, scale and translation
    Matrix matModel = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    
    // Set model matrix uniform
    SetShaderValueMatrix(shader, modelLoc, matModel);
    
    // Calculate and set MVP matrix
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    Matrix matProjection = MatrixPerspective(camera.fovy*DEG2RAD, 
                                           (float)GetScreenWidth()/(float)GetScreenHeight(),
                                           0.1f, 100.0f);
    Matrix mvp = MatrixMultiply(matView, matProjection);
    SetShaderValueMatrix(shader, mvpLoc, mvp);
    
    // Update cloud texture binding explicitly if we have clouds
    if (cloudTexture.id > 0) {
        SetShaderValueTexture(shader, cloudMapLoc, cloudTexture);
    }
    
    // Draw the model
    DrawModel(model, Vector3Zero(), 1.0f, WHITE);
}

void CelestialBody::UpdateShaderValues(const Camera3D& camera, const Vector3& lightPos) {
    // Update view position for specular calculations
    SetShaderValue(shader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
    
    // Update light position
    SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);

    int hasDiffuseMap = (diffuseTexture.id > 0);
    int hasNormalMap = (normalTexture.id > 0);
    int hasSpecularMap = (specularTexture.id > 0);
    int hasEmissionMap = (emissionTexture.id > 0);
    int hasCloudMap = (cloudTexture.id > 0);

    SetShaderValue(shader, hasDiffuseMapLoc, &hasDiffuseMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasNormalMapLoc, &hasNormalMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasSpecularMapLoc, &hasSpecularMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasEmissionMapLoc, &hasEmissionMap, SHADER_UNIFORM_INT);
    SetShaderValue(shader, hasCloudMapLoc, &hasCloudMap, SHADER_UNIFORM_INT);
}

void CelestialBody::SetPosition(const Vector3& newPosition) {
    position = newPosition;
}

Vector3 CelestialBody::GetPosition() const {
    return position;
}

void CelestialBody::SetRotationAxis(const Vector3& axis) {
    rotationAxis = axis;
}

void CelestialBody::SetScale(float newScale) {
    scale = newScale;
}

void CelestialBody::SetOrbit(CelestialBody* parent, float distance, float speed, float tilt) {
    orbitParent = parent;
    orbitDistance = distance;
    orbitSpeed = speed;
    orbitTilt = tilt;
    orbitAngle = 0.0f; // Reset orbit angle
}

void CelestialBody::UnloadTextures() {
    if (diffuseTexture.id > 0) UnloadTexture(diffuseTexture);
    if (normalTexture.id > 0) UnloadTexture(normalTexture);
    if (specularTexture.id > 0) UnloadTexture(specularTexture);
    if (emissionTexture.id > 0) UnloadTexture(emissionTexture);
    if (cloudTexture.id > 0) UnloadTexture(cloudTexture);
}
