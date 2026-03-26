#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include "effect_context.h"

using namespace godot;

class EffectContext;

class EffectModifier : public Resource {
    GDCLASS(EffectModifier, Resource)

protected:
    static void _bind_methods();

public:
    EffectModifier() = default;

    GDVIRTUAL1(_apply, Ref<EffectContext>);
    GDVIRTUAL1(_remove, Ref<EffectContext>);

    virtual ~EffectModifier() = default;

    virtual void apply(const Ref<EffectContext>&  p_context) { 
        GDVIRTUAL_CALL(_apply, p_context); 
    }
    virtual void remove(const Ref<EffectContext>& p_context) {
        GDVIRTUAL_CALL(_remove, p_context);
    }
};