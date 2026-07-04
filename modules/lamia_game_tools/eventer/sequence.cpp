#include "sequence.h"
#include "core/error/error_macros.h"
#include "core/object/object.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "modules/lamia_game_tools/eventer/builtin_commands.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "scene/resources/theme.h"

EventerSequence::EventerSequence()
{

}

void EventerSequence::_bind_methods()
{
    BIND(D_METHOD("set_root", "root"), &EventerSequence::set_root);
    BIND(D_METHOD("get_root"), &EventerSequence::get_root);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root", PROPERTY_HINT_RESOURCE_TYPE, "EVCommand", PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_root", "get_root");
}

Ref<EventerSequenceInstance> EventerSequence::instantiate() const
{
    Ref<EventerSequenceInstance> instance;
    instance.instantiate();
    instance->setup(Ref<EventerSequence>(this));
    return instance;
}

void EventerSequenceInstance::_bind_methods()
{
    GDVIRTUAL_BIND(_on_start);
}

void EventerSequenceInstance::setup(Ref<EventerSequence> p_sequence)
{
    whiteboard.instantiate();
    root = p_sequence->get_root()->duplicate_deep();

    Ref<EVBase> next = root;
    while (next.is_valid())
    {
        // next->save_default_properties();
        next->sequence_inst = Ref<EventerSequenceInstance>(this);
        next->whiteboard = whiteboard;
        next->setup();
        next = next->get_next_enabled();
    }
}



Ref<EVBase> EventerSequenceInstance::get_next_in_tree(Ref<EVBase> p_from)
{
    if (next_override.is_valid())
    {
        auto override = next_override;
        next_override.unref();
        return override;
    }
    if (p_from == nullptr or not p_from.is_valid()) { return nullptr; }

    Ref<EVBase> next = p_from->get_next_in_tree_enabled();
    if (next.is_valid())
    {
        //next->load_default_properties();
    }
    print_line(next);

    return next;
}

Ref<EVBase> EventerSequenceInstance::get_next_sibling(Ref<EVBase> p_from)
{
    if (next_override.is_valid())
    {
        auto override = next_override;
        next_override.unref();
        return override;
    }
    if (p_from == nullptr or not p_from.is_valid()) { return nullptr; }

    Ref<EVBase> next = p_from->get_next();
    if (next.is_valid())
    {
        //next->load_default_properties();
    }

    return next;
}

void EventerSequenceInstance::on_start()
{
    GDVIRTUAL_CALL(_on_start);
}



void EventerSequenceInterpreter::_bind_methods()
{
    BIND(D_METHOD("get_sequence"), &EventerSequenceInterpreter::get_sequence);
    BIND(D_METHOD("is_running"), &EventerSequenceInterpreter::is_running);
    BIND(D_METHOD("start", "sequence"), &EventerSequenceInterpreter::start);
    BIND(D_METHOD("update", "delta"), &EventerSequenceInterpreter::update);
    BIND(D_METHOD("stop"), &EventerSequenceInterpreter::stop);

    ADD_SIGNAL(MethodInfo("updated", PropertyInfo(Variant::INT, "last_result", PROPERTY_HINT_ENUM, "EVResult")));
    ADD_SIGNAL(MethodInfo("done"));
}

void EventerSequenceInterpreter::_next_in_tree()
{
    if (not running) { return; }

    current_command = sequence_instance->get_next_in_tree(current_command);
    if (current_command.is_valid())
    {
        current_command->start();
    }
    else
    {
        stop();
    }
}

void EventerSequenceInterpreter::_next_sibling()
{
    if (not running) { return; }

    current_command = sequence_instance->get_next_sibling(current_command);
    if (current_command.is_valid())
    {
        current_command->start();
    }
    else
    {
        stop();
    }
}

void EventerSequenceInterpreter::start(Ref<EventerSequence> p_sequence)
{
    if (running) { WARN_PRINT("Interpreter is already running"); return; }
    if (p_sequence.is_valid())
    {
        if (p_sequence->get_root()->get_child_count() == 0)
        {
            emit_signal("done");
            return;
        }
        else
        {
            sequence = p_sequence;
        }
    }

    sequence_instance = sequence->instantiate();
    current_command = sequence_instance->get_root();
    running = true;
    _next_in_tree();
}

