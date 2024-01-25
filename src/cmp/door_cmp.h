#ifndef _DOOR_CMP_H_
#define _DOOR_CMP_H_

#define MAX_PRESSURE_PLATES 16

struct DoorCMP : CMP<DoorCMP> {

    bool open { false };
    SlotmapKey plates[MAX_PRESSURE_PLATES];
    i32 platesCount { 0 };

    void Initialize(SlotmapKey *plates_, i32 count) {
        open = false;
        memcpy(plates, plates_, sizeof(SlotmapKey) * count);
        platesCount = count;
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "door");
        Write(s, "count", platesCount);
        for(i32 i = 0; i < platesCount; i++) {
            Write(s, "id", plates[i].id);
            Write(s, "gen", plates[i].gen);
        }
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {

        i32 platesCount_;
        SlotmapKey plates_[MAX_PRESSURE_PLATES];

        ReadBeginObject(t, "door");
        Read(t, "count", &platesCount_);
        for(i32 i = 0; i < platesCount_; i++) {
            Read(t, "id", &plates_[i].id);
            Read(t, "gen", &plates_[i].gen);
        }
        ReadEndObject(t);

        Initialize(plates_, platesCount_);
    }

};

#endif
