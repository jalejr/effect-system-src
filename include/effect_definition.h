#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include "effect_modifier.h"
#include "godot_cpp/core/binder_common.hpp"

using namespace godot;

class EffectModifier;

class EffectDefinition : public Resource {
    GDCLASS(EffectDefinition, Resource)

public:
    enum StackPolicy { IGNORE, REFRESH, ACCUMULATE, ACCUMULATE_REFRESH };

private:
    StringName effect_tag;
    float duration = 0.0f;
    float period = 0.0f;
    bool tick_on_apply = true;
    StackPolicy stack_policy = REFRESH;
    int stack_limit = 0;
    TypedArray<EffectModifier> modifiers;

    TypedArray<StringName> application_required_tags;
    TypedArray<StringName> application_blocked_tags;
    TypedArray<StringName> ongoing_required_tags;
    TypedArray<StringName> ongoing_blocked_tags;

protected:
    static void _bind_methods();

public:
    bool is_instant() const { return duration == 0.0f; }
    bool is_infinite() const { return duration < 0.0f; }
    bool is_periodic() const { return period > 0.0f; }
    bool is_tick_on_apply() const { return tick_on_apply; }

    StringName get_effect_tag();
    void set_effect_tag(const StringName& p_tag);

    float get_duration() const;
    void set_duration(float p_duration);

    float get_period() const;
    void set_period(float p_period);

    bool get_tick_on_apply() const;
    void set_tick_on_apply(bool p_tick_on_apply);

    StackPolicy get_stack_policy() const;
    void set_stack_policy(StackPolicy p_policy);

    int get_stack_limit() const;
    void set_stack_limit(int p_stack_limit);

    TypedArray<EffectModifier> get_modifiers() const;
    void set_modifiers(const TypedArray<EffectModifier>& p_modifiers);

    TypedArray<StringName> get_application_required_tags() const { return application_required_tags; }
    void set_application_required_tags(const TypedArray<StringName>& p_tags) { application_required_tags = p_tags; }
 
    TypedArray<StringName> get_application_blocked_tags() const { return application_blocked_tags; }
    void set_application_blocked_tags(const TypedArray<StringName>& p_tags) { application_blocked_tags = p_tags; }
 
    TypedArray<StringName> get_ongoing_required_tags() const { return ongoing_required_tags; }
    void set_ongoing_required_tags(const TypedArray<StringName>& p_tags) { ongoing_required_tags = p_tags; }
 
    TypedArray<StringName> get_ongoing_blocked_tags() const { return ongoing_blocked_tags; }
    void set_ongoing_blocked_tags(const TypedArray<StringName>& p_tags) { ongoing_blocked_tags = p_tags; }

    bool has_ongoing_requirements() const;
};

VARIANT_ENUM_CAST(EffectDefinition::StackPolicy)