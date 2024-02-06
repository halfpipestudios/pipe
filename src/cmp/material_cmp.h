#ifndef _MATERIAL_CMP_H_
#define _MATERIAL_CMP_H_

struct MaterialCMP : CMP<MaterialCMP> {
    Material material {};

    void Initialize(Vec3 ambient_, Vec3 diffuse_, Vec3 specular_, f32 shininess_) {
        material.ambient = ambient_;
        material.diffuse = diffuse_;
        material.specular = specular_;
        material.shininess = shininess_;
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "material");
        Write(s, "ambient", material.ambient);
        Write(s, "diffuse", material.diffuse);
        Write(s, "specular", material.specular);
        Write(s, "shininess", material.shininess);
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "material");
        Read(t, "ambient", &material.ambient);
        Read(t, "diffuse", &material.diffuse);
        Read(t, "specular", &material.specular);
        Read(t, "shininess", &material.shininess);
        ReadEndObject(t);
    };
};

#endif
