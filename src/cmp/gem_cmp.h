#ifndef _GEM_CMP_H_
#define _GEM_CMP_H_

struct GemCMP : CMP<GemCMP> {
    i32 value { 5 };
    f32 timer { 0.0f };
    f32 timerOffset { 0 };

    void Initialize(i32 value) {
        this->value = value;
        this->timerOffset = (f32)value; //(value - 50.0f) / 49.0f;
    }

    void Serialize(Serializer *s) override {
        BeginObject(s, "gem");
        Write(s, "value", value);
        Write(s, "timer", timer);
        Write(s, "timer_offset", timerOffset);
        EndObject(s);
    };

};

#endif // _GEM_CMP_H_
