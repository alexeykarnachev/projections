#include "cimgui_utils.h"
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"
#include <stdbool.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 1200

static const Color BACKGROUND_COLOR = {20, 40, 0, 255};

static Camera3D CAMERA = {
    .position = {0.0, 5.0, 5.0},
    .target = {0.0, 0.0},
    .up = {0.0, 1.0, 0.0},
    .fovy = 70.0,
    .projection = CAMERA_PERSPECTIVE,
};

static bool IS_EDITOR_INTERACTED = false;

static float PROJECTION_ASPECT = 1.0;
static bool IS_AUTO_PROJECTION_ASPECT = true;

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

Matrix get_mat_perspective(float fovy, float aspect, float near, float far) {
    Matrix result = {0};

    double top = near * tan(fovy * 0.5);
    double bottom = -top;
    double right = top * aspect;
    double left = -right;

    // MatrixFrustum(-right, right, -top, top, near, far);
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(far - near);

    result.m0 = ((float)near * 2.0f) / rl;
    result.m5 = ((float)near * 2.0f) / tb;
    result.m8 = ((float)right + (float)left) / rl;
    result.m9 = ((float)top + (float)bottom) / tb;
    result.m10 = -((float)far + (float)near) / fn;
    result.m11 = -1.0f;
    result.m14 = -((float)far * (float)near * 2.0f) / fn;

    return result;
}

void draw_editor(void) {
    begin_imgui();

    ImGuiIO *io = igGetIO();
    IS_EDITOR_INTERACTED = io->WantCaptureMouse || io->WantCaptureKeyboard;

    if (igBegin("Editor", NULL, 0)) {
        if (ig_collapsing_header("Projection", true)) {
            igSliderFloat("FOV", &CAMERA.fovy, 0.0, 180.0, "%f", 0);

            igSliderFloat("Aspect", &PROJECTION_ASPECT, 0.0, 1.0, "%f", 0);
            igCheckbox("Auto aspect", &IS_AUTO_PROJECTION_ASPECT);
        }
    }

    igEnd();
    end_imgui();
}

void update_projection_aspect() {
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    float aspect = screen_width / screen_height;
    if (!IS_AUTO_PROJECTION_ASPECT) aspect = PROJECTION_ASPECT;

    PROJECTION_ASPECT = aspect;
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
        update_projection_aspect();

        update_camera_arcball();

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        {
            BeginMode3D(CAMERA);

            BeginShaderMode(shader);

            int mat_projection_loc = GetShaderLocation(shader, "u_mat_proj");

            Matrix mat_projection = get_mat_perspective(
                DEG2RAD * CAMERA.fovy, PROJECTION_ASPECT, 0.1, 100.0
            );

            SetShaderValueMatrix(shader, mat_projection_loc, mat_projection);

            // cube
            Vector3 position = {3.0, 0.0, -3.0};
            Vector3 size = {1.0, 1.0, 1.0};
            DrawCubeV(position, size, ORANGE);
            DrawCubeWiresV(position, size, RED);
            DrawGrid(16, 2.0);

            EndShaderMode();
            EndMode3D();
        }

        draw_editor();

        EndDrawing();
    }

    CloseWindow();
}
