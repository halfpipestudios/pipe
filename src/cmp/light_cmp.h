#ifndef _LIGHT_CMP_H_
#define _LIGHT_CMP_H_

#include "mgr/shadowmap_manager.h"

#define MAX_SHADOWMAP_NAME 32

struct LightCMP : CMP<LightCMP> {
    
    LightType type;
    Vec3 pos;
    Vec3 dir;
    
    Vec3 ambient;
    Vec3 diffuse;
    Vec3 specular;

    f32 constant;
    f32 linear;
    f32 quadratic;

    f32 cutOff;
    f32 outerCutOff;

    bool haveSahdowMap { false };
    Handle shadowMap {};
    char shadowMapName[MAX_SHADOWMAP_NAME];


    void InitializeDirLight(Vec3 dir_, Vec3 ambient_, Vec3 diffuse_, Vec3 specular_) {
        type = LIGHT_TYPE_DIRECTIONAL;
        dir = dir_;
        ambient = ambient_;
        diffuse = diffuse_;
        specular = specular_;
    }

    void InitializePointLight(Vec3 pos_,
                              Vec3 ambient_, Vec3 diffuse_, Vec3 specular_,
                              f32 constant_, f32 linear_, f32 quadratic_,
                              char *shadowMapName_) {
        type = LIGHT_TYPE_POINT;
        pos = pos_;
        ambient = ambient_;
        diffuse = diffuse_;
        specular = specular_;
        constant = constant_;
        linear = linear_;
        quadratic = quadratic_;
        strcpy(shadowMapName, shadowMapName_);
        shadowMap = ShadowMapManager::Get()->GetAsset(shadowMapName_);
        haveSahdowMap = true;
    }


    void InitializePointLight(Vec3 pos_,
                              Vec3 ambient_, Vec3 diffuse_, Vec3 specular_,
                              f32 constant_, f32 linear_, f32 quadratic_) {
        type = LIGHT_TYPE_POINT;
        pos = pos_;
        ambient = ambient_;
        diffuse = diffuse_;
        specular = specular_;
        constant = constant_;
        linear = linear_;
        quadratic = quadratic_;
        haveSahdowMap = false;
    }

    void AddShadowMapToPointLight(char *shadowMapName_) {
        strcpy(shadowMapName, shadowMapName_);
        shadowMap = ShadowMapManager::Get()->GetAsset(shadowMapName_);
        haveSahdowMap = true;
    }

    void InitializeSpotLight(Vec3 pos_, Vec3 dir_, f32 cutOff_, f32 outerCutOff_,
                             Vec3 ambient_, Vec3 diffuse_, Vec3 specular_) {
        type = LIGHT_TYPE_SPOT;
        pos = pos_;
        dir = dir_;
        cutOff = cutOff_;
        outerCutOff = outerCutOff_;
        ambient = ambient_;
        diffuse = diffuse_;
        specular = specular_;
    }
    
    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "light");
        Write(s, "type", (i32)type);
        Write(s, "pos", pos);
        Write(s, "dir", dir);
        Write(s, "ambient", ambient);
        Write(s, "diffuse", diffuse);
        Write(s, "specular", specular);
        Write(s, "constant", constant);
        Write(s, "linear", linear);
        Write(s, "quadratic", quadratic);
        Write(s, "cutOff", cutOff);
        Write(s, "outerCutOff", outerCutOff);
        Write(s, "shadowMap", shadowMapName);
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {


        char shadowMapName_[MAX_SHADOWMAP_NAME];

        ReadBeginObject(t, "light");
        Read(t, "type", &(i32)type);
        Read(t, "pos", &pos);
        Read(t, "dir", &dir);
        Read(t, "ambient", &ambient);
        Read(t, "diffuse", &diffuse);
        Read(t, "specular", &specular);
        Read(t, "constant", &constant);
        Read(t, "linear", &linear);
        Read(t, "quadratic", &quadratic);
        Read(t, "cutOff", &cutOff);
        Read(t, "outerCutOff", &outerCutOff);
        Read(t, "shadowMap", shadowMapName_, MAX_SHADOWMAP_NAME);
        ReadEndObject(t);

        if(type == LIGHT_TYPE_POINT) {
            AddShadowMapToPointLight(shadowMapName_);
        }
    }

};

#endif
