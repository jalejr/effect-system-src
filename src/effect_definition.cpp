#include "effect_definition.h"
#include "tag_manager.h"
#include "effect_modifier.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void EffectDefinition::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_effect_tag"), &EffectDefinition::get_effect_tag);
    ClassDB::bind_method(D_METHOD("set_effect_tag", "tag_name"), &EffectDefinition::set_effect_tag);

    ClassDB::bind_method(D_METHOD("get_duration"), &EffectDefinition::get_duration);
    ClassDB::bind_method(D_METHOD("set_duration", "duration"), &EffectDefinition::set_duration);

    ClassDB::bind_method(D_METHOD("get_period"), &EffectDefinition::get_period);
    ClassDB::bind_method(D_METHOD("set_period", "period"), &EffectDefinition::set_period);

    ClassDB::bind_method(D_METHOD("get_tick_on_apply"), &EffectDefinition::get_tick_on_apply);
    ClassDB::bind_method(D_METHOD("set_tick_on_apply", "tick_on_apply"), &EffectDefinition::set_tick_on_apply);

    ClassDB::bind_method(D_METHOD("is_instant"), &EffectDefinition::is_instant);
    ClassDB::bind_method(D_METHOD("is_infinite"), &EffectDefinition::is_infinite);
    ClassDB::bind_method(D_METHOD("is_periodic"), &EffectDefinition::is_periodic);
    ClassDB::bind_method(D_METHOD("is_tick_on_apply"), &EffectDefinition::is_tick_on_apply);

    ClassDB::bind_method(D_METHOD("get_stack_policy"), &EffectDefinition::get_stack_policy);
    ClassDB::bind_method(D_METHOD("set_stack_policy", "policy"), &EffectDefinition::set_stack_policy);

    ClassDB::bind_method(D_METHOD("get_stack_limit"), &EffectDefinition::get_stack_limit);
    ClassDB::bind_method(D_METHOD("set_stack_limit", "stack_limit"), &EffectDefinition::set_stack_limit);

    ClassDB::bind_method(D_METHOD("get_modifiers"), &EffectDefinition::get_modifiers);
    ClassDB::bind_method(D_METHOD("set_modifiers", "modifiers"), &EffectDefinition::set_modifiers);

    ClassDB::bind_method(D_METHOD("get_application_required_tags"),
        &EffectDefinition::get_application_required_tags);
    ClassDB::bind_method(D_METHOD("set_application_required_tags", "tags"),
        &EffectDefinition::set_application_required_tags);
    
    ClassDB::bind_method(D_METHOD("get_application_blocked_tags"),
        &EffectDefinition::get_application_blocked_tags);
    ClassDB::bind_method(D_METHOD("set_application_blocked_tags", "tags"),
        &EffectDefinition::set_application_blocked_tags);

    ClassDB::bind_method(D_METHOD("get_ongoing_required_tags"),
        &EffectDefinition::get_ongoing_required_tags);
    ClassDB::bind_method(D_METHOD("set_ongoing_required_tags", "tags"),
        &EffectDefinition::set_ongoing_required_tags);

    ClassDB::bind_method(D_METHOD("get_ongoing_blocked_tags"),
        &EffectDefinition::get_ongoing_blocked_tags);
    ClassDB::bind_method(D_METHOD("set_ongoing_blocked_tags", "tags"),
        &EffectDefinition::set_ongoing_blocked_tags);

    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "effect_tag",
        PROPERTY_HINT_PLACEHOLDER_TEXT, "tag"),
        "set_effect_tag", "get_effect_tag");

    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration"),
        "set_duration", "get_duration");

    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "period"),
        "set_period", "get_period");
    
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tick_on_apply"),
        "set_tick_on_apply", "get_tick_on_apply");
    
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stack_policy", PROPERTY_HINT_ENUM,
        "Ignore,Refresh,Accumulate,AccumulateRefresh"),
        "set_stack_policy", "get_stack_policy");
    
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stack_limit"),
        "set_stack_limit", "get_stack_limit");
    
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "modifiers", PROPERTY_HINT_TYPE_STRING,
        String::num(Variant::OBJECT) + "/" +
        String::num(PROPERTY_HINT_RESOURCE_TYPE) + ":EffectModifier"),
        "set_modifiers", "get_modifiers");

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "application_required_tags",
        PROPERTY_HINT_ARRAY_TYPE, "StringName/0:tag"),
        "set_application_required_tags", "get_application_required_tags");
 

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "application_blocked_tags",
        PROPERTY_HINT_ARRAY_TYPE, "StringName/0:tag"),
        "set_application_blocked_tags", "get_application_blocked_tags");

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "ongoing_required_tags",
        PROPERTY_HINT_ARRAY_TYPE, "StringName/0:tag"),
        "set_ongoing_required_tags", "get_ongoing_required_tags");
 
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "ongoing_blocked_tags",
        PROPERTY_HINT_ARRAY_TYPE, "StringName/0:tag"),
        "set_ongoing_blocked_tags", "get_ongoing_blocked_tags");

    BIND_ENUM_CONSTANT(IGNORE);
    BIND_ENUM_CONSTANT(REFRESH);
    BIND_ENUM_CONSTANT(ACCUMULATE);
    BIND_ENUM_CONSTANT(ACCUMULATE_REFRESH);
}

StringName EffectDefinition::get_effect_tag() { return effect_tag; }
void EffectDefinition::set_effect_tag(const StringName& p_tag) {
    TagManager* tag_manager = TagManager::get_singleton();
    ERR_FAIL_COND_MSG(!tag_manager,
        "EffectDefinition: TagManager not available.");
    
    effect_tag = p_tag;
}

float EffectDefinition::get_duration() const { return duration; }
void EffectDefinition::set_duration(float p_duration) { duration = p_duration; }

float EffectDefinition::get_period() const { return period; }
void EffectDefinition::set_period(float p_period) { period = p_period; } 

bool EffectDefinition::get_tick_on_apply() const { return tick_on_apply; }
void EffectDefinition::set_tick_on_apply(bool p_tick_on_apply) { tick_on_apply = p_tick_on_apply; }

EffectDefinition::StackPolicy EffectDefinition::get_stack_policy() const { return stack_policy; }
void EffectDefinition::set_stack_policy(StackPolicy p_policy) { stack_policy = p_policy; }

int EffectDefinition::get_stack_limit() const { return stack_limit; }
void EffectDefinition::set_stack_limit(int p_stack_limit) { stack_limit = p_stack_limit; }

TypedArray<EffectModifier> EffectDefinition::get_modifiers() const { return modifiers; }
void EffectDefinition::set_modifiers(const TypedArray<EffectModifier>& p_modifiers) { modifiers = p_modifiers; }

bool EffectDefinition::has_ongoing_requirements() const {
    return ongoing_required_tags.size() > 0 || ongoing_blocked_tags.size() > 0;
}