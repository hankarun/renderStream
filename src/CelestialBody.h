#ifndef CELESTIAL_BODY_H
#define CELESTIAL_BODY_H

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <memory>

class CelestialBody {
public:
    // Constructor/Destructor
    CelestialBody();
    CelestialBody(const std::string& name, float radius, float rotationSpeed);
    ~CelestialBody();

    // Initialize celestial body with model and textures
    void Initialize(const char* modelPath,
                  const char* diffuseMapPath,
                  const char* normalMapPath = nullptr,
                  const char* specularMapPath = nullptr,
                  const char* emissionMapPath = nullptr,
                  const char* cloudMapPath = nullptr);

    // Update the celestial body (rotation, position, etc.)
    void Update(float deltaTime);

    // Draw the celestial body
    void Draw(const Camera3D& camera);

    // Position and orientation setters/getters
    void SetPosition(const Vector3& position);
    Vector3 GetPosition() const;
    void SetRotationAxis(const Vector3& axis);
    void SetScale(float scale);

    // Orbit related methods
    void SetOrbit(CelestialBody* parent, float orbitDistance, float orbitSpeed, float orbitTilt);
    void UpdateOrbit(float deltaTime);

    // Shader related methods
    void SetCustomShader(Shader shader);
    void UpdateShaderValues(const Camera3D& camera, const Vector3& lightPos);

private:
    std::string name;
    float radius;
    float rotationSpeed;
    float rotationAngle;
    float scale;
    
    Vector3 position;
    Vector3 rotationAxis;
    
    // Orbit properties
    CelestialBody* orbitParent;
    float orbitDistance;
    float orbitSpeed;
    float orbitAngle;
    float orbitTilt;
    
    // 3D model and textures
    Model model;
    Texture2D diffuseTexture;
    Texture2D normalTexture;
    Texture2D specularTexture;
    Texture2D emissionTexture;
    Texture2D cloudTexture;
    
    // Shader data
    Shader shader;
    bool hasCustomShader;
    
    // Shader locations
    int mvpLoc;
    int modelLoc;
    int lightPosLoc;
    int viewPosLoc;
    int cloudMapLoc;
    

    int hasDiffuseMapLoc;
    int hasNormalMapLoc;
    int hasSpecularMapLoc;
    int hasEmissionMapLoc;
    int hasCloudMapLoc;


    // Helper methods
    void UnloadTextures();
    void SetupShaderLocations();
};

#endif // CELESTIAL_BODY_H
