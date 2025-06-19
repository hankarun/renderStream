#include "raylib.h"
#include "raymath.h"

int main() 
{
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Path Tracer");
    
    // Define the camera
    Camera3D camera = { 0 };
    camera.position = Vector3{ 2.0f, 0.0f, 2.0f };  // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };    // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };        // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type
    
    // Load shader
    Shader shader = LoadShader("resources/shaders/basic.vs", "resources/shaders/basic.fs");
      // Get shader uniform locations
    int mvpLoc = GetShaderLocation(shader, "mvp");
    int lightPosLoc = GetShaderLocation(shader, "lightPos");
    int diffuseColorLoc = GetShaderLocation(shader, "diffuseColor");
    int viewPosLoc = GetShaderLocation(shader, "viewPos");
    
    // Define the light position (in world space)
    Vector3 lightPos = { 3.0f, 3.0f, 3.0f };
    SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
    
    // Define diffuse color
    Vector4 diffuseColor = { 1.0f, 0.0f, 0.0f, 1.0f };  // Red color
    SetShaderValue(shader, diffuseColorLoc, &diffuseColor, SHADER_UNIFORM_VEC4);
    
    // Create a model to render with the shader
    Model sphere = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));
    sphere.materials[0].shader = shader;
    
    Vector3 spherePosition = { 0.0f, 0.0f, 0.0f }; // Position of the sphere
    
    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second
      // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        //UpdateCamera(&camera, CAMERA_ORBITAL);
        
        // Update camera position in shader for specular lighting
        SetShaderValue(shader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
        
        Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
        Matrix matProjection = MatrixPerspective(camera.fovy*DEG2RAD, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        Matrix mvp = MatrixMultiply(matView, matProjection);
        SetShaderValueMatrix(shader, mvpLoc, mvp);
        
        // Draw
        BeginDrawing();
        
            ClearBackground(RAYWHITE);
            
            BeginMode3D(camera);
                DrawModel(sphere, spherePosition, 1.0f, WHITE);              
            EndMode3D();
            
            DrawText("Use mouse and WASD keys to move camera", 10, 20, 20, BLACK);
            DrawFPS(10, 50);
        
        EndDrawing();
    }
    
    // De-Initialization
    UnloadShader(shader);
    UnloadModel(sphere);
    CloseWindow();     // Close window and OpenGL context
    
    return 0;
}
