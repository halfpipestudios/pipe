#include "graphics_sys.h"

#include "../graphics_manager.h"

#include "../cmp/animation_cmp.h"

template <typename EM>
void GraphicsSys<EM>::Update(EM& em) {

    auto& components = em.GetComponents<GraphicsCMP>();
 
    for(i32 i = 0; i < components.size; ++i) {
        GraphicsCMP* graphic = &components[i];
        Entity_* e = graphic->entity;
        
        // NOTE: the transform component says where to draw the component
        TransformCMP *transform = e->template GetComponent<TransformCMP>();
        if(transform == nullptr) continue;

        TransformCMP renderTransform = *transform;

        if(graphic->model.type == MODEL_TYPE_ANIMATED) {
            renderTransform.pos.y -= 0.75f;
            AnimationCMP *animationCmp = e->template GetComponent<AnimationCMP>();
            if(animationCmp != nullptr) {
                GraphicsManager::Get()->SetAnimMatrices(animationCmp->finalTransformMatrix, animationCmp->numFinalTransformMatrix);
            }
            else {

                // TODO: refactor this. this is just to not animate the entities with
                // no AnimationComponent
                Mat4 identity[100];
                for(i32 i = 0; i < 100; ++i) {
                    identity[i] = Mat4();
                }
                GraphicsManager::Get()->SetAnimMatrices(identity, 100);
            }
        }

        
        GraphicsManager::Get()->SetWorldMatrix(renderTransform.GetWorldMatrix());
        
        for(u32 meshIndex = 0; meshIndex < graphic->model.numMeshes; ++meshIndex) {
            Mesh *mesh = graphic->model.meshes + meshIndex;
            GraphicsManager::Get()->BindTextureBuffer(mesh->texture);
            if(mesh->indexBuffer) {
                GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, graphic->shader);
            } else {
                GraphicsManager::Get()->DrawVertexBuffer(mesh->vertexBuffer, graphic->shader);
            }
        }
    }


}
