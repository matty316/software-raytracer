#include "game.h"
#include "raylib.h"
#include "raymath.h"
#include <cfloat>
#include <array>

#define BACKGROUND_COLOR WHITE

typedef struct Sphere {
    Vector3 center;
    float radius;
    Color color;
    float specular;
} Sphere;

typedef struct Intersection {
    float t1;
    float t2;
} Intersection;

typedef struct SphereIntersection {
    Sphere sphere;
    float closest;
} SphereIntersection;

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
    {.center = {.x = 0.0f, .y = -1.0f, .z = 3.0f}, .radius = 1.0f, .color = RED, .specular = 500.0f },
    {.center = {.x = 2.0f, .y = 0.0f, .z = 4.0f}, .radius = 1.0f, .color = BLUE, .specular = 500.0f },
    {.center = {.x = -2.0f, .y = 0.0f, .z = 4.0f}, .radius = 1.0f, .color = GREEN, .specular = 10.0f },
    {.center = {.x = 0.0f, .y = -5001.0, .z = 0.0f}, .radius = 5000.0f, .color = YELLOW, .specular = 1000.0f},
} };

float projectionPlaneD = 1.0f;
Vector2 viewportSize = { .x = 1.6f, .y = 0.9f };

void putPixel(int x, int y, Color color);
Vector3 canvasToViewport(int x, int y);
Color traceRay(Vector3 origin, Vector3 distance, float min, float max);
Intersection intersectRaySphere(Vector3 origin, Vector3 direction, Sphere sphere);
SphereIntersection closestIntersection(Vector3 origin, Vector3 direction, float min, float max);
float computeLighting(Vector3 point, Vector3 normal, Vector3 view, float specular);

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
        ClearBackground(BACKGROUND_COLOR);

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
    SphereIntersection sphereIntersection = closestIntersection(origin, direction, min, max);

    if (sphereIntersection.sphere.radius < 0.0f) {
        return BACKGROUND_COLOR;
    }
    Sphere sphere = sphereIntersection.sphere;
    Vector3 point = origin + (direction * sphereIntersection.closest);
    Vector3 normal = point - sphere.center;
    normal = Vector3Normalize(normal);
    float lighting = computeLighting(point, normal, Vector3Negate(direction), sphere.specular);
    float r = sphere.color.r * lighting;
    float g = sphere.color.g * lighting;
    float b = sphere.color.b * lighting;
    if (r > 255.0f) {
        r = 255.0f;
    }
    if (r < 0.0f) {
        r = 0.0f;
    }
    if (g > 255.0f) {
        g = 255.0f;
    }
    if (g < 0.0f) {
        g = 0.0f;
    }
    if (b > 255.0f) {
        b = 255.0f;
    }
    if (b < 0.0f) {
        b = 0.0f;
    }
    
    Color newColor{ .r = static_cast<unsigned char>(r), .g = static_cast<unsigned char>(g), .b = static_cast<unsigned char>(b), .a = sphere.color.a };
    return newColor;
}

Intersection intersectRaySphere(Vector3 origin, Vector3 direction, Sphere sphere) {
    float radius = sphere.radius;
    Vector3 co = origin - sphere.center;
    float a = Vector3DotProduct(direction, direction);
    float b = 2.0f * Vector3DotProduct(co, direction);
    float c = Vector3DotProduct(co, co) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return Intersection{ FLT_MAX, FLT_MAX };
    }

    float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
    float t2 = (-b - sqrtf(discriminant)) / (2.0f * a);
    return Intersection{ t1, t2 };
}

float computeLighting(Vector3 point, Vector3 normal, Vector3 view, float specular) {
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
            float max;
            if (light.type == Point) {
                l = light.position - point;
                max = 1.0f;
            }
            else {
                l = light.direction;
                max = FLT_MAX;
            }

            SphereIntersection shadowIntersection = closestIntersection(point, l, 0.001f, max);
            if (shadowIntersection.sphere.radius > 0.0f) {
                continue;
            }

            float nDotl = Vector3DotProduct(normal, l);
            if (nDotl > 0.0f) {
                i += light.intensity * nDotl / (Vector3Length(normal) * Vector3Length(l));
            }
            
            if (specular != -1) {
                Vector3 reflection = normal * 2.0f * Vector3DotProduct(normal, l) - l;
                float rDotV = Vector3DotProduct(reflection, view);
                if (rDotV > 0.0f) {
                    i += light.intensity * pow(rDotV / (Vector3Length(reflection) * Vector3Length(view)), specular);
                }
            }
            break;
        }
        }
    }
    return i;
}

SphereIntersection closestIntersection(Vector3 origin, Vector3 direction, float min, float max) {
    float closestT = FLT_MAX;
    Sphere closestSphere{ .center = {.x = 0.0f, .y = 0.0f, .z = 0.0f}, .radius = -1.0f, .color = WHITE, .specular = 0.0f };

    for (auto sphere : spheres) {
        Intersection intersection = intersectRaySphere(origin, direction, sphere);
        if (intersection.t1 > min && intersection.t1 < max && intersection.t1 < closestT) {
            closestT = intersection.t1;
            closestSphere = sphere;
        } 
        if (intersection.t2 > min && intersection.t2 < max && intersection.t2 < closestT) {
            closestT = intersection.t2;
            closestSphere = sphere;
        }
    }

    return SphereIntersection{ closestSphere, closestT };
}