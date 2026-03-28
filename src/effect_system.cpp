#include "effect_system.h"
#include "effect_definition.h"
#include "effect_instance.h"
#include "godot_cpp/variant/typed_array.hpp"
#include "tag_manager.h"
#include <algorithm>

void EffectSystem::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_tag_container"),
        &EffectSystem::get_tag_container);
    ClassDB::bind_method(D_METHOD("set_tag_container", "container"),
        &EffectSystem::set_tag_container);
    ClassDB::bind_method(D_METHOD("apply_effect", "definition", "context"),
        &EffectSystem::apply_effect);
    ClassDB::bind_method(D_METHOD("remove_effect", "effect_instance_id"),
        &EffectSystem::remove_effect);
    ClassDB::bind_method(D_METHOD("remove_effects_by_tag", "effect_tag"),
        &EffectSystem::remove_effects_by_tag);
    ClassDB::bind_method(D_METHOD("is_effect_active", "effect_tag"),
        &EffectSystem::is_effect_active);
    ClassDB::bind_method(D_METHOD("get_remaining_time", "effect_tag"),
        &EffectSystem::get_remaining_time);
    ClassDB::bind_method(D_METHOD("get_normalized_time", "effect_tag"),
        &EffectSystem::get_normalized_time);
    ClassDB::bind_method(D_METHOD("remove_effects_by_tags", "tag_container"),
        &EffectSystem::remove_effects_by_tags);
    ClassDB::bind_method(D_METHOD("get_active_effects_by_tags", "tag_container"),
        &EffectSystem::get_active_effects_by_tags);
    ClassDB::bind_method(
        D_METHOD("_reevaluate_ongoing", "tag", "count", "instance_id"),
        &EffectSystem::_reevaluate_ongoing);

    ADD_SIGNAL(MethodInfo("effect_applied",
        PropertyInfo(Variant::STRING_NAME, "effect_tag"),
        PropertyInfo(Variant::INT, "instance_id")));
    ADD_SIGNAL(MethodInfo("effect_removed",
        PropertyInfo(Variant::STRING_NAME, "effect_tag"),
        PropertyInfo(Variant::INT, "instance_id")));
    
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tag_container",
        PROPERTY_HINT_NODE_TYPE, "TagContainer"),
        "set_tag_container", "get_tag_container");
}

void EffectSystem::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_READY: _ready(); 
            break;
    }
}

void EffectSystem::_ready() {
    tag_manager = TagManager::get_singleton();

    ERR_FAIL_NULL_MSG(tag_container,
        "EffectSystem: tag_container not assigned. "
        "Drag a TagContainer node into the inspector slot.");
    
    set_physics_process(true);
    is_initialized = true;
}

void EffectSystem::_physics_process(double delta) {
    if (!is_initialized || instances.empty()) return;

    std::vector<uint32_t> expired;
    for (auto& instance : instances) {
        if (instance.definition->get_period() > 0.0f) {
            instance.period_accumulator += static_cast<float>(delta);

            while (instance.period_accumulator >= instance.definition->get_period()) {
                instance.period_accumulator -= instance.definition->get_period();
                if (!instance.is_supressed) _tick_periodic(instance);
            }
        }

        if (instance.definition->is_infinite()) continue;

        instance.time_remaining -= static_cast<float>(delta);

        if (instance.time_remaining <= 0.0f) expired.push_back(instance.instance_id);
    }
    
    for (uint32_t id : expired) remove_effect(id);
}

uint32_t EffectSystem::apply_effect(const Ref<EffectDefinition>& definition,
            const Ref<EffectContext>& context) {
    ERR_FAIL_COND_V_MSG(!is_initialized, 0,
        "EffectSystem: apply_effect called before _ready().");
    ERR_FAIL_COND_V_MSG(definition.is_null(), 0,
        "EffectSystem: null definition.");
    ERR_FAIL_COND_V_MSG(context.is_null(), 0,
        "EffectSystem: null context.");
    ERR_FAIL_COND_V_MSG(tag_manager->get_tag_index(definition->get_effect_tag()) == TagManager::INVALID_INDEX, 0,
        vformat("EffectDefinition: effect_tag '%s' is not a registered tag. "
                "Register it in TagManager before assigning.", definition->get_effect_tag()));
    
    if (!_passes_application_checks(definition)) return 0;

    if (!definition->is_instant()) {

        auto it = effect_tag_to_ids.find(definition->get_effect_tag());
        if (it != effect_tag_to_ids.end() && !it->second.empty()) {

            int limit = definition->get_stack_limit();
            
            switch (definition->get_stack_policy()) {
                case EffectDefinition::IGNORE:
                    return 0;
                case EffectDefinition::REFRESH: {
                    uint32_t existing_id = it->second.front();
                    auto idx_it = instance_id_to_instances_idx.find(existing_id);
                    if (idx_it != instance_id_to_instances_idx.end())
                        instances[idx_it->second].time_remaining = definition->get_duration();

                    return existing_id;
                }
                case EffectDefinition::ACCUMULATE:
                    if (limit > 0 && static_cast<int>(it->second.size()) >= limit) {
                        uint32_t oldest_id = it->second.front();
                        auto idx_it = instance_id_to_instances_idx.find(oldest_id);

                        if (idx_it != instance_id_to_instances_idx.end())
                            instances[idx_it->second].time_remaining = definition->get_duration();

                        it->second.erase(it->second.begin());
                        it->second.push_back(oldest_id);

                        return oldest_id;
                    }
                    
                    break;
                case EffectDefinition::ACCUMULATE_REFRESH:
                    for (uint32_t existing_id : it->second) {
                        auto idx_it = instance_id_to_instances_idx.find(existing_id);
                        if (idx_it != instance_id_to_instances_idx.end())
                            instances[idx_it->second].time_remaining = definition->get_duration();
                    }

                    if (limit > 0 && static_cast<int>(it->second.size()) >= limit) return 0;

                    break;
            }
        }
    }

    if (definition->is_instant()) {
        const TypedArray<EffectModifier>& modifiers = definition->get_modifiers();
        for (int i = 0; i < modifiers.size(); i++) {
            Ref<EffectModifier> mod = modifiers[i];
            if (mod.is_null()) continue;
            context->instance_id    = 0;
            context->modifier_index = i;
            mod->apply(context);
        }

        return 0;
    }

    return _create_instance(definition, context);
}

void EffectSystem::remove_effect(uint32_t p_instance_id)
{
    auto idx_it = instance_id_to_instances_idx.find(p_instance_id);
    ERR_FAIL_COND_MSG(idx_it == instance_id_to_instances_idx.end(),
        "EffectSystem: unknown instance_id in remove_effect.");

    size_t index = idx_it->second;
    EffectInstance& instance = instances[index];
    StringName effect_tag = instance.definition->get_effect_tag();

    _remove_modifiers(&instance);

    auto& id_list = effect_tag_to_ids[effect_tag];
    id_list.erase(
        std::remove(id_list.begin(), id_list.end(), p_instance_id),
        id_list.end());

    if (id_list.empty()) effect_tag_to_ids.erase(effect_tag);

    instance_id_to_instances_idx.erase(idx_it);

    if (index != instances.size() - 1) {
        instances[index] = std::move(instances.back());
        instance_id_to_instances_idx[instances[index].instance_id] = index;
    }
    
    instances.pop_back();

    emit_signal("effect_removed", effect_tag, static_cast<int>(p_instance_id));
}

void EffectSystem::remove_effects_by_tag(const StringName& p_definition_tag)
{
    auto it = effect_tag_to_ids.find(p_definition_tag);
    if (it == effect_tag_to_ids.end()) return;
    std::vector<uint32_t> ids = it->second;
    for (uint32_t id : ids) remove_effect(id);
}

bool EffectSystem::is_effect_active(const StringName& p_definition_tag) const
{
    auto it = effect_tag_to_ids.find(p_definition_tag);
    return it != effect_tag_to_ids.end() && !it->second.empty();
}

