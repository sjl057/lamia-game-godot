#pragma once

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/typedefs.h"
#include "modules/lamia_game_tools/eventer/whiteboard.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "scene/resources/texture.h"

class EVBase;
class EventerSequence;
class EventerSequenceInstance;

class EVBase : public Resource
{
    GDCLASS(EVBase, Resource)

    friend class EventerSequence;
    friend class EventerSequenceInstance;
    friend class EventerEditorTree;

public:
    enum EditFlags
    {
        EDIT_FLAGS_NONE = 0,
        EDIT_FLAGS_CAN_MOVE = 1, // can drag
        EDIT_FLAGS_CAN_REPARENT = 2, // can drop onto another parent
        EDIT_FLAGS_CAN_MODIFY = 4, // can edit the resource by double clicking, and use the context menu

        EDIT_FLAGS_ALL = EDIT_FLAGS_CAN_MOVE | EDIT_FLAGS_CAN_REPARENT | EDIT_FLAGS_CAN_MODIFY
    };

private:
    Ref<EVBase> parent;
    int child_index = -1;
    Vector<Ref<EVBase>> children;

    bool enabled = true;
    AHashMap<StringName, Variant> default_properties;
    BitField<EditFlags> edit_flags;
    
    Ref<EventerSequence> sequence;
    Ref<EventerSequenceInstance> sequence_inst;
    Ref<EventerWhiteboard> whiteboard;

    // void _unlink();
    // void _change_sequence(Ref<EventerSequence> p_sequence);
    void _on_children_reordered();

protected:
    GDVIRTUAL0RC(String, _get_command_name)
    GDVIRTUAL0RC(String, _get_command_text);
    GDVIRTUAL0RC(Ref<Texture2D>, _get_command_icon);
    GDVIRTUAL0RC(String, _get_command_suffix);
    GDVIRTUAL0RC(String, _get_command_category);
    GDVIRTUAL0RC(Color, _get_command_color);
    GDVIRTUAL0RC(PackedStringArray, _get_configuration_warnings);

    GDVIRTUAL0RC(bool, _show_in_add_tree);
    GDVIRTUAL0RC(bool, _can_add_children);

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
    _FORCE_INLINE_ void set_enabled(const bool &p_enabled) { enabled = p_enabled; }
    _FORCE_INLINE_ bool is_enabled() const { return enabled; }
    _FORCE_INLINE_ void set_edit_flags(const BitField<EditFlags> &p_edit_flags) { edit_flags = p_edit_flags; }
    _FORCE_INLINE_ BitField<EditFlags> get_edit_flags() const { return edit_flags; }

    void add_child(Ref<EVBase> p_child, EditFlags p_edit_flags = EDIT_FLAGS_ALL);
    void remove_child(Ref<EVBase> p_child);
    void move_child(Ref<EVBase> p_child, const int &p_idx);
    bool has_child(const Ref<EVBase> p_child) const;
    _FORCE_INLINE_ bool is_root() const { return not parent.is_valid(); }

    Ref<EVBase> get_next() const;
    Ref<EVBase> get_next_enabled() const;
    Ref<EVBase> get_next_in_tree() const;
    Ref<EVBase> get_next_in_tree_enabled() const;
    int get_child_index() const { return child_index; };
    Ref<EVBase> get_child(int p_idx) const;
    _FORCE_INLINE_ int get_child_count() const { return children.size(); }

    _FORCE_INLINE_ void set_whiteboard(const Ref<EventerWhiteboard> p_whiteboard) { whiteboard = p_whiteboard; }
    _FORCE_INLINE_ Ref<EventerWhiteboard> get_whiteboard() const { return whiteboard; }
    Ref<EventerSequenceInstance> get_sequence_inst() const { return sequence_inst; };

    virtual String get_command_name();

    virtual String get_command_text();
    virtual Ref<Texture2D> get_command_icon();
    virtual String get_command_suffix();
    virtual String get_command_category();
    virtual Color get_command_color();
    virtual PackedStringArray get_configuration_warnings();

    virtual bool show_in_add_tree();
    virtual bool can_add_children();

    virtual void on_created();
    virtual void on_added();
    virtual void on_removed();
    virtual void on_child_order_changed();

    virtual void setup();
    virtual void start();
    virtual EVResult update(const float &p_delta);
    virtual void end();

};

VARIANT_BITFIELD_CAST(EVBase::EditFlags);


class EVCommand : public EVBase
{
    GDCLASS(EVCommand, EVBase);

protected:
    static void _bind_methods() {}
};



class EventerSequence : public Resource
{
    GDCLASS(EventerSequence, Resource);

private:
    Ref<EVCommand> root;

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_root(const Ref<EVCommand> &p_root)
    {
        root = p_root;
        if (root.is_valid())
        {
            root->set_edit_flags(EVBase::EDIT_FLAGS_NONE);
        }
    }
    _FORCE_INLINE_ Ref<EVCommand> get_root() const { return root; }
    Ref<EventerSequenceInstance> instantiate() const;
    EventerSequence();
};

class EventerSequenceInstance : public RefCounted
{
    GDCLASS(EventerSequenceInstance, RefCounted);

private:
    Ref<EVBase> root;
    Ref<EventerWhiteboard> whiteboard;
    Ref<EVBase> next_override;

protected:
    GDVIRTUAL0(_on_start);
    static void _bind_methods();

public:
    void setup(Ref<EventerSequence> p_sequence);
    _FORCE_INLINE_ Ref<EVBase> get_root() const { return root; }
    Ref<EVBase> get_next_in_tree(Ref<EVBase> p_from = nullptr);
    Ref<EVBase> get_next_sibling(Ref<EVBase> p_from = nullptr);

    virtual void on_start();
};

class EventerSequenceInterpreter : public RefCounted
{
    GDCLASS(EventerSequenceInterpreter, RefCounted);

private:
    Ref<EventerSequence> sequence;
    Ref<EventerSequenceInstance> sequence_instance;
    Ref<EVBase> current_command;
    bool running = false;

    void _next_in_tree();
    void _next_sibling();

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ Ref<EventerSequence> get_sequence() const { return sequence; };
    _FORCE_INLINE_ bool is_running() const { return running; }

    void start(Ref<EventerSequence> p_sequence = nullptr);
    EVResult update(const float &p_delta);
    void stop();
};