#pragma once

#include <godot_cpp/variant/string_name.hpp>
#include "effect_definition.h"

using namespace godot;

class EffectDefinition;
class EffectContext;

struct EffectInstance {
    uint32_t instance_id = 0;
    Ref<EffectDefinition> definition;
    Ref<EffectContext> context;
    float time_remaining = 0.0f;
    float period_accumulator = 0.0f;
    bool is_supressed = false;

    void initialize(uint32_t p_id,
            const Ref<EffectDefinition>& p_definition,
            const Ref<EffectContext>& p_context) {
        instance_id = p_id;
        definition = p_definition;
        context = p_context;
        time_remaining = p_definition->get_duration();
        is_supressed = false;
    }

    bool has_ongoing_requirements() const {
        return definition->has_ongoing_requirements();
    }
};