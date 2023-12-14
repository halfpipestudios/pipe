#include "moving_platform_sys.h"

static Vec3 gCube_[] = {

    // bottom
    Vec3(-0.5, -0.5f,  0.5f),
    Vec3( 0.5, -0.5f,  0.5f),
    Vec3( 0.5, -0.5f, -0.5f),
    Vec3(-0.5, -0.5f, -0.5f),
    // top
    Vec3(-0.5, 0.5f,  0.5f),
    Vec3( 0.5, 0.5f,  0.5f),
    Vec3( 0.5, 0.5f, -0.5f),
    Vec3(-0.5, 0.5f, -0.5f),
    // left
    Vec3(-0.5, -0.5f,  0.5f),
    Vec3(-0.5, -0.5f, -0.5f),
    Vec3(-0.5,  0.5f, -0.5f),
    Vec3(-0.5,  0.5f,  0.5f),
    // right
    Vec3(0.5, -0.5f,  0.5f),
    Vec3(0.5, -0.5f, -0.5f),
    Vec3(0.5,  0.5f, -0.5f),
    Vec3(0.5,  0.5f,  0.5f),
    // front
    Vec3(-0.5, -0.5f, -0.5f),
    Vec3(-0.5,  0.5f, -0.5f),
    Vec3( 0.5,  0.5f, -0.5f),
    Vec3( 0.5, -0.5f, -0.5f),
    // back
    Vec3(-0.5, -0.5f, 0.5f),
    Vec3(-0.5,  0.5f, 0.5f),
    Vec3( 0.5,  0.5f, 0.5f),
    Vec3( 0.5, -0.5f, 0.5f)

};

static MapImporter::EntityFace gCubeFaces_[] = {
    // bottom
    { {{ 0, -1,  0}, 32.0f}, {}, 0 },
    // top
    { {{ 0,  1,  0}, 32.0f}, {}, 0 },
    // left
    { {{-1,  0,  0}, 32.0f}, {}, 0 },
    // right
    { {{ 1,  0,  0}, 32.0f}, {}, 0 },
    // front
    { {{ 0,  0,  1}, 32.0f}, {}, 0 },
    // back
    { {{ 0,  0, -1}, 32.0f}, {}, 0 }
};



static void TransformCube_(Vec3 *cube,  Vec3 p, Vec3 scale, f32 angle) {
    for(i32 i = 0; i < 24; ++i) {
        cube[i] = Mat4::TransformPoint(Mat4::Translate(p.x, p.y, p.z) * Mat4::RotateX(angle) * Mat4::Scale(scale), gCube_[i]);
    }
}

static void TransformEntity_(MapImporter::Entity *mapEntity, Vec3 scale, Vec3 movement) {
    for(i32 i = 0; i < mapEntity->facesCount; ++i) {
        MapImporter::EntityFace *face = mapEntity->faces + i;

        f32 d = fabsf(face->plane.n.Dot(scale)) / 2;
        f32 displacement = movement.Dot(face->plane.n);
        if(displacement != 0.0f) {
            face->plane.d = (gCubeFaces_[i].plane.d * d) + (displacement*32.0f);
        } else {
            face->plane.d = (gCubeFaces_[i].plane.d * d);
        }

    }
}

template <typename EM>
void MovingPlatformSys<EM>::Update(EM& em, f32 dt) {

    auto& platforms = em.GetComponents<MovingPlatformCMP>();

    for(i32 i = 0; i < platforms.size; ++i) {
        
        MovingPlatformCMP *platform = &platforms[i];
        SlotmapKey entityKey = platform->entityKey;

        TransformCMP *transform = em.GetComponent<TransformCMP>(entityKey);
        CollisionCMP *collider  = em.GetComponent<CollisionCMP>(entityKey);

        if(transform == nullptr || collider == nullptr) continue;


        Vec3 lastPos = transform->pos;
        Vec3 ab = platform->b - platform->a;
        f32 len = ab.Len();
        
        if(len > 0.001f) {
            f32 t = (sinf((platform->dtElapsed / len) * platform->speed) + 1.0f) * 0.5f;
            transform->pos = platform->a + (platform->b - platform->a) * t;
        }

        platform->movement = transform->pos - lastPos;

        TransformCube_(collider->poly3D.convexHull.points, transform->pos, transform->scale, 0);
        TransformEntity_(&collider->poly3D.entity, transform->scale, transform->pos);

        platform->dtElapsed += dt; 
        
    }

}
