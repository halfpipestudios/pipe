#ifndef _MOVING_PLATFORM_CMP_
#define _MOVING_PLATFORM_CMP_

struct MovingPlatformCMP : CMP<MovingPlatformCMP> {
    Vec3 a        { };
    Vec3 b        { };
    Vec3 movement { };
    f32 dtElapsed { 0 };
    f32 speed     { 15 };

    void Initialize(Vec3 a_, Vec3 b_) {
        a = a_;
        b = b_;
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "moving_platform");
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "moving_platform");
        ReadEndObject(t);
    };



};

#endif // _MOVING_PLATFORM_CMP_