EVResult EventerSequenceInterpreter::update(const float &p_delta)
{
    if (not running) { return EV_INVALID; }
    ERR_FAIL_COND_V_MSG(not current_command.is_valid(), EV_INVALID, "Invalid command");

    EVResult status = current_command->update(p_delta);
    switch (status)
    {
		case EV_NEXT_IN_TREE:
            _next_in_tree();
            break;
		case EV_NEXT_SIBLING:
            _next_sibling();
            break;
		case EV_ABORT:
            stop();
			break;
		default:
            ERR_PRINT("invalid status");
            status = EV_INVALID;
            stop();
			break;
	}
	emit_signal("updated", status);
    return status;
}

void EventerSequenceInterpreter::stop()
{
    if (not running) { return; }
    running = false;
    sequence.unref();
    sequence_instance.unref();
    current_command.unref();
    emit_signal("done");
}

void EVBase::_bind_methods()
{
    BIND_BITFIELD_FLAG(EDIT_FLAGS_NONE);
    BIND_BITFIELD_FLAG(EDIT_FLAGS_CAN_MOVE);
    BIND_BITFIELD_FLAG(EDIT_FLAGS_CAN_REPARENT);
    BIND_BITFIELD_FLAG(EDIT_FLAGS_CAN_MODIFY);
    BIND_BITFIELD_FLAG(EDIT_FLAGS_ALL);

    BIND(D_METHOD("_set_children", "children"), &EVBase::set_children);
    BIND(D_METHOD("_get_children"), &EVBase::get_children);
    BIND(D_METHOD("_set_enabled", "enabled"), &EVBase::set_enabled);
    BIND(D_METHOD("_is_enabled"), &EVBase::is_enabled);
    BIND(D_METHOD("_set_edit_flags", "flags"), &EVBase::set_edit_flags);
    BIND(D_METHOD("_get_edit_flags"), &EVBase::get_edit_flags);

    BIND_D(D_METHOD("_add_child", "child", "edit_flags"), &EVBase::add_child, DEFVAL(EDIT_FLAGS_ALL));
    BIND(D_METHOD("_remove_child", "child"), &EVBase::remove_child);
    BIND(D_METHOD("_move_child", "child", "idx"), &EVBase::move_child);

    BIND(D_METHOD("get_whiteboard"), &EVBase::get_whiteboard);

    GDVIRTUAL_BIND(_get_command_name);
    GDVIRTUAL_BIND(_get_command_text);
    GDVIRTUAL_BIND(_get_command_icon)
    GDVIRTUAL_BIND(_get_command_suffix);
    GDVIRTUAL_BIND(_get_command_category);
    GDVIRTUAL_BIND(_get_command_color);
    GDVIRTUAL_BIND(_get_configuration_warnings);

    GDVIRTUAL_BIND(_show_in_add_tree);
    GDVIRTUAL_BIND(_can_add_children);

    GDVIRTUAL_BIND(_on_created);
    GDVIRTUAL_BIND(_on_added);
    GDVIRTUAL_BIND(_on_removed);
    GDVIRTUAL_BIND(_on_child_order_changed);

    GDVIRTUAL_BIND(_setup);
    GDVIRTUAL_BIND(_start);
    GDVIRTUAL_BIND(_update, "delta");
    GDVIRTUAL_BIND(_end);

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children", PROPERTY_HINT_TYPE_STRING, vformat("%d:EVBase", Variant::OBJECT), PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR), "_set_children", "_get_children");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR), "_set_enabled", "_is_enabled");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "edit_flags", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR), "_set_edit_flags", "_get_edit_flags");
}

// void EVBase::_unlink()
// {
//     auto prev_child = prev;
//     if (prev_child.is_valid())
//     {
//         prev_child->next = next;
//     }
//     if (next.is_valid())
//     {
//         next->prev = prev_child;
//     }
//     if (parent.is_valid())
//     {
//         parent->children.erase(this);
//         if (parent->first_child == this)
//         {
//             parent->first_child = next;
//         }
//         if (parent->last_child == this)
//         {
//             parent->last_child = prev;
//         }
//     }
// }

// void EVBase::_change_sequence(Ref<EventerSequence> p_sequence)
// {
//     if (p_sequence == sequence) { return; }

//     if (sequence->get_root() == Ref<EVCommand>(this))
//     {
//         sequence->set_root(nullptr);
//     }

//     Ref<EVBase> c = first_child;
//     while (c.is_valid())
//     {
//         c->_change_sequence(p_sequence);
//         c = c->next;
//     }



//     sequence = p_sequence;
// }

void EVBase::_on_children_reordered()
{
    for (int i = 0; i < children.size(); i++)
    {
        Ref<EVBase> child = children[i];
        child->child_index = i;
        child->on_child_order_changed();
    }
    emit_changed();
}

void EVBase::set_children(const TypedArray<Ref<EVBase>> &p_children)
{
    if (children.size() > 0)
    {
        for (int i = 0; i < children.size(); i++)
        {
            remove_child(children[i]);
        }
    }
    children.clear();
    
    int size = p_children.size();
    children.resize(size);
    for (int i = 0; i < size; i++)
    {
        Ref<EVBase> child = p_children[i];
        if (not child.is_valid()) { continue; }

        child->parent = this;
        children.set(i, child);
    }
    _on_children_reordered();
}

TypedArray<Ref<EVBase>> EVBase::get_children() const
{
    TypedArray<Ref<EVBase>> ret;
    int size = children.size();
    ret.resize(size);
    for (int i = 0; i < size; i++)
    {
        ret.set(i, children[i]);
    }
    return ret;
}

void EVBase::add_child(Ref<EVBase> p_child, EditFlags p_edit_flags)
{
    ERR_FAIL_COND_MSG(not can_add_children(), "Command doesn't support adding children");
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't add a null child");
    ERR_FAIL_COND_MSG(p_child.ptr() == this, "Can't parent something to itself");
    
    if (p_child->parent == this) { return; }

    if (p_child->parent.is_valid())
    {
        p_child->parent->remove_child(p_child);
    }

    p_child->set_edit_flags(p_edit_flags);
    p_child->parent = this;
    children.append(p_child);

    _on_children_reordered();
}

void EVBase::remove_child(Ref<EVBase> p_child)
{
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't remove a null child");
    ERR_FAIL_COND_MSG(not children.has(p_child), vformat("Child doesn't exist: %s", p_child));

    children.erase(p_child);
    p_child->parent = nullptr;

    _on_children_reordered();
}

void EVBase::move_child(Ref<EVBase> p_child, const int &p_idx)
{
    ERR_FAIL_NULL(p_child);
    ERR_FAIL_COND(p_child->parent != this);
    ERR_FAIL_INDEX(p_idx, get_child_count());

    if (p_child->child_index == p_idx) { return; }

    children.erase(p_child);
    children.insert(p_idx, p_child);

    _on_children_reordered();
}

bool EVBase::has_child(const Ref<EVBase> p_child) const
{
    return children.has(p_child);
}

Ref<EVBase> EVBase::get_child(int p_idx) const
{
    if (p_idx < 0)
    {
        p_idx += children.size();
    }
    ERR_FAIL_INDEX_V(p_idx, children.size(), nullptr);
    return children[p_idx];
}

Ref<EVBase> EVBase::get_next() const
{
    if (parent.is_valid())
    {
        return parent->get_child(child_index + 1);
    }
    return nullptr;
}

Ref<EVBase> EVBase::get_next_enabled() const
{
    Ref<EVBase> next_item = get_next();
    while (next_item.is_valid() and not next_item->is_enabled())
    {
        next_item = next_item->get_next();
    }
    return next_item;
}

Ref<EVBase> EVBase::get_next_in_tree() const
{
    if (get_child_count() > 0) // if this node has children, go to the first one
    {
        return get_child(0);
    }
    else if (get_parent().is_valid() and get_parent()->get_child_count() > child_index - 1) // if this has no children go to the next sibling
    {
        return get_next_enabled();
    }
    else // otherwise, try to step out or return nullptr if done
    {
        auto current = Ref<EVBase>(this);

        Ref<EVBase> next = nullptr;
        if (current->get_child_count() > 0)
        {
            next = current->get_child(current->child_index + 1);
        }

        while (current.is_valid() and not next.is_valid())
        {
            current = current->parent;
        }

        if (not current.is_valid())
        {
            return nullptr;
        }
        else
        {
            return current;
        }
    }
}

Ref<EVBase> EVBase::get_next_in_tree_enabled() const
{
    Ref<EVBase> next_item = get_next_in_tree();
    while (next_item.is_valid() and not next_item->is_enabled())
    {
        next_item = next_item->get_next_in_tree();
    }
    return next_item;
}

/* int EVBase::get_index() const
{
    int idx = 0;
    Ref<EVBase> current = Ref<EVBase>(this);
    while (current.is_valid())
    {
        current = current->prev;
        idx++;
    }
    return idx - 1;
} */

/* int EVBase::get_child_index() const
{
    int idx = 0;
    Ref<EVBase> current = Ref<EVBase>(this);
    while (current.is_valid() and current->parent == parent)
    {
        current = current->prev;
        idx++;
    }
    return idx - 1;
} */

String EVBase::get_command_name()
{
    String ret;

    if (not GDVIRTUAL_CALL(_get_command_name, ret))
    {
        Ref<Script> script = get_script();
        if (script.is_valid())
        {
            String path = script->get_path();
            ret = path.get_file().trim_suffix(path.get_extension()).rstrip(".").capitalize();
        }
        else
        {
            ret = get_class().trim_prefix("EV").capitalize();
        }
    }

    return ret;
}

String EVBase::get_command_text()
{
    String ret;
    if (not GDVIRTUAL_CALL(_get_command_text, ret))
    {
        return get_command_name();
    }
    return ret;
}

Ref<Texture2D> EVBase::get_command_icon()
{
    Ref<Texture2D> ret;
    if (not GDVIRTUAL_CALL(_get_command_icon, ret))
    {
        Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();
        const auto ei = EditorStringName(EditorIcons);

        if (theme->has_icon(get_class_name(), ei))
        {
            ret = theme->get_icon(get_class_name(), ei);
        }
        else
        {
            StringName class_name = ClassDB::get_parent_class(get_class_name());
            while (not theme->has_icon(class_name, ei) and not class_name.is_empty())
            {
                class_name = ClassDB::get_parent_class(class_name);
            }

            if (not class_name.is_empty())
            {
                ret = theme->get_icon(class_name, ei);
            }
            else
            {
                ret = theme->get_icon("EVBase", ei);
            }
        }
    }
    return ret;
}

String EVBase::get_command_suffix()
{
    String ret;
    GDVIRTUAL_CALL(_get_command_suffix, ret);
    return ret;
}

String EVBase::get_command_category()
{
    String ret;
    GDVIRTUAL_CALL(_get_command_category, ret);
    return ret;
}

Color EVBase::get_command_color()
{
    Color ret;
    if (not GDVIRTUAL_CALL(_get_command_color, ret))
    {
        ret = Color(1.0, 1.0, 1.0, 0.0);
    }
    return ret;
}

PackedStringArray EVBase::get_configuration_warnings()
{
    PackedStringArray ret;
    GDVIRTUAL_CALL(_get_configuration_warnings, ret);
    return ret;
}

bool EVBase::show_in_add_tree()
{
    bool ret = true;
    GDVIRTUAL_CALL(_show_in_add_tree, ret);
    return ret;
}

bool EVBase::can_add_children()
{
    bool ret = is_root();
    GDVIRTUAL_CALL(_can_add_children, ret);
    return ret;
}

void EVBase::on_created()
{
    GDVIRTUAL_CALL(_on_created);
}

void EVBase::on_added()
{
    GDVIRTUAL_CALL(_on_added);
}

void EVBase::on_removed()
{
    GDVIRTUAL_CALL(_on_removed);
}

void EVBase::on_child_order_changed()
{
    GDVIRTUAL_CALL(_on_child_order_changed);
}

void EVBase::setup()
{
    GDVIRTUAL_CALL(_setup);
}

void EVBase::start()
{
    GDVIRTUAL_CALL(_start);
}

EVResult EVBase::update(const float &p_delta)
{
    EVResult ret = EV_NEXT_IN_TREE;
    GDVIRTUAL_CALL(_update, p_delta, ret);
    return ret;
}

void EVBase::end()
{
    GDVIRTUAL_CALL(_end);
}


