#include "SimpleGameEngine.h"
#include <cmath>
#include <list>

struct vec3d {
    float x, y, z;
};

struct triangle {
    vec3d p[3];
    Color color;
};

struct mesh {
    std::vector<triangle> tris;
};

// Although we are dealing with 3D, the extra 4th element is just for calculation purposes
struct mat4x4 {
    float m[4][4] = { 0 };
};

class Engine3D : public GameEngine {
private:
    mesh meshCube;
    mat4x4 matProj;
    float fTheta;
    vec3d vCamera = {0};

    void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
    {
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

        if (w != 0.0f)
        {
            o.x /= w; o.y /= w; o.z /= w;
        }
    }


    float clamp(float value, float min_value, float max_value) {
        return std::max(min_value, std::min(value, max_value));
    }

    Color dotProductToRGB(float dot_product, float min_dot_product, float max_dot_product) {
        // Clamp the dot product value between min_dot_product and max_dot_product
        dot_product = clamp(dot_product, min_dot_product, max_dot_product);

        // Normalize the dot product value to a range of 0 to 1
        float normalized_dot_product = (dot_product - min_dot_product) / (max_dot_product - min_dot_product);

        // Convert the normalized dot product value to an 8-bit integer value
        uint8_t color_value = static_cast<uint8_t>(normalized_dot_product * 255);

        // Set the RGB color channels
        Color color;
        color.r = color_value;
        color.g = color_value;
        color.b = color_value;

        return color;
    }

public:
    Engine3D(){}

    bool onInit() override {
        meshCube.tris = {
                // SOUTH
                { 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

                // EAST
                { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
                { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

                // NORTH
                { 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
                { 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

                // WEST
                { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

                // TOP
                { 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
                { 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

                // BOTTOM
                { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
                { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },
        };

        // Projection Matrix
        float fNear = 0.1f;
        float fFar = 1000.0f;
        float fFoV = 90.0f; // theta in degrees
        float fAspectRatio = (float)mWindowHeight / (float)mWindowWidth;
        float fFovRad = 1.0f / std::tanf(fFoV * 0.5f / 180.0f * 3.14159f);

        matProj.m[0][0] = fAspectRatio * fFovRad;
        matProj.m[1][1] = fFovRad;
        matProj.m[2][2] = fFar / (fFar - fNear);
        matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matProj.m[2][3] = 1.0f;
        matProj.m[3][3] = 0.0f;

        return true;
    }

    bool onFrameUpdate(float fElapsedTime) override {
        // Set up rotation matrices
        mat4x4 matRotZ, matRotX;
        fTheta += 1.0f * fElapsedTime;

        // Rotation Z
        matRotZ.m[0][0] = cosf(fTheta);
        matRotZ.m[0][1] = sinf(fTheta);
        matRotZ.m[1][0] = -sinf(fTheta);
        matRotZ.m[1][1] = cosf(fTheta);
        matRotZ.m[2][2] = 1;
        matRotZ.m[3][3] = 1;

        // Rotation X
        matRotX.m[0][0] = 1;
        matRotX.m[1][1] = cosf(fTheta * 0.5f);
        matRotX.m[1][2] = sinf(fTheta * 0.5f);
        matRotX.m[2][1] = -sinf(fTheta * 0.5f);
        matRotX.m[2][2] = cosf(fTheta * 0.5f);
        matRotX.m[3][3] = 1;
        for(auto tri: meshCube.tris){
            triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

            // rotate tri around Z axis
            MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
            MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
            MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);
            // rotate rotated tri around X axis
            MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
            MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

            // We want to be away from the world, not at the center. So we offset z axis
            triTranslated = triRotatedZX;
            triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
            triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
            triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

            vec3d normal, line1, line2;
            line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
            line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
            line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

            line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
            line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
            line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

            // normal is the cross product of the 2 lines
            normal.x = line1.y * line2.z - line1.z * line2.y;
            normal.y = line1.z * line2.x - line1.x * line2.z;
            normal.z = line1.x * line2.y - line1.y * line2.x;

            // convert normal into unit vector
            float l = std::sqrtf(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
            normal.x /= l; normal.y /= l; normal.z /= l;

            // only project to screen if the normal and the line from camera (0,0,0) to any point on triangle plane are 'similar'
            // here similar means that the line of sight and the normal from plane should have angle less than 90 between them.
            // So we calculate this using a dot product
            if(normal.x * (triTranslated.p[0].x - vCamera.x) +
               normal.y * (triTranslated.p[0].y - vCamera.y) +
               normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f)
            {
                //Illumination. light coming towards screen from all points
                vec3d light_direction = {0.0f, 0.0f, -1.0f};
                float l = std::sqrtf(light_direction.x*light_direction.x +
                        light_direction.y*light_direction.y + light_direction.z*light_direction.z);
                light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;

                float dp = normal.x*light_direction.x + normal.y*light_direction.y + normal.z*light_direction.z;
                triTranslated.color = dotProductToRGB(dp, -1.0f, 1.0f);
                // Project from world space (3D) to Screen space (2D)
                MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
                MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
                MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
                triProjected.color = triTranslated.color;

                // scale to positive (from -1,+1 to 0,+2)
                triProjected.p[0].x += 1.0f;
                triProjected.p[0].y += 1.0f;
                triProjected.p[1].x += 1.0f;
                triProjected.p[1].y += 1.0f;
                triProjected.p[2].x += 1.0f;
                triProjected.p[2].y += 1.0f;

                // scale to screen size
                triProjected.p[0].x *= 0.5f * mWindowWidth;
                triProjected.p[0].y *= 0.5f * mWindowHeight;
                triProjected.p[1].x *= 0.5f * mWindowWidth;
                triProjected.p[1].y *= 0.5f * mWindowHeight;
                triProjected.p[2].x *= 0.5f * mWindowWidth;
                triProjected.p[2].y *= 0.5f * mWindowHeight;

                fillTriangle(triProjected.p[0].x, triProjected.p[0].y,
                             triProjected.p[1].x, triProjected.p[1].y,
                             triProjected.p[2].x, triProjected.p[2].y, triProjected.color);
            }
        }
        return true;
    }
};

int main() {
    Engine3D engine3D;
    engine3D.constructConsole(1000, 800, "engine3D");
    engine3D.startGameLoop();
    return 0;
}
