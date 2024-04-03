quat Camera_Get_Orientation(camera* Camera) {
    return Quat_Normalize(Quat_RotZ(Camera->Roll)*Quat_RotY(Camera->Yaw)*Quat_RotX(Camera->Pitch));
}

void Camera_Get_View(camera* Camera, matrix4_affine* View) {
    matrix3 Orientation = Camera_Get_Orientation(Camera);
    vec3 Position = Camera->Target + (Orientation.z*Camera->Distance);
    Matrix4_Affine_Inverse_No_Scale(View, Position, Orientation);
}

void Camera_Get_Perspective(camera* Camera, matrix4* Perspective, f32 AspectRatio) {
    Matrix4_Perspective(Perspective, AspectRatio, Camera->FieldOfView, Camera->ZNear, Camera->ZFar);
}

vec3 Camera_Get_Position(camera* Camera) {
    matrix3 Orientation = Camera_Get_Orientation(Camera);
    vec3 Position = Camera->Target + (Orientation.z*Camera->Distance);
    return Position;
}