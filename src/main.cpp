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
    mat4x4 Matrix_MakeRotationX(float fAngleRad)
    {
        mat4x4 matrix;
        matrix.m[0][0] = 1.0f;
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[1][2] = sinf(fAngleRad);
        matrix.m[2][1] = -sinf(fAngleRad);
        matrix.m[2][2] = cosf(fAngleRad);
        matrix.m[3][3] = 1.0f;
        return matrix;
    }

    mat4x4 Matrix_MakeRotationZ(float fAngleRad)
    {
        mat4x4 matrix;
        matrix.m[0][0] = cosf(fAngleRad);
        matrix.m[0][1] = sinf(fAngleRad);
        matrix.m[1][0] = -sinf(fAngleRad);
        matrix.m[1][1] = cosf(fAngleRad);
        matrix.m[2][2] = 1.0f;
        matrix.m[3][3] = 1.0f;
        return matrix;
    }
    vec3d Vector_Add(vec3d &v1, vec3d &v2)
    {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    vec3d Vector_Sub(vec3d &v1, vec3d &v2)
    {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }

    float Vector_DotProduct(vec3d &v1, vec3d &v2)
    {
        return v1.x*v2.x + v1.y*v2.y + v1.z * v2.z;
    }

    float Vector_Length(vec3d &v)
    {
        return sqrtf(Vector_DotProduct(v, v));
    }

    vec3d Vector_Normalise(vec3d &v)
    {
        float l = Vector_Length(v);
        return { v.x / l, v.y / l, v.z / l };
    }

    vec3d Vector_CrossProduct(vec3d &v1, vec3d &v2)
    {
        vec3d v;
        v.x = v1.y * v2.z - v1.z * v2.y;
        v.y = v1.z * v2.x - v1.x * v2.z;
        v.z = v1.x * v2.y - v1.y * v2.x;
        return v;
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

        matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
        matRotX = Matrix_MakeRotationX(fTheta);
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
            vec3d zOffset = {0, 0, 3.0f};
            triTranslated.p[0] = Vector_Add(triRotatedZX.p[0], zOffset);
            triTranslated.p[1] = Vector_Add(triRotatedZX.p[1], zOffset);
            triTranslated.p[2] = Vector_Add(triRotatedZX.p[2], zOffset);


            vec3d normal, line1, line2;

            // Get lines either side of triangle
            line1 = Vector_Sub(triTranslated.p[1], triTranslated.p[0]);
            line2 = Vector_Sub(triTranslated.p[2], triTranslated.p[0]);
            // normal is the cross product of the 2 lines
            normal = Vector_CrossProduct(line1, line2);
            // convert normal into unit vector
            normal = Vector_Normalise(normal);
            // Get Ray from triangle to camera
            vec3d vCameraRay = Vector_Sub(triTranslated.p[0], vCamera);
            // only project to screen if the normal and camera ray are aligned
            if(Vector_DotProduct(normal, vCameraRay)  < 0.0f)
            {
                //Illumination. light coming towards screen from all points
                vec3d light_direction = {0.0f, 0.0f, -1.0f};
                light_direction = Vector_Normalise(light_direction);
                // get alignment between normal and light
                float dp = std::max(0.1f, Vector_DotProduct(light_direction, normal));
                triTranslated.color = dotProductToRGB(dp, -1.0f, 1.0f);
                // Project from world space (3D) to Screen space (2D)
                MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
                MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
                MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
                triProjected.color = triTranslated.color;

                // offset x,y to positive into visible normalised space (from -1,+1 to 0,+2)
                vec3d vOffsetView = { 1,1,0 };
                triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
                triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
                triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

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
