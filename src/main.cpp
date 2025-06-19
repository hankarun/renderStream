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
    camera.position = Vector3{ 2.0f, 1.0f, 2.0f };  // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };    // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };        // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type      // Load earth textures
    Texture2D earthTexture = LoadTexture("resources/images/2k_earth_daymap.png");
    Texture2D nightTexture = LoadTexture("resources/images/2k_earth_nightmap.png");
    Texture2D specularTexture = LoadTexture("resources/images/2k_earth_specular_map.png");
    Texture2D normalTexture = LoadTexture("resources/images/2k_earth_normal_map.png");
    Texture2D cloudTexture = LoadTexture("resources/images/2k_earth_clouds.png");
    
    // Load shader
    Shader shader = LoadShader("resources/shaders/basic.vs", "resources/shaders/basic.fs");
    // Get shader uniform locations
    int mvpLoc = GetShaderLocation(shader, "mvp");    
    int modelLoc = GetShaderLocation(shader, "matModel"); // Add model matrix location
    int lightPosLoc = GetShaderLocation(shader, "lightPos");
    int diffuseColorLoc = GetShaderLocation(shader, "diffuseColor");
    int viewPosLoc = GetShaderLocation(shader, "viewPos");
    int cloudMapLoc = GetShaderLocation(shader, "cloudMap");
      // Define the light position (in world space) - moved to better illuminate the Earth
    Vector3 lightPos = { 5.0f, 3.0f, 5.0f };
    SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
      // Define diffuse color
    Vector4 diffuseColor = { 1.0f, 0.0f, 0.0f, 1.0f };  // Red color
    SetShaderValue(shader, diffuseColorLoc, &diffuseColor, SHADER_UNIFORM_VEC4);
      // Load the 3D model from file
    Model sphere = LoadModel("resources/model/sphere.glb");
    sphere.materials[0].shader = shader;// Set the earth texture to the model
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocation(shader, "diffuseMap");
    shader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(shader, "emissionMap");
    shader.locs[SHADER_LOC_MAP_SPECULAR] = GetShaderLocation(shader, "specularMap");
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetShaderLocation(shader, "normalMap");
    
    // Set cloud map texture location explicitly (it might not be automatically recognized)
    shader.locs[SHADER_LOC_MAP_DIFFUSE + 10] = cloudMapLoc; // Using a custom slot that doesn't conflict
    
    sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = earthTexture;
    sphere.materials[0].maps[MATERIAL_MAP_EMISSION].texture = nightTexture;
    sphere.materials[0].maps[MATERIAL_MAP_SPECULAR].texture = specularTexture;   
    sphere.materials[0].maps[MATERIAL_MAP_NORMAL].texture = normalTexture;
    
    // Create a custom material map slot for clouds
    // Using index 10 which is beyond the standard material map indices
    sphere.materials[0].maps[10].texture = cloudTexture;
    
    Vector3 spherePosition = { 0.0f, 0.0f, 0.0f }; // Position of the sphere
    float rotationAngle = 0.0f; // Current rotation angle of the Earth
    float rotationSpeed = 0.8f; // Rotation speed in degrees per frame
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f }; // Y-axis rotation for Earth
    
    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update camera with orbital controls
        UpdateCamera(&camera, CAMERA_ORBITAL);
        
        // Update rotation of the Earth
        rotationAngle += rotationSpeed;
        if (rotationAngle > 360.0f) rotationAngle -= 360.0f;        // Update camera position in shader for specular lighting
        SetShaderValue(shader, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
        
        // Create rotation matrix for model
        Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
        // Create translation matrix
        Matrix matTranslation = MatrixTranslate(spherePosition.x, spherePosition.y, spherePosition.z);
        // Create model matrix by combining rotation and translation
        Matrix matModel = MatrixMultiply(matTranslation, matRotation);
        
        // Set model matrix uniform
        SetShaderValueMatrix(shader, modelLoc, matModel);
        
        // Calculate and set MVP matrix
        Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
        Matrix matProjection = MatrixPerspective(camera.fovy*DEG2RAD, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);        Matrix matViewProjection = MatrixMultiply(matView, matProjection);
        Matrix mvp = MatrixMultiply(matModel, matViewProjection);
        SetShaderValueMatrix(shader, mvpLoc, mvp);
        
        // Update cloud texture binding explicitly each frame to ensure it's correctly bound
        SetShaderValueTexture(shader, cloudMapLoc, cloudTexture);
        
        // Draw
        BeginDrawing();
        
            ClearBackground(BLACK);            
            BeginMode3D(camera);
                // Use the shader with our calculated matrices
                // Draw the model at origin since our matrices already include the position
                DrawModelEx(sphere, Vector3Zero(), rotationAxis, rotationAngle, Vector3One(), WHITE);
            EndMode3D();
            
            DrawFPS(10, 50);
        
        EndDrawing();
    }    // De-Initialization
    UnloadShader(shader);
    UnloadModel(sphere);
    UnloadTexture(earthTexture);
    UnloadTexture(nightTexture);
    UnloadTexture(specularTexture);
    UnloadTexture(normalTexture);
    UnloadTexture(cloudTexture);
    CloseWindow();     // Close window and OpenGL context
    
    return 0;
}
