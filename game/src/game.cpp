#include "game.h"
#include "raylib.h"
#include "raymath.h"
#include <cfloat>
#include <array>
#include <optional>

typedef struct Sphere {
    Vector3 center;
    float radius;
    Color color;
} Sphere;

typedef enum LightType {
    Ambient, Point, Directional
} LightType;

struct Light {
    LightType type;
    float intensity;
    Vector3 position;
    Vector3 direction;
} ambient, point, directional;

std::array<Light, 3> lights;

std::array<Sphere, 4> spheres{ {
    {.center = {.x = 0.0f, .y = -1.0f, .z = 3.0f}, .radius = 1.0f, .color = RED },
    {.center = {.x = 2.0f, .y = 0.0f, .z = 4.0f}, .radius = 1.0f, .color = BLUE },
    {.center = {.x = -2.0f, .y = 0.0f, .z = 4.0f}, .radius = 1.0f, .color = GREEN },
    {.center = {.x = 0.0f, .y = -5001.0, .z = 0.0f}, .radius = 5000.0f, .color = YELLOW},
} };

float projectionPlaneD = 1.0f;
Vector2 viewportSize = { .x = 1.6f, .y = 0.9f };

void putPixel(int x, int y, Color color);
Vector3 canvasToViewport(int x, int y);
Color traceRay(Vector3 origin, Vector3 distance, float min, float max);
Vector2 intersectRaySphere(Vector3 origin, Vector3 distance, Sphere sphere);
float computeLighting(Vector3 point, Vector3 normal);

int main() {
    run();
    return 0;
}


void run() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hell Yeah");

    SetTargetFPS(60);
    Vector3 origin = { .x = 0.0f, .y = 0.0f, .z = 0.0f };
    
    ambient.type = Ambient;
    ambient.intensity = 0.2f;
    point.type = Point;
    point.intensity = 0.6f;
    point.position = { .x = 2.0f, .y = 1.0f, .z = 0.0f };
    directional.type = Directional;
    directional.intensity = 0.2f;
    directional.direction = { .x = 1.0f, .y = 4.0f, .z = 4.0f };

    lights[0] = ambient;
    lights[1] = point;
    lights[2] = directional;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);

        for (int x = -SCREEN_WIDTH / 2; x < SCREEN_WIDTH / 2; x++) {
            for (int y = -SCREEN_HEIGHT / 2; y < SCREEN_HEIGHT / 2; y++) {
                Vector3 direction = canvasToViewport(x, y);
                Color color = traceRay(origin, direction, 1.0f, FLT_MAX);
                putPixel(x, y, color);
            }
        }

        EndDrawing();
    }
    CloseWindow();
}

void putPixel(int x, int y, Color color) {
    int convertedX = SCREEN_WIDTH / 2 + x;
    int convertedY = SCREEN_HEIGHT / 2 - y - 1;

    if (convertedX < 0 || convertedX >= SCREEN_WIDTH || convertedY < 0 || convertedY >= SCREEN_HEIGHT) {
        return;
    }

    DrawPixel(convertedX, convertedY, color);
}

Vector3 canvasToViewport(int x, int y) {
    return Vector3(x * viewportSize.x / SCREEN_WIDTH, y * viewportSize.y / SCREEN_HEIGHT, projectionPlaneD);
}

Color traceRay(Vector3 origin, Vector3 direction, float min, float max) {
    float closestT = FLT_MAX;
    std::optional<Sphere> closestSphere;

    for (auto sphere: spheres) {
        Vector2 intersection = intersectRaySphere(origin, direction, sphere);
        if (intersection.x >= min && intersection.x <= max && intersection.x < closestT) {
            closestT = intersection.x;
            closestSphere = sphere;
        }
        if (intersection.y >= min && intersection.y <= max && intersection.y < closestT) {
            closestT = intersection.y;
            closestSphere = sphere;
        }        
    }

    if (!closestSphere.has_value()) {
        return WHITE;
    }
    Sphere sphere = *closestSphere;
    Vector3 point = origin + (direction * closestT);
    Vector3 normal = point - sphere.center;
    normal = Vector3Normalize(normal);
    float lighting = computeLighting(point, normal);
    float r = sphere.color.r * lighting;
    float g = sphere.color.g * lighting;
    float b = sphere.color.b * lighting;
    Color newColor{ .r = static_cast<unsigned char>(r), .g = static_cast<unsigned char>(g), .b = static_cast<unsigned char>(b), .a = sphere.color.a };
    return newColor;
}

Vector2 intersectRaySphere(Vector3 origin, Vector3 direction, Sphere sphere) {
    float radius = sphere.radius;
    Vector3 co = origin - sphere.center;
    float a = Vector3DotProduct(direction, direction);
    float b = 2.0f * Vector3DotProduct(co, direction);
    float c = Vector3DotProduct(co, co) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return Vector2(FLT_MAX, FLT_MAX);
    }

    float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
    float t2 = (-b - sqrtf(discriminant)) / (2.0f * a);
    return Vector2(t1, t2);
}

float computeLighting(Vector3 point, Vector3 normal) {
    float i = 0.0f;
    for (auto light: lights) {

        switch (light.type) {
        case Ambient: {
            i += light.intensity;
            break;
        }
        case Point: 
        case Directional: {
            Vector3 l;
            if (light.type == Point) {
                l = light.position - point;
            }
            else {
                l = light.direction;
            }
            float nDot1 = Vector3DotProduct(normal, l);
            if (nDot1 > 0.0f) {
                i += light.intensity * nDot1 / (Vector3Length(normal) * Vector3Length(l));
            }
            break;
        }
        }
    }
    return i;
}