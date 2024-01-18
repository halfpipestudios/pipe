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
        WriteBeginObject(s, "gem");
        Write(s, "value", value);
        Write(s, "timer", timer);
        Write(s, "timer_offset", timerOffset);
        WriteEndObject(s);
    };

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "gem");
        i32 value_;
        f32 timerOffset_;
        Read(t, "value", &value_);
        Read(t, "timer", &timer);
        Read(t, "timer_offset", &timerOffset_);
        ReadEndObject(t);

        Initialize(value_);
        timerOffset = timerOffset_;
    };

};

#endif // _GEM_CMP_H_
