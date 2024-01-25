#ifndef _PRESSURE_PLATE_CMP_H_
#define _PRESSURE_PLATE_CMP_H_

struct PressurePlateCMP : CMP<PressurePlateCMP> {
    ConvexHull hull { };

    void Initialize(ConvexHull hull_) {
        hull = hull_;
    }

    void Serialize(Serializer *s) override {
        WriteBeginObject(s, "pressure_plate");
        
        WriteBeginObject(s, "convex_hull");
        Write(s, "count", hull.count);
        for(i32 i = 0; i < hull.count; i++) {
            Write(s, "v", hull.points[i]);
        }
        WriteEndObject(s);
        
        WriteEndObject(s);
    }

    void Deserialize(Tokenizer *t) override {
        ReadBeginObject(t, "pressure_plate");

        ConvexHull convexHull_;
        ReadBeginObject(t, "convex_hull");
        Read(t, "count", &convexHull_.count);
        convexHull_.points = (Vec3 *)MemoryManager::Get()->AllocStaticMemory(convexHull_.count * sizeof(Vec3), 8);
        for(i32 i = 0; i < convexHull_.count; i++) {
            Read(t, "v", convexHull_.points + i);
        }
        ReadEndObject(t);

        Initialize(convexHull_);

        ReadEndObject(t);
    }
};

#endif
