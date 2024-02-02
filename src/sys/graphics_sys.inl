#include "graphics_sys.h"

#include "../graphics_manager.h"

#include "../cmp/animation_cmp.h"

template <typename EM>
void GraphicsSys<EM>::Update(EM& em) {

    auto& components = em.GetComponents<GraphicsCMP>();
 
    for(i32 i = 0; i < components.size; ++i) {
        GraphicsCMP* graphic = &components[i];

        if(graphic->active == false) continue;

        SlotmapKey e = graphic->entityKey;
        
        // NOTE: the transform component says where to draw the component
        TransformCMP *transform = em.template GetComponent<TransformCMP>(e);
        if(transform == nullptr) continue;

        TransformCMP renderTransform = *transform;
        renderTransform.pos += transform->renderOffset;
        Model *model = ModelManager::Get()->Dereference(graphic->model);
        if(model->type == MODEL_TYPE_ANIMATED) {
            renderTransform.pos.y -= 0.75f;
            AnimationCMP *animationCmp = em.template GetComponent<AnimationCMP>(e);
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

        Material defaultMat = {};
        defaultMat.ambient = Vec3(0.4f, 0.4f, 0.4f);
        defaultMat.diffuse = Vec3(0.5f, 0.5f, 0.5f);
        defaultMat.specular = Vec3(0.6f, 0.6f, 0.6f);
        defaultMat.shininess = 1.0f;

        GraphicsManager::Get()->SetMaterial(defaultMat);


        VShader vShader = *VShaderManager::Get()->Dereference(graphic->vShader);
        FShader fShader = *FShaderManager::Get()->Dereference(graphic->fShader);
        
        for(u32 meshIndex = 0; meshIndex < model->numMeshes; ++meshIndex) {
            Mesh *mesh = model->meshes + meshIndex;
            GraphicsManager::Get()->BindTextureBuffer(*TextureManager::Get()->Dereference(mesh->texture));
            if(mesh->indexBuffer) {
                GraphicsManager::Get()->DrawIndexBuffer(mesh->indexBuffer, mesh->vertexBuffer, vShader, fShader);
            } else {
                GraphicsManager::Get()->DrawVertexBuffer(mesh->vertexBuffer, vShader, fShader);
            }
        }
    }


}
