#ifndef ORBIT_SYSTEM_H
#define ORBIT_SYSTEM_H

#include "raylib.h"
#include "raymath.h"

// Forward declaration to avoid circular dependency
class CelestialBody;

class OrbitSystem {
public:
    // Constructor/Destructor
    OrbitSystem();
    ~OrbitSystem() = default;    // Initialize orbit parameters
    void SetOrbit(CelestialBody* parent, float distance, float speed, float tilt);
    
    // Update orbit calculations
    void UpdateOrbit(float deltaTime);
    
    // Get calculated position based on orbit
    Vector3 GetOrbitalPosition() const;
    
    // Pause/Resume orbit
    void SetPaused(bool paused);
    bool IsPaused() const;
    
    // Getters for orbit properties
    float GetOrbitDistance() const;
    float GetOrbitSpeed() const;
    float GetOrbitAngle() const;
    float GetOrbitTilt() const;
    CelestialBody* GetOrbitParent() const;
    
    // Check if this orbit system has a parent
    bool HasParent() const;

private:
    // Orbit properties
    CelestialBody* orbitParent;
    float orbitDistance;
    float orbitSpeed;
    float orbitAngle;
    float orbitTilt;
    Vector3 calculatedPosition;
    bool isPaused;
};

#endif // ORBIT_SYSTEM_H
