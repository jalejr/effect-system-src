#pragma once

#include <godot_cpp/classes/node.hpp>
#include "effect_definition.h"
#include "effect_instance.h"
#include "effect_context.h"
#include "tag_container.h"
#include "tag_manager.h"
#include "identity_hash.h"

using namespace godot;

class EffectSystem : public Node {
    GDCLASS(EffectSystem, Node)

private:
    std::vector<EffectInstance> instances;
    std::unordered_map<uint32_t, size_t, IdentityHash> instance_id_to_instances_idx;
    std::unordered_map<StringName, std::vector<uint32_t>> effect_tag_to_ids;

    TagManager* tag_manager = nullptr;

    TagContainer* tag_container = nullptr;
    uint32_t next_id = 1;
    bool is_initialized = false;

    friend class EffectContext;
    // friend class EffectUtil;
protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    EffectSystem() = default;

    void _ready() override;
    void _physics_process(double delta) override;

    TagContainer* get_tag_container() const { return tag_container; }
    void set_tag_container(TagContainer* p_container) { tag_container = p_container; }

    uint32_t apply_effect(const Ref<EffectDefinition>& p_definition, 
            const Ref<EffectContext>& p_context);
    void remove_effect(uint32_t p_instance_id);
    void remove_effects_by_tag(const StringName& p_definition_tag);

    bool is_effect_active(const StringName& p_definition_tag) const;
    float get_remaining_time(const StringName& p_definition_tag) const;
    float get_normalized_time(const StringName& p_definition_tag) const;

    void  remove_effects_by_tags(const TypedArray<StringName>& p_tags);
    Array get_active_effects_by_tags(const TypedArray<StringName>& p_tags) const;

private:
    uint32_t _create_instance(const Ref<EffectDefinition>& p_definition, 
            const Ref<EffectContext>& p_context);
    void _apply_modifiers(const Ref<EffectDefinition>& p_definition, EffectInstance* p_instance,
            const Ref<EffectContext>& p_context);
    void _remove_modifiers(EffectInstance* p_instance);
    void _tick_periodic(EffectInstance& instance);

    bool _passes_application_checks(const Ref<EffectDefinition>& p_definition) const;
    bool _passes_ongoing_checks(const EffectInstance& p_instance) const;
    void _connect_ongoing_watchers(EffectInstance& p_instance);
    void _disconnect_ongoing_watchers(EffectInstance& p_instance);
    void _suppress_instance(EffectInstance& p_instance);
    void _unsuppress_instance(EffectInstance& p_instance);
    void _reevaluate_ongoing(const StringName& p_tag, int32_t p_count, int64_t p_instance_id);

};