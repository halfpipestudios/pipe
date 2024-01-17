#ifndef _GRAPHIC_CMP_H_
#define _GRAPHIC_CMP_H_

#include "graphics.h"
#include "mgr/model_manager.h"

#define MAX_MODEL_NAME 256

struct GraphicsCMP : CMP<GraphicsCMP> {
    
    void Initialize(char *name, Shader shader_) {
        strcpy(modelName, name);
        model = *ModelManager::Get()->Dereference(ModelManager::Get()->GetAsset(name));
        shader = shader_;
    }

    Model model;
    char modelName[MAX_MODEL_NAME];
    
    Shader shader;
    // TODO: char *shaderName;
    
    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "graphics");
        Read(t, "model", modelName, MAX_MODEL_NAME);
        ReadEndObject(t);
    };

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "graphics");
        Write(s, "model", modelName);
        WriteEndObject(s);
    };

};


#endif // _GRAPHIC_CMP_H_
