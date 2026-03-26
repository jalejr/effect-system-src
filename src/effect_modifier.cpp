#include "effect_modifier.h"
#include <godot_cpp//core/class_db.hpp>

using namespace godot;

void EffectModifier::_bind_methods() {
    GDVIRTUAL_BIND(_apply, "context");
    GDVIRTUAL_BIND(_remove, "context");
}