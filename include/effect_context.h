#pragma once

#include "godot_cpp/variant/string_name.hpp"
#include <cstdint>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>
#include "snapshot_data.h"
#include <unordered_map>

using namespace godot;

class EffectSystem;

class EffectContext : public RefCounted {
    GDCLASS(EffectContext, RefCounted)

private:
    EffectSystem* source_system = nullptr;
    EffectSystem* target_system = nullptr;

    int level = 1;
    uint32_t instance_id = 0;
    int modifier_index = -1;

    std::unordered_map<StringName, Ref<SnapshotData>> snapshot_store;
    std::unordered_map<int32_t, Variant> modifier_store;

    friend class EffectSystem;
    friend class EffectUtil;

protected:
    static void _bind_methods();

public:
    EffectContext() = default;

    void initialize(EffectSystem* p_source_effect_system, EffectSystem* p_target_effect_system, int p_level);

    EffectSystem* get_source_system() const { return source_system; }
    EffectSystem* get_target_system() const { return target_system; }
    int get_level() const { return level; }
    int get_effect_id() const { return instance_id; }
    int get_modifier_index() const { return modifier_index; }

    template<typename T>
    Ref<T> get_snapshot_object(const StringName& p_key) const {
        auto it = snapshot_store.find(p_key);
        if (it == snapshot_store.end()) return nullptr;
        return Ref<T>(Object::cast_to<T>(it->second.ptr()));
    }
    void set_snapshot(const StringName& p_key, const Ref<SnapshotData>& p_data) {
        snapshot_store[p_key] = p_data;
    }
    Variant get_snapshot_value(const StringName& p_path) const;

    void  set_data(const Variant& data);
    Variant get_data() const;
};