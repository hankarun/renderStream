#include "raylib.h"

int main() 
{
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Path Tracer");    // Define the camera
    Camera3D camera = { 0 };
    camera.position = Vector3{ 2.0f, 0.0f, 2.0f };  // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };    // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };        // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type
    
    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second
    
    Vector3 spherePosition = { 0.0f, 0.0f, 0.0f }; // Position of the sphere
    float sphereRadius = 1.0f;                    // Radius of the sphere
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        // TODO: Update your variables here
          // Draw
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            BeginMode3D(camera);
                DrawSphere(spherePosition, sphereRadius, RED);                
            EndMode3D();
            
            DrawText("Camera at (2,0,2) looking at (0,0,0)", 10, 40, 20, BLACK);
        
        EndDrawing();
    }
    
    // De-Initialization
    CloseWindow();     // Close window and OpenGL context
    
    return 0;
}
