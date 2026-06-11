#include "ev.h"
#include "core/error/error_macros.h"
#include "core/math/color.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/script_language.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/general/defs.h"

void EVBase::_bind_methods()
{
    BIND(D_METHOD("_set_children", "children"), &EVBase::set_children);
    BIND(D_METHOD("_get_children"), &EVBase::get_children);
    BIND(D_METHOD("_set_enabled", "enabled"), &EVBase::set_enabled);
    BIND(D_METHOD("_is_enabled"), &EVBase::is_enabled);

    BIND(D_METHOD("_add_child", "child"), &EVBase::add_child);
    BIND(D_METHOD("_remove_child", "child"), &EVBase::remove_child);
    BIND(D_METHOD("_move_child", "child"), &EVBase::move_child);

    GDVIRTUAL_BIND(_get_command_text);
    GDVIRTUAL_BIND(_get_command_suffix);
    GDVIRTUAL_BIND(_get_command_category);
    GDVIRTUAL_BIND(_get_command_color);
    GDVIRTUAL_BIND(_get_configuration_warnings);

    GDVIRTUAL_BIND(_can_drop_on, "command");
    GDVIRTUAL_BIND(_can_drop_above, "command");
    GDVIRTUAL_BIND(_can_drop_below, "command");
    GDVIRTUAL_BIND(_can_move);

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
}

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
    children.clear();
    int size = p_children.size();
    children.resize(size);
    for (int i = 0; i < size; i++)
    {
        Ref<EVBase> child = p_children[i];
        if (child.is_valid())
        {
            Ref<EVBase> child_parent = child->get_parent();
            if (child_parent.is_valid() and child_parent != this)
            {
                child = child->duplicate();
            }
            children.set(i, child);
        }
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

void EVBase::add_child(Ref<EVBase> p_child)
{
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't add a null child");
    ERR_FAIL_COND_MSG(p_child->get_parent().is_valid(), "DatabaseResource has a parent, can't add");
    ERR_FAIL_COND_MSG(p_child.ptr() == this, "Can't parent something to itself");
    ERR_FAIL_COND_MSG(children.has(p_child), "Already has child");
    if (p_child->get_parent().is_valid())
    {
        if (p_child->get_parent() != this)
        {
            p_child->get_parent()->remove_child(p_child);
        }
    }

    p_child->set_parent(this);
    children.append(p_child);
    _on_children_reordered();
}


void EVBase::remove_child(Ref<EVBase> p_child)
{
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't remove a null child");
    ERR_FAIL_COND_MSG(not children.has(p_child), vformat("Child doesn't exist: %s", p_child));
    p_child->set_parent(nullptr);
    p_child->set_child_index(-1);
    children.erase(p_child);
    _on_children_reordered();
}

void EVBase::move_child(Ref<EVBase> p_child, const int &p_to)
{
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't move a null child");
    ERR_FAIL_COND_MSG(not children.has(p_child), vformat("Child doesn't exist: %s", p_child));

    if (p_child->get_parent() == this)
    {
        children.erase(p_child);
        children.insert(p_to, p_child);
        p_child->set_child_index(p_to);
        _on_children_reordered();
    }
}

bool EVBase::has_child(const Ref<EVBase> p_child) const
{
    return children.has(p_child);
}

Ref<EVBase> EVBase::get_root() const
{
    const EVBase *p = this;
    while (not p->is_root())
    {
        p = p->parent;
    }
    return Ref<EVBase>(p);
}

Ref<EVBase> EVBase::get_child(int p_idx) const
{
    if (p_idx < 0)
    {
        p_idx = children.size() - 1;
    }
    ERR_FAIL_INDEX_V(p_idx, (int)children.size(), nullptr);
    return children[p_idx];
}

/* Ref<EVBase> EVBase::get_next_child(bool p_must_be_enabled) const
{
    
} */

Ref<EVBase> EVBase::get_next_sibling(bool p_must_be_enabled) const
{
    ERR_FAIL_COND_V(not parent, nullptr);
    if (p_must_be_enabled)
    {
        Ref<EVBase> next_sibling = get_next_sibling(false);
        while (next_sibling.is_valid() and not next_sibling->is_enabled())
        {
            next_sibling = get_next_sibling(false);
            if (not next_sibling.is_valid())
            {
                return nullptr;
            }
            return next_sibling;
        }
    }
    else
    {
        return parent->get_child(child_index + 1);
    }
}

Ref<EVBase> EVBase::get_next_in_tree(bool p_must_be_enabled) const
{
    ERR_FAIL_COND_V(not parent, nullptr);
    const EVBase *next = this;

    if (next->get_child_count() > 0)
    {
        // this doesn't seem right
        next = next->get_child(0).ptr();
    }
    else if (parent and parent->get_child(child_index + 1).is_valid())
    {
        next = get_next_sibling().ptr();
    }
    else
    {
        Ref<EVBase> root = get_root();
        while (next and not next->get_child(next->child_index + 1).is_valid())
        {
            if (next->parent == root.ptr())
            {
                break;
            }
            next = next->parent;
        }

        if (not next)
        {
            next = nullptr;
        }
        else
        {
            next = next->get_next_sibling().ptr();
        }
    }

    return Ref<EVBase>(next);
}

String EVBase::get_command_name() const
{
    Ref<Script> script = get_script();
    if (script.is_valid())
    {
        String path = script->get_path();
        return path.get_file().trim_suffix(path.get_extension()).rstrip(".").capitalize();
    }
    else
    {
        return get_class().capitalize();
    }
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

bool EVBase::can_drop_on(Ref<EVBase> p_command)
{
    bool ret;
    if (not GDVIRTUAL_CALL(_can_drop_on, p_command, ret))
    {
        ret = true;
    }
    return ret;
}

bool EVBase::can_drop_above(Ref<EVBase> p_command)
{
    bool ret;
    if (not GDVIRTUAL_CALL(_can_drop_above, p_command, ret))
    {
        ret = true;
    }
    return ret;
}

bool EVBase::can_drop_below(Ref<EVBase> p_command)
{
    bool ret;
    if (not GDVIRTUAL_CALL(_can_drop_below, p_command, ret))
    {
        ret = true;
    }
    return ret;
}

bool EVBase::can_move()
{
    bool ret;
    if (not GDVIRTUAL_CALL(_can_move, ret))
    {
        ret = true;
    }
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

EVResult EVBase::update(float p_delta)
{
    EVResult ret;
    GDVIRTUAL_CALL(_update, p_delta, ret);
    return ret;
}

void EVBase::end()
{
    GDVIRTUAL_CALL(_end);
}


