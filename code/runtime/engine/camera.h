#ifndef CAMERA_H
#define CAMERA_H

struct camera {
    f32  Pitch = 0.0f;
    f32  Yaw = 0.0f;
    f32  Roll = 0.0f;
    f32  Distance = 5.0f;
    f32  FieldOfView = To_Radians(60.0f);
    f32  ZNear = 0.01f;
    f32  ZFar = 500.0f;
    vec3 Target;

    camera() = default;
};

quat Camera_Get_Orientation(camera* Camera);
void Camera_Get_View(camera* Camera, matrix4_affine* View);
void Camera_Get_Perspective(camera* Camera, matrix4* Perspective, f32 AspectRatio);
vec3 Camera_Get_Position(camera* Camera);

#endif