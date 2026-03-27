#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class SnapshotData : public RefCounted {
    GDCLASS(SnapshotData, RefCounted)

protected:
    static void _bind_methods() {}

public:
    SnapshotData() = default;
    virtual Variant resolve(const StringName& subpath) const { return Variant(); }
};