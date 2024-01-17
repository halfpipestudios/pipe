#ifndef _GRAPHIC_CMP_H_
#define _GRAPHIC_CMP_H_

#include "graphics.h"
#include "mgr/model_manager.h"

struct GraphicsCMP : CMP<GraphicsCMP> {
    
    void Initialize(char *name, Shader shader_) {
        modelName = name;
        model = *ModelManager::Get()->Dereference(ModelManager::Get()->GetAsset(name));
        shader = shader_;
    }

    Model model;
    char *modelName;
    
    Shader shader;
    // TODO: char *shaderName;

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "graphics");
        Write(s, "model", modelName);
        WriteEndObject(s);
    };

};


#endif // _GRAPHIC_CMP_H_
