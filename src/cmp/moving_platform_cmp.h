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
        Write(s, "a", a);
        Write(s, "b", b);
        Write(s, "movement", movement);
        Write(s, "dt_elapse", dtElapsed);
        Write(s, "speed", speed);
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        Vec3 a_, b_;
        ReadBeginObject(t, "moving_platform");
        Read(t, "a", &a_);
        Read(t, "b", &b_);
        Read(t, "movement", &movement);
        Read(t, "dt_elapse", &dtElapsed);
        Read(t, "speed", &speed);
        ReadEndObject(t);
        Initialize(a_, b_);
    };



};

#endif // _MOVING_PLATFORM_CMP_
