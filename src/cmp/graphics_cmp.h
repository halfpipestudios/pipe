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

    bool active { true }; 

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "graphics");
        Write(s, "model", modelName);
        Write(s, "vshader", vShaderName);
        Write(s, "fshader", fShaderName);
        // TODO: ...
        //Write(s, "active", (i32)active);
        WriteEndObject(s);
    };
    
    void Deserialize(Tokenizer *t) override {
        char modelName_[MAX_MODEL_NAME];
        char vShaderName_[MAX_SHADER_NAME];
        char fShaderName_[MAX_SHADER_NAME];

        ReadBeginObject(t, "graphics");
        Read(t, "model", modelName_, MAX_MODEL_NAME);
        Read(t, "vshader", vShaderName_, MAX_SHADER_NAME);
        Read(t, "fshader", fShaderName_, MAX_SHADER_NAME);
        // TODO: ...
        //Read(t, "active", (i32 *)&active);
        ReadEndObject(t);

        Initialize(modelName_, vShaderName_, fShaderName_);

    };

};


#endif // _GRAPHIC_CMP_H_