float EffectSystem::get_remaining_time(const StringName& p_definition_tag) const
{
    auto it_1 = effect_tag_to_ids.find(p_definition_tag);
    if (it_1 == effect_tag_to_ids.end() || it_1->second.empty()) return 0.0f;
    const auto& [_1, ids] = *it_1;

    auto it_2 = instance_id_to_instances_idx.find(ids.front());
    if (it_2 == instance_id_to_instances_idx.end()) return 0.0f;
    const auto& [_2, slot_index] = *it_2;

    return instances[slot_index].time_remaining;
}

float EffectSystem::get_normalized_time(const StringName& p_definition_tag) const
{
    // 6 lines could be made into a function to get first instance found
    auto it_1 = effect_tag_to_ids.find(p_definition_tag);
    if (it_1 == effect_tag_to_ids.end() || it_1->second.empty()) return 0.0f;
    const auto& [_1, ids] = *it_1;

    auto it_2 = instance_id_to_instances_idx.find(ids.front());
    if (it_2 == instance_id_to_instances_idx.end()) return 0.0f;
    const auto& [_2, slot_index] = *it_2;

    const EffectInstance& instance = instances[slot_index];
    float total = instance.definition->get_duration();
    return total <= 0.0f ? 0.0f : instance.time_remaining / total;
}

void EffectSystem::remove_effects_by_tags(const TypedArray<StringName>& p_tags)
{
    std::vector<uint32_t> to_remove;

    for (const auto& [effect_tag, ids] : effect_tag_to_ids) {
        uint32_t effect_tag_index = tag_manager->get_tag_index(effect_tag);

        if (effect_tag_index == TagManager::INVALID_INDEX) continue;

        for (int i = 0; i < p_tags.size(); i++) {
            uint32_t query_tag_index = tag_manager->get_tag_index(p_tags[i]);
            if (query_tag_index == effect_tag_index || 
                    tag_manager->is_parent_of(query_tag_index, effect_tag_index)
            ) {
                to_remove.insert(to_remove.end(), ids.begin(), ids.end());
                break;
            }
        }
    }

    for (uint32_t id : to_remove) remove_effect(id);
}

Array EffectSystem::get_active_effects_by_tags(const TypedArray<StringName>& p_tags) const
{
    Array result;

    for (const auto& [effect_tag, ids] : effect_tag_to_ids) {
        uint32_t effect_tag_index = tag_manager->get_tag_index(effect_tag);

        if (effect_tag_index == TagManager::INVALID_INDEX) continue;

        for (int i = 0; i < p_tags.size(); i++) {
            uint32_t query_tag_index = tag_manager->get_tag_index(p_tags[i]);
            if (query_tag_index == effect_tag_index || 
                    tag_manager->is_parent_of(query_tag_index, effect_tag_index)
            ) {
                for (uint32_t id : ids) result.push_back(id);
                break;
            }
        }
    }
    
    return result;
}

uint32_t EffectSystem::_create_instance(const Ref<EffectDefinition>& p_definition, 
            const Ref<EffectContext>& p_context)
{
    uint32_t id = next_id++;
    instances.emplace_back();
    EffectInstance& instance = instances.back();
    instance.initialize(id, p_definition, p_context);
    instance_id_to_instances_idx[id] = instances.size() - 1;
    effect_tag_to_ids[p_definition->get_effect_tag()].push_back(id);

    if (instance.has_ongoing_requirements()) {
        _connect_ongoing_watchers(instance);
        instance.is_supressed = !_passes_ongoing_checks(instance);
    }

    if (!p_definition->is_periodic() && !instance.is_supressed) {
        _apply_modifiers(p_definition, &instance, p_context);
    }

    if (p_definition->is_periodic() && p_definition->is_tick_on_apply() && !instance.is_supressed) {
        _tick_periodic(instance);
    }

    emit_signal("effect_applied", p_definition->get_effect_tag(), static_cast<int>(id));
    
    return id;
}

void EffectSystem::_apply_modifiers(const Ref<EffectDefinition>& p_definition, EffectInstance* p_instance,
            const Ref<EffectContext>& p_context)
{
    p_context->instance_id = p_instance->instance_id;
    const TypedArray<EffectModifier>& modifiers = p_definition->get_modifiers();

    for (int i = 0; i < modifiers.size(); i++) {
        Ref<EffectModifier> modifier = modifiers[i];
        if (modifier.is_null()) continue;
        
        p_context->modifier_index = i;
        modifier->apply(p_context);
    }
}

