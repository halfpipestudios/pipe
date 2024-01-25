#ifndef _GLOBALS_H_
#define _GLOBALS_H_

extern Vec3 gCube[24];
extern MapImporter::EntityFace gCubeFaces[6];

Vec3 *CreateCube();
void TransformCube(Vec3 *cube,  Mat4 model);
void TransformEntity(MapImporter::Entity *mapEntity, Vec3 scale, Vec3 movement);

void DrawCube(Vec3 *cube, u32 color);
void DrawCylinder(Cylinder cylinder, u32 color);

#endif


