#include "raylib.h"
#include "raymath.h"
#include "CelestialBody.h"

int main() 
{
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Celestial Bodies Renderer");
    
    // Define the camera
    Camera3D camera = { 0 };
    camera.position = Vector3{ -1.0f, 1.0f, -3.0f };  // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };    // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };        // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type
    
    // Define the light position (in world space)
    Vector3 lightPos = { 5.0f, 3.0f, 0.0f };
    
    // Create Earth celestial body
    CelestialBody earth("Earth", 1.0f, 10.0f); // Name, radius, rotation speed
    earth.Initialize(
        "resources/model/sphere.glb",
        "resources/images/2k_earth_daymap.png",
        "resources/images/2k_earth_normal_map.png",
        "resources/images/2k_earth_specular_map.png",
        "resources/images/2k_earth_nightmap.png",
        "resources/images/2k_earth_clouds.png"
    );
    
    // Create Moon celestial body
    CelestialBody moon("Moon", 0.27f, 3.0f); // Name, radius (27% of Earth), rotation speed
    moon.Initialize(
        "resources/model/sphere.glb",
       "resources/images/Moon.Diffuse.png",
        "resources/images/Moon.Normal.png"
    );
    moon.SetScale(0.27f); // Set moon scale to 27% of Earth's size
    moon.SetOrbit(&earth, 1.5f, 5.0f, 5.0f); // Parent, distance, speed, tilt
    

    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {        
        // Update celestial bodies
        float deltaTime = GetFrameTime();
        earth.Update(deltaTime);
        moon.Update(deltaTime);
        
        // Update shader values with current camera and light positions
        earth.UpdateShaderValues(camera, lightPos);
        moon.UpdateShaderValues(camera, lightPos);
        
        // Draw
        BeginDrawing();
        
            ClearBackground(BLACK);            
            BeginMode3D(camera);
                earth.Draw(camera);
                moon.Draw(camera);
            EndMode3D();
            
            DrawFPS(10, 50);
        
        EndDrawing();
    }
    
    // No need to manually unload textures and models, the CelestialBody destructor will handle it
    CloseWindow();     // Close window and OpenGL context
    
    return 0;
}