void EffectSystem::_remove_modifiers(EffectInstance* p_instance)
{
    if (p_instance->definition->is_periodic()) return;

    p_instance->context->instance_id = p_instance->instance_id;
    const TypedArray<EffectModifier>& modifiers = p_instance->definition->get_modifiers();
    
    for (int i = modifiers.size() - 1; i >= 0; i--) {
        Ref<EffectModifier> modifier = modifiers[i];
        if (modifier.is_null()) continue;
        
        p_instance->context->modifier_index = i;
        modifier->remove(p_instance->context);
    }
}

void EffectSystem::_tick_periodic(EffectInstance& instance)
{
    instance.context->instance_id = 0;
    const TypedArray<EffectModifier>& modifiers = instance.definition->get_modifiers();
    
    for (int i = 0; i < modifiers.size(); i++) {
        Ref<EffectModifier> modifier = modifiers[i];
        if (modifier.is_null()) continue;

        instance.context->modifier_index = i;
        modifier->apply(instance.context);
    }
}

bool EffectSystem::_passes_application_checks(const Ref<EffectDefinition>& p_definition) const
{
    if (!tag_container->has_all_tags(p_definition->get_application_required_tags())) return false;

    if (tag_container->has_any_tags(p_definition->get_application_blocked_tags())) return false;

    return true;
}

bool EffectSystem::_passes_ongoing_checks(const EffectInstance& p_instance) const
{
    if (!tag_container->has_all_tags(p_instance.definition->get_ongoing_required_tags())) return false;

    if (tag_container->has_any_tags(p_instance.definition->get_ongoing_blocked_tags())) return false;

    return true;
}

void EffectSystem::_connect_ongoing_watchers(EffectInstance& p_instance)
{
    Callable callback = callable_mp(this, &EffectSystem::_reevaluate_ongoing)
        .bind(static_cast<int64_t>(p_instance.instance_id));
    
    const TypedArray<StringName>& required_tags = p_instance.definition->get_ongoing_required_tags();
    for (int i = 0; i < required_tags.size(); i++)
        tag_container->watch_tag(StringName(required_tags[i]), callback);
 
    const TypedArray<StringName>& blocked_tags = p_instance.definition->get_ongoing_blocked_tags();
    for (int i = 0; i < blocked_tags.size(); i++)
        tag_container->watch_tag(StringName(blocked_tags[i]), callback);
}

void EffectSystem::_disconnect_ongoing_watchers(EffectInstance& p_instance)
{
    Callable callback = callable_mp(this, &EffectSystem::_reevaluate_ongoing)
        .bind(static_cast<int64_t>(p_instance.instance_id));
 
    const TypedArray<StringName>& required_tags = p_instance.definition->get_ongoing_required_tags();
    for (int i = 0; i < required_tags.size(); i++)
        tag_container->unwatch_tag(StringName(required_tags[i]), callback);
 
    const TypedArray<StringName>& blocked_tags = p_instance.definition->get_ongoing_blocked_tags();
    for (int i = 0; i < blocked_tags.size(); i++)
        tag_container->unwatch_tag(StringName(blocked_tags[i]), callback);
}

void EffectSystem::_suppress_instance(EffectInstance& p_instance)
{
    if (!p_instance.is_supressed) return;

    p_instance.is_supressed = true;
    _remove_modifiers(&p_instance);
}

void EffectSystem::_unsuppress_instance(EffectInstance& p_instance)
{
    if (p_instance.is_supressed) return;

    p_instance.is_supressed = false;
    _apply_modifiers(p_instance.definition, &p_instance, p_instance.context);
}

void EffectSystem::_reevaluate_ongoing(const StringName& p_tag, int32_t p_count, int64_t p_instance_id)
{
    auto it = instance_id_to_instances_idx.find(static_cast<uint32_t>(p_instance_id));
    if (it == instance_id_to_instances_idx.end()) return;

    EffectInstance& instance = instances[it->second];

    if (_passes_ongoing_checks(instance)) _unsuppress_instance(instance);
    else _suppress_instance(instance);
}