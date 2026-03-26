#include "effect_context.h"

#include <godot_cpp//core/class_db.hpp>
#include "effect_system.h"

using namespace godot;

void EffectContext::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "source_system", "target_system", "level"), &EffectContext::initialize);
    ClassDB::bind_method(D_METHOD("get_level"), &EffectContext::get_level);
    ClassDB::bind_method(D_METHOD("get_effect_id"), &EffectContext::get_effect_id);
    ClassDB::bind_method(D_METHOD("get_source_system"), &EffectContext::get_source_system);
    ClassDB::bind_method(D_METHOD("get_target_system"), &EffectContext::get_target_system);

    ClassDB::bind_method(D_METHOD("get_snapshot", "path"),
        &EffectContext::get_snapshot_value);

    ClassDB::bind_method(D_METHOD("set_data", "data"), &EffectContext::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &EffectContext::get_data);
}

void EffectContext::initialize(EffectSystem* p_source_system, EffectSystem* p_target_system, int p_level) {
    source_system = p_source_system;
    target_system = p_target_system;
    level = p_level;
}

Variant EffectContext::get_snapshot_value(const StringName& p_path) const {
    String s   = String(p_path);
    int sep = s.find("/");
    StringName key = StringName(sep < 0 ? s : s.substr(0, sep));
    StringName subpath = StringName(sep < 0 ? String() : s.substr(sep + 1));
    auto it = snapshot_store.find(key);
    if (it == snapshot_store.end()) return Variant();
    return it->second->resolve(subpath);
}

void EffectContext::set_data(const Variant& data) {
    modifier_store[modifier_index] = data;
}

Variant EffectContext::get_data() const {
    auto it = modifier_store.find(modifier_index);
    return it != modifier_store.end() ? it->second : Variant();
}