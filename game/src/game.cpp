#include "raylib.h"
#include "raymath.h"
#include <cfloat>
#include <array>
#include <optional>

#define BACKGROUND_COLOR BLACK
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

typedef struct Sphere {
    Vector3 center;
    float radius;
    Color color;
    float specular;
    float reflective;
} Sphere;

typedef struct Intersection {
    float t1;
    float t2;
} Intersection;

typedef struct SphereIntersection {
    std::optional<Sphere> sphere;
    float closest;
} SphereIntersection;

typedef enum LightType {
    Ambient, Point, Directional
} LightType;

typedef struct Light {
    LightType type;
    float intensity;
    Vector3 position;
} Light;

std::array<Light, 3> lights{ {
    {.type = Ambient,.intensity = 0.2f, .position = {.x = 0.0f, .y = 0.0f, .z = 0.0f } },
    {.type = Point,.intensity = 0.6f, .position = {.x = 2.0f, .y = 1.0f, .z = 0.0f } },
    {.type = Directional,.intensity = 0.2f, .position = {.x = 1.0f, .y = 4.0f, .z = 4.0f } },
} };

std::array<Sphere, 4> spheres{ {
    {.center = {.x = 0.0f, .y = -1.0f, .z = 3.0f}, .radius = 1.0f, .color = RED, .specular = 500.0f, .reflective = 0.2f },
    {.center = {.x = 2.0f, .y = 0.0f, .z = 4.0f}, .radius = 1.0f, .color = BLUE, .specular = 500.0f, .reflective = 0.3f },
    {.center = {.x = -2.0f, .y = 0.0f, .z = 4.0f}, .radius = 1.0f, .color = GREEN, .specular = 10.0f, .reflective = 0.4f },
    {.center = {.x = 0.0f, .y = -5001.0, .z = 0.0f}, .radius = 5000.0f, .color = YELLOW, .specular = 1000.0f, .reflective = 0.5f},
} };

float projectionPlaneD = 1.0f;
Vector2 viewportSize = { .x = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), .y = 1.0f };
Vector3 cameraPos{.x = 3.0f, .y = 0.0f, .z = 1.0f};
float cameraRotation[3][3] {
    {0.7071f, 0.0f, -0.7071f},
    {0.0f,    1.0f,  0.0f},
    {0.7071f, 0.0f,  0.7071f},
};
void run();
void putPixel(int x, int y, Color color);
Vector3 canvasToViewport(int x, int y);
Color traceRay(Vector3 origin, Vector3 distance, float min, float max, int limit);
Intersection intersectRaySphere(Vector3 origin, Vector3 direction, Sphere sphere);
SphereIntersection closestIntersection(Vector3 origin, Vector3 direction, float min, float max);
float computeLighting(Vector3 point, Vector3 normal, Vector3 view, float specular);
Vector3 reflectRay(Vector3 normal, Vector3 l);
Color multiplyColor(Color color, float value);
Color addColors(Color color1, Color color2);
Vector3 vecMulMatrix(Vector3 vec, float matrix[3][3]);

int main() {
    run();
    return 0;
}


void run() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Software Raytracing");

    SetTargetFPS(60);
    Vector3 origin = { .x = 0.0f, .y = 0.0f, .z = 0.0f };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        for (int x = -SCREEN_WIDTH / 2; x < SCREEN_WIDTH / 2; x++) {
            for (int y = -SCREEN_HEIGHT / 2; y < SCREEN_HEIGHT / 2; y++) {
                Vector3 direction = vecMulMatrix(canvasToViewport(x, y), cameraRotation);
                Color color = traceRay(cameraPos, direction, 1.0f, FLT_MAX, 3);
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

Color traceRay(Vector3 origin, Vector3 direction, float min, float max, int limit) {
    SphereIntersection sphereIntersection = closestIntersection(origin, direction, min, max);
    float clostestT = sphereIntersection.closest;

    if (!sphereIntersection.sphere.has_value()) {
        return BACKGROUND_COLOR;
    }

    Sphere sphere = sphereIntersection.sphere.value();
    Vector3 point = origin + (direction * clostestT);
    Vector3 normal = point - sphere.center;
    normal = Vector3Normalize(normal);
    float lighting = computeLighting(point, normal, Vector3Negate(direction), sphere.specular);
    Color localColor = multiplyColor(sphere.color, lighting);
    
    Vector3 view = direction * -1.0f;
    float reflective = sphere.reflective;
    if (limit <= 0 || reflective <= 0.0f) {
        return localColor;
    }
    Vector3 reflectionRay = reflectRay(view, normal);
    Color reflectedColor = traceRay(point, reflectionRay, 0.01f, FLT_MAX, limit - 1);
    
    Color local = multiplyColor(localColor, (1.0f - reflective));
    Color reflected = multiplyColor(reflectedColor, reflective);
    return addColors(local, reflected);
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
    for (auto &light: lights) {
        if (light.type == Ambient) {
            i += light.intensity;
            continue;
        }

        Vector3 l;
        float max;
        if (light.type == Point) {
            l = light.position - point;
            max = 1.0f;
        }
        else {
            l = light.position;
            max = FLT_MAX;
        }
        
        SphereIntersection shadowIntersection = closestIntersection(point, l, 0.01f, max);
        if (shadowIntersection.sphere.has_value()) {
            continue;
        }

        float nDotl = Vector3DotProduct(normal, l);
        if (nDotl > 0.0f) {
            i += light.intensity * nDotl / (Vector3Length(normal) * Vector3Length(l));
        }

        if (specular != -1) {
            Vector3 reflection = normal * 2.0f * nDotl - l;
            float rDotV = Vector3DotProduct(reflection, view);
            if (rDotV > 0.0f) {
                i += light.intensity * powf(rDotV / (Vector3Length(reflection) * Vector3Length(view)), specular);
            }
        }
    }
    return i;
}

SphereIntersection closestIntersection(Vector3 origin, Vector3 direction, float min, float max) {
    float closestT = FLT_MAX;
    std::optional<Sphere> closestSphere;

    for (auto &sphere: spheres) {
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

Vector3 reflectRay(Vector3 v1, Vector3 v2) {
    return v2 * 2.0f * Vector3DotProduct(v1, v2) - v1;
}

Color multiplyColor(Color color, float value) {
    float r = color.r * value;
    float g = color.g * value;
    float b = color.b * value;
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

    Color newColor{ .r = static_cast<unsigned char>(r), .g = static_cast<unsigned char>(g), .b = static_cast<unsigned char>(b), .a = 255 };
    return newColor;
}

Color addColors(Color color1, Color color2) {
    unsigned char r = color1.r + color2.r;
    unsigned char g = color1.g + color2.g;
    unsigned char b = color1.b + color2.b;
    if (r > 255) {
        r = 255;
    }
    if (r < 0) {
        r = 0;
    }
    if (g > 255) {
        g = 255;
    }
    if (g < 0) {
        g = 0;
    }
    if (b > 255) {
        b = 255;
    }
    if (b < 0) {
        b = 0;
    }

    Color newColor{ .r = r, .g = g, .b = b, .a = 255 };
    return newColor;
}

Vector3 vecMulMatrix(Vector3 vec, float matrix[3][3]) {
    float result[3] = { 0.0f, 0.0f, 0.0f };
    float vecArray[3] = { vec.x, vec.y, vec.z };

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result[i] += vecArray[j] * matrix[i][j];
        }
    }

    Vector3 newVec{.x = result[0], .y = result[1], .z = result[2]};
    return newVec;
}