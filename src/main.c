#include "cimgui_utils.h"
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 1024

static const Color BACKGROUND_COLOR = {20, 40, 0, 255};

static Camera3D CAMERA = {
    .position = {0.0, 5.0, -5.0},
    .target = {0.0, 0.0},
    .up = {0.0, 1.0, 0.0},
    .fovy = 70.0,
    .projection = CAMERA_PERSPECTIVE,
};

static bool IS_EDITOR_INTERACTED = false;

static void update_camera_arcball() {
    static const float rot_speed = 0.003;
    static const float move_speed = 0.01;
    static const float zoom_speed = 1.0;

    bool is_mmb_down = IsMouseButtonDown(2);
    bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouse_delta = GetMouseDelta();

    bool is_moving = is_mmb_down && is_shift_down;
    bool is_rotating = is_mmb_down && !is_shift_down;

    // move
    if (is_moving) {
        CameraMoveRight(&CAMERA, -move_speed * mouse_delta.x, true);

        // camera basis
        Vector3 z = GetCameraForward(&CAMERA);
        Vector3 x = Vector3Normalize(Vector3CrossProduct(z, (Vector3){0.0, 1.0, 0.0}));
        Vector3 y = Vector3Normalize(Vector3CrossProduct(x, z));

        Vector3 up = Vector3Scale(y, move_speed * mouse_delta.y);

        CAMERA.position = Vector3Add(CAMERA.position, up);
        CAMERA.target = Vector3Add(CAMERA.target, up);
    }

    // rotate
    if (is_rotating) {
        CameraYaw(&CAMERA, -rot_speed * mouse_delta.x, true);
        CameraPitch(&CAMERA, rot_speed * mouse_delta.y, true, true, false);
    }

    // zoom
    CameraMoveToTarget(&CAMERA, -GetMouseWheelMove() * zoom_speed);
}

void draw_editor(void) {
    begin_imgui();

    ImGuiIO *io = igGetIO();
    IS_EDITOR_INTERACTED = io->WantCaptureMouse || io->WantCaptureKeyboard;

    if (igBegin("Editor", NULL, 0)) {
        if (ig_collapsing_header("Camera", true)) {
            igSliderFloat("FOV", &CAMERA.fovy, 0.0, 180.0, "%f", 0);
        }
        igText("ZALOOOOOOOOPA!!!");
    }

    igEnd();
    end_imgui();
}

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Projections");

    SetTargetFPS(60);
    rlSetLineWidth(3.0);

    Shader shader = LoadShader(
        "./resources/shaders/base.vert.glsl", "./resources/shaders/base.frag.glsl"
    );

    load_imgui();

    while (!WindowShouldClose()) {
        update_camera_arcball();

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        {
            BeginMode3D(CAMERA);
            DrawGrid(16, 2.0);

            BeginShaderMode(shader);
            // cube
            Vector3 position = {0.0, 0.0, 0.0};
            Vector3 size = {1.0, 1.0, 1.0};
            DrawCubeV(position, size, ORANGE);
            DrawCubeWiresV(position, size, RED);

            EndShaderMode();
            EndMode3D();
        }

        draw_editor();

        EndDrawing();
    }

    CloseWindow();
}
