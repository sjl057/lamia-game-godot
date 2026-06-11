#pragma once

#include "core/object/gdvirtual.gen.inc"
#include "core/string/string_name.h"
#include "core/templates/a_hash_map.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "core/io/resource.h"
#include "modules/lamia_game_tools/general/defs.h"
#include <cstdint>

class EVBase : public Resource
{
    GDCLASS(EVBase, Resource)

private:
    EVBase *parent = nullptr;
    Vector<Ref<EVBase>> children;
    bool enabled = true;
    int child_index = -1;
    AHashMap<StringName, Variant> default_properties;

    void _on_children_reordered();

protected:
    GDVIRTUAL0R(String, _get_command_text);
    GDVIRTUAL0R(String, _get_command_suffix);
    GDVIRTUAL0R(String, _get_command_category);
    GDVIRTUAL0R(Color, _get_command_color);
    GDVIRTUAL0R(PackedStringArray, _get_configuration_warnings);

    GDVIRTUAL1R(bool, _can_drop_on, Ref<EVBase>);
    GDVIRTUAL1R(bool, _can_drop_above, Ref<EVBase>);
    GDVIRTUAL1R(bool, _can_drop_below, Ref<EVBase>);
    GDVIRTUAL0R(bool, _can_move);

    GDVIRTUAL0(_on_created);
    GDVIRTUAL0(_on_added);
    GDVIRTUAL0(_on_removed);
    GDVIRTUAL0(_on_child_order_changed);

    GDVIRTUAL0(_setup);
    GDVIRTUAL0(_start);
    GDVIRTUAL1R(EVResult, _update, float);
    GDVIRTUAL0(_end);

    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_parent(const Ref<EVBase> p_parent) { parent = p_parent.ptr(); }
    _FORCE_INLINE_ Ref<EVBase> get_parent() const { return Ref<EVBase>(parent); }
    void set_children(const TypedArray<Ref<EVBase>> &p_children);
    TypedArray<Ref<EVBase>> get_children() const;
    _FORCE_INLINE_ void set_child_index(const int &p_idx) { child_index = p_idx; }
    _FORCE_INLINE_ int get_child_index() const { return child_index; }
    _FORCE_INLINE_ void set_enabled(const bool &p_enabled) { enabled = p_enabled; }
    _FORCE_INLINE_ bool is_enabled() const { return enabled; }

    void add_child(Ref<EVBase> p_child);
    void remove_child(Ref<EVBase> p_child);
    void move_child(Ref<EVBase> p_child, const int &p_to);
    bool has_child(const Ref<EVBase> p_child) const;
    _FORCE_INLINE_ bool is_root() const { return not parent; } 
    Ref<EVBase> get_root() const;
    Ref<EVBase> get_child(int p_idx) const;
    _FORCE_INLINE_ uint32_t get_child_count() const { return children.size(); }
    // Ref<EVBase> get_next_child(bool p_must_be_enabled) const; // null if no child
    Ref<EVBase> get_next_sibling(bool p_must_be_enabled = true) const; // null if no sibling
    Ref<EVBase> get_next_in_tree(bool p_must_be_enabled = true) const; // null in the way you'd expect

    bool is_descendent_of(Ref<EVBase> p_command) const;
    bool is_ancestor_of(Ref<EVBase> p_command) const;

    virtual String get_command_name() const;

    virtual String get_command_text();
    virtual String get_command_suffix();
    virtual String get_command_category();
    virtual Color get_command_color();
    virtual PackedStringArray get_configuration_warnings();

    virtual bool can_drop_on(Ref<EVBase> p_command);
    virtual bool can_drop_above(Ref<EVBase> p_command);
    virtual bool can_drop_below(Ref<EVBase> p_command);
    virtual bool can_move();

    virtual void on_created();
    virtual void on_added();
    virtual void on_removed();
    virtual void on_child_order_changed();

    virtual void setup();
    virtual void start();
    virtual EVResult update(float p_delta);
    virtual void end();

};