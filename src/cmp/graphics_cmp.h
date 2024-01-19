#ifndef _GRAPHIC_CMP_H_
#define _GRAPHIC_CMP_H_

#include "graphics.h"
#include "mgr/model_manager.h"
#include "mgr/shader_manager.h"

#define MAX_MODEL_NAME 32
#define MAX_SHADER_NAME 32

struct GraphicsCMP : CMP<GraphicsCMP> {
    
    void Initialize(char *name, char *vName, char *fName) {
        strcpy(modelName, name);
        model = ModelManager::Get()->GetAsset(name);

        strcpy(vShaderName, vName);
        vShader = VShaderManager::Get()->GetAsset(vName);

        strcpy(fShaderName, fName);
        fShader = FShaderManager::Get()->GetAsset(fName);
    }

    Handle model;
    Handle vShader;
    Handle fShader;

    char modelName[MAX_MODEL_NAME];
    char vShaderName[MAX_SHADER_NAME];
    char fShaderName[MAX_SHADER_NAME];
    
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
