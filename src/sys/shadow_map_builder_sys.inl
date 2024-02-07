#include "shadow_map_builder_sys.h"

template <typename EM>
void ShadowMapBuilderSys<EM>::RenderScene(EM& em, Map *map) {
    bool buildShadows = true;
    map->Render(buildShadows);


    auto& components = em.GetComponents<GraphicsCMP>();
 
    for(i32 i = 0; i < components.size; ++i) {
        GraphicsCMP* graphic = &components[i];

        if(graphic->active == false) continue;

        SlotmapKey e = graphic->entityKey;
        
        // NOTE: the transform component says where to draw the component
        TransformCMP *transform = em.template GetComponent<TransformCMP>(e);
        if(transform == nullptr) continue;

        if(graphic->type == GRAPHIC_CMP_TYPE_DYNAMIC) continue;

        TransformCMP renderTransform = *transform;
        renderTransform.pos += transform->renderOffset;
        Model *model = ModelManager::Get()->Dereference(graphic->model);
        
        GraphicsManager::Get()->SetWorldMatrix(renderTransform.GetWorldMatrix());

        VShader vShader = *VShaderManager::Get()->Dereference(VShaderManager::Get()->GetAsset("shadowMapVert.hlsl"));
        FShader fShader = *FShaderManager::Get()->Dereference(FShaderManager::Get()->GetAsset("shadowMapFrag.hlsl")); 

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

static void SMBWriteEntireFile(char *path, void *data, size_t size) {
    FILE *file = fopen(path, "wb");
    fwrite(data, size, 1, file);
    fclose(file);
}



template <typename EM>
void ShadowMapBuilderSys<EM>::GenerateSahdowMaps(EM& em, Level *level) {

    i32 w = SHADOW_MAP_RESOLUTION; 
    i32 h = SHADOW_MAP_RESOLUTION;
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(90, (f32)w/(f32)h, 0.5f, SHADOW_MAP_FAR_PLANE));
    GraphicsManager::Get()->SetShadowMappingRasterState();
    ///////////////////////////////////////////////////////////////////////////////////////////
 

    auto& lightComponents = em.GetComponents<LightCMP>();
    
    MemoryManager::Get()->BeginTemporalMemory();

    size_t expectedSize = 4*SHADOW_MAP_RESOLUTION*SHADOW_MAP_RESOLUTION;
    size_t totalSize = expectedSize * 6;
    size_t readSize = 0;
    u8 *buffer = (u8 *)MemoryManager::Get()->AllocTemporalMemory(totalSize, 8);
    static char path[4096];

    for(i32 i = 0; i < lightComponents.size; i++) {
        LightCMP *lig = &lightComponents[i];
        if(lig->type != LIGHT_TYPE_POINT) continue;

        GraphicsManager::Get()->UpdateLights(lig->pos, SHADOW_MAP_FAR_PLANE, nullptr, 0);

        SlotmapKey e = lig->entityKey;
        Entity_ *entity = em.GetEntity(e);

        u8 *write = buffer;

        // render right 
        GraphicsManager::Get()->BindDepthBufferAsRenderTarget();
        GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(lig->pos, lig->pos + Vec3( 1, 0, 0), Vec3(0, 1,  0)));
        RenderScene(em, &level->map);
        GraphicsManager::Get()->ReadDepthBuffer(&write, readSize);
        ASSERT(expectedSize == readSize);
        write += readSize;
        //sprintf(path, "./data/shadowmaps/%s-%s.sm", entity->name, "right");
        //SMBWriteEntireFile(path, (void *)buffer, readSize);

        // render left 
        GraphicsManager::Get()->BindDepthBufferAsRenderTarget();
        GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(lig->pos, lig->pos + Vec3(-1, 0, 0), Vec3(0, 1,  0)));
        RenderScene(em, &level->map);
        GraphicsManager::Get()->ReadDepthBuffer(&write, readSize);
        ASSERT(expectedSize == readSize);
        write += readSize;
        //sprintf(path, "./data/shadowmaps/%s-%s.sm", entity->name, "left");
        //SMBWriteEntireFile(path, (void *)buffer, readSize);
        
        // render top 
        GraphicsManager::Get()->BindDepthBufferAsRenderTarget();
        GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(lig->pos, lig->pos + Vec3(0,  1, 0), Vec3(0, 0, -1)));
        RenderScene(em, &level->map);
        GraphicsManager::Get()->ReadDepthBuffer(&write, readSize);
        ASSERT(expectedSize == readSize);
        write += readSize;
        //sprintf(path, "./data/shadowmaps/%s-%s.sm", entity->name, "top");
        //SMBWriteEntireFile(path, (void *)buffer, readSize);
        
        // render bottom 
        GraphicsManager::Get()->BindDepthBufferAsRenderTarget();
        GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(lig->pos, lig->pos + Vec3(0, -1, 0), Vec3(0, 0,  1)));
        RenderScene(em, &level->map);
        GraphicsManager::Get()->ReadDepthBuffer(&write, readSize);
        ASSERT(expectedSize == readSize);
        write += readSize;
        //sprintf(path, "./data/shadowmaps/%s-%s.sm", entity->name, "bottom");
        //SMBWriteEntireFile(path, (void *)buffer, readSize);
        
        // render front 
        GraphicsManager::Get()->BindDepthBufferAsRenderTarget();
        GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(lig->pos, lig->pos + Vec3(0, 0,  1), Vec3(0, 1,  0)));
        RenderScene(em, &level->map);
        GraphicsManager::Get()->ReadDepthBuffer(&write, readSize);
        ASSERT(expectedSize == readSize);
        write += readSize;
        //sprintf(path, "./data/shadowmaps/%s-%s.sm", entity->name, "front");
        //SMBWriteEntireFile(path, (void *)buffer, readSize);
        
        // render back 
        GraphicsManager::Get()->BindDepthBufferAsRenderTarget();
        GraphicsManager::Get()->SetViewMatrix(Mat4::LookAt(lig->pos, lig->pos + Vec3(0, 0, -1), Vec3(0, 1,  0)));
        RenderScene(em, &level->map);
        GraphicsManager::Get()->ReadDepthBuffer(&write, readSize);
        ASSERT(expectedSize == readSize);
        //sprintf(path, "./data/shadowmaps/%s-%s.sm", entity->name, "back");
        //SMBWriteEntireFile(path, (void *)buffer, readSize);
        
        sprintf(path, "./data/shadowmaps/%s.sm", entity->name);
        SMBWriteEntireFile(path, (void *)buffer, totalSize);

        char shadowMapName_[256];
        sprintf(lig->shadowMapName, "%s.sm", entity->name);
        lig->haveSahdowMap = true;

        lig->AddShadowMapToPointLight(lig->shadowMapName); 
        
    }


    MemoryManager::Get()->EndTemporalMemory();


    ///////////////////////////////////////////////////////////////////////////////////////////
    w = PlatformManager::Get()->GetWindow()->GetWidth(); 
    h = PlatformManager::Get()->GetWindow()->GetHeight(); 
    GraphicsManager::Get()->SetViewport(0, 0, w, h);
    GraphicsManager::Get()->SetRasterizerState(RASTERIZER_STATE_CULL_NONE);
    GraphicsManager::Get()->SetProjMatrix(Mat4::Perspective(60, (f32)w/(f32)h, 0.01f, 1000.0f));
    GraphicsManager::Get()->BindFrameBuffer(nullptr);
    GraphicsManager::Get()->ClearColorBuffer(nullptr, 0.1f, 1.0f, 0.4f);
    GraphicsManager::Get()->ClearDepthStencilBuffer(nullptr);

}
