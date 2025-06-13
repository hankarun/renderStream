#include "raylib.h"

int main() 
{
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Path Tracer");
    
    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        // TODO: Update your variables here
        
        // Draw
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            DrawText("Welcome to Raylib!", 190, 200, 40, MAROON);
            DrawCircle(screenWidth/2, screenHeight/2 + 100, 50, DARKBLUE);
        
        EndDrawing();
    }
    
    // De-Initialization
    CloseWindow();     // Close window and OpenGL context
    
    return 0;
}
