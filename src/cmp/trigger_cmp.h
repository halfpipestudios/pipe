#ifndef _TRIGGER_CMP_H_
#define _TRIGGER_CMP_H_

struct TriggerCMP : CMP<TriggerCMP> {
    Cylinder collider  {};
    SlotmapKey whoTriggerThis {};
    void Initialize(SlotmapKey who, Cylinder collider) {
        this->collider = collider;
        whoTriggerThis = who;
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "trigger");
        
        WriteBeginObject(s, "cylinder");
        Write(s, "c", collider.c);
        Write(s, "u", collider.u);
        Write(s, "radii", collider.radii);
        Write(s, "n", collider.n);
        WriteEndObject(s);
        
        WriteBeginObject(s, "who");
        Write(s, "id", whoTriggerThis.id);
        Write(s, "gen", whoTriggerThis.gen);
        WriteEndObject(s);
        
        WriteEndObject(s);
    };

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "trigger");

        Cylinder collider_;
        ReadBeginObject(t, "cylinder");
        Read(t, "c", &collider_.c);
        Read(t, "u", &collider_.u);
        Read(t, "radii", &collider_.radii);
        Read(t, "n", &collider_.n);
        ReadEndObject(t);

        SlotmapKey who;
        ReadBeginObject(t, "who");
        Read(t, "id", &who.id);
        Read(t, "gen", &who.gen);
        ReadEndObject(t);
        
        ReadEndObject(t);

        Initialize(who, collider_);
    };

};

#endif
