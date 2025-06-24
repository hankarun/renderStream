#include "OrbitSystem.h"
#include "CelestialBody.h"

OrbitSystem::OrbitSystem()
    : orbitParent(nullptr),
      orbitDistance(0.0f),
      orbitSpeed(0.0f),
      orbitAngle(0.0f),
      orbitTilt(0.0f),
      calculatedPosition({0.0f, 0.0f, 0.0f}),
      isPaused(false)
{
}

void OrbitSystem::SetOrbit(CelestialBody* parent, float distance, float speed, float tilt) {
    orbitParent = parent;
    orbitDistance = distance;
    orbitSpeed = speed;
    orbitTilt = tilt;
    orbitAngle = 0.0f; // Reset orbit angle
    
    // Calculate initial position
    if (orbitParent != nullptr) {
        UpdateOrbit(0.0f);
    }
}

void OrbitSystem::UpdateOrbit(float deltaTime) {
    if (orbitParent == nullptr || isPaused) return;
    
    // Update orbit angle
    orbitAngle += orbitSpeed * deltaTime;
    if (orbitAngle > 360.0f) orbitAngle -= 360.0f;
    
    // Get parent position
    Vector3 parentPosition = orbitParent->GetPosition();
    
    // Calculate new position based on orbit
    float x = parentPosition.x + orbitDistance * cosf(orbitAngle * DEG2RAD);
    float z = parentPosition.z + orbitDistance * sinf(orbitAngle * DEG2RAD);
    
    // Apply orbit tilt if needed
    if (orbitTilt != 0.0f) {
        // This is a simplified tilt calculation
        float tiltRadians = orbitTilt * DEG2RAD;
        float y = parentPosition.y + orbitDistance * sinf(orbitAngle * DEG2RAD) * sinf(tiltRadians);
        calculatedPosition = Vector3{ x, y, z };
    } else {
        calculatedPosition = Vector3{ x, parentPosition.y, z };
    }
}

Vector3 OrbitSystem::GetOrbitalPosition() const {
    return calculatedPosition;
}

float OrbitSystem::GetOrbitDistance() const {
    return orbitDistance;
}

float OrbitSystem::GetOrbitSpeed() const {
    return orbitSpeed;
}

void OrbitSystem::SetOrbitSpeed(float speed) {
    orbitSpeed = speed;
}

float OrbitSystem::GetOrbitAngle() const {
    return orbitAngle;
}

float OrbitSystem::GetOrbitTilt() const {
    return orbitTilt;
}

CelestialBody* OrbitSystem::GetOrbitParent() const {
    return orbitParent;
}

bool OrbitSystem::HasParent() const {
    return orbitParent != nullptr;
}

void OrbitSystem::SetPaused(bool paused) {
    isPaused = paused;
}

bool OrbitSystem::IsPaused() const {
    return isPaused;
}
