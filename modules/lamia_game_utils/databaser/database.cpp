#include "database.h"
#include "core/io/resource_saver.h"
#include "core/object/script_language.h"

// #ifdef TOOLS_ENABLED
// #include "godot_cpp/classes/editor_interface.hpp"
// #include "godot_cpp/classes/editor_file_system.hpp"
// #endif

void DatabaseResource::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_data_id", "id"), &DatabaseResource::set_data_id);
    ClassDB::bind_method(D_METHOD("get_data_id"), &DatabaseResource::get_data_id);
    ClassDB::bind_method(D_METHOD("set_data_name", "name"), &DatabaseResource::set_data_name);
    ClassDB::bind_method(D_METHOD("get_data_name"), &DatabaseResource::get_data_name);
    // ClassDB::bind_method(D_METHOD("get_database_path"), &DatabaseResource::get_database_path);
    // ClassDB::bind_method(D_METHOD("reparent", "parent"), &DatabaseResource::reparent);
    ClassDB::bind_method(D_METHOD("set_parent", "parent"), &DatabaseResource::set_parent);
    ClassDB::bind_method(D_METHOD("get_parent"), &DatabaseResource::get_parent);
    ClassDB::bind_method(D_METHOD("set_children"), &DatabaseResource::set_children);
    ClassDB::bind_method(D_METHOD("get_children"), &DatabaseResource::get_children);
    ClassDB::bind_method(D_METHOD("instantiate"), &DatabaseResource::instantiate);
    GDVIRTUAL_BIND(_instantiate);

    ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "id", PROPERTY_HINT_NONE, ""), "set_data_id", "get_data_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "name"), "set_data_name", "get_data_name");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "children", PROPERTY_HINT_TYPE_STRING, vformat("%d:DatabaseResource", PROPERTY_HINT_RESOURCE_TYPE), PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR), "set_children", "get_children");
}

void DatabaseResource::_ensure_child_ids_are_unique()
{
    for (int i = 0; i < get_child_count(); i++)
    {
        Ref<DatabaseResource> child = children[i];

        StringName new_id = child->get_data_id();
        if (new_id.is_empty())
        {
            String path = child->get_path();
            if (not path.is_empty() and not path.contains("::"))
            {
                new_id = path.get_file().trim_suffix(path.get_extension()).rstrip(".");
            }
            else
            {
                Ref<Script> script = Object::cast_to<Script>(child->get_script());
                new_id = script->get_global_name();
            }
        }

        // this part's broken, it activates when it shouldn't
/*         int instances = 0;
        for (int i = 0; i < get_child_count(); i++)
        {
            Ref<DatabaseResource> child = children[i];
            if (child->get_data_id().contains(new_id))
                instances++;
        }

        if (instances > 0)
            new_id = new_id + itos(instances); */

        child->set_data_id_no_signal(new_id);
    }
}

void DatabaseResource::get_all_data(Ref<DatabaseResource> p_from, TypedArray<Ref<DatabaseResource>> &r_ret) const
{
    if (not p_from.is_valid())
    {
        p_from = this;
    }
    r_ret.append_array(get_children());
    for (int i = 0; i < get_child_count(); i++)
    {
        get_all_data(get_child(i), r_ret);
    }
}

void DatabaseResource::reparent(Ref<DatabaseResource> p_parent)
{
    if (parent.is_valid())
    {
        parent->remove_child(this);
    }
    p_parent->add_child(this);
}

bool DatabaseResource::can_add_child(Ref<DatabaseResource> p_child) const
{
    Ref<Script> this_script = Object::cast_to<Script>(this->get_script());
    Ref<Script> child_script = Object::cast_to<Script>(p_child->get_script());
    if (not this_script.is_valid() or not child_script.is_valid())
    {
        return false;
    }
    return p_child.is_valid() and p_child.ptr() != this and not children.has(p_child) and (this_script->get_global_name() == child_script->get_global_name());
}

void DatabaseResource::add_child(Ref<DatabaseResource> p_child)
{
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't add a null child");
    ERR_FAIL_COND_MSG(p_child->get_parent().is_valid(), "DatabaseResource has a parent, can't add");
    ERR_FAIL_COND_MSG(p_child.ptr() == this, "Can't parent something to itself");
    ERR_FAIL_COND_MSG(children.has(p_child), "Already has child");
    p_child->set_parent(this);
    p_child->connect("changed", callable_mp(Object::cast_to<Resource>(this), &Resource::emit_changed));
    p_child->connect("changed", callable_mp(this, &DatabaseResource::_ensure_child_ids_are_unique));
    children.append(p_child);
    _ensure_child_ids_are_unique();
}

void DatabaseResource::remove_child(Ref<DatabaseResource> p_child)
{
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't remove a null child");
    ERR_FAIL_COND_MSG(not children.has(p_child), vformat("Child doesn't exist: %s", p_child));
    p_child->disconnect("changed", callable_mp(Object::cast_to<Resource>(this), &Resource::emit_changed));
    p_child->disconnect("changed", callable_mp(this, &DatabaseResource::_ensure_child_ids_are_unique));
    p_child->set_parent(nullptr);
    ResourceSaver::save(p_child);
    children.erase(p_child);
}

void DatabaseResource::move_child(Ref<DatabaseResource> p_child, int p_to_index)
{
    ERR_FAIL_COND_MSG(not p_child.is_valid(), "Can't move a null child");
    ERR_FAIL_COND_MSG(not children.has(p_child), vformat("Child doesn't exist: %s", p_child));
    children.erase(p_child);
    children.insert(p_to_index, p_child);
}

void DatabaseResource::set_children(const TypedArray<Ref<DatabaseResource>> &p_children)
{
    if (not children.is_empty())
    {
        for (int i = 0; i < children.size(); i++)
        {
            remove_child(children[i]);
        }
    }
    children.clear();

    for (int i = 0; i < p_children.size(); i++)
    {
        add_child(p_children[i]);
    }
}

Ref<DatabaseResource> DatabaseResource::get_child(int p_idx) const
{
    if (p_idx < 0)
    {
        p_idx += children.size();
    }
    ERR_FAIL_INDEX_V(p_idx, (int)children.size(), nullptr);
    return children[p_idx];
}

Ref<DatabaseResource> DatabaseResource::get_child_with_id(StringName p_id) const
{
    for (int i = 0; i < children.size(); i++)
    {
        Ref<DatabaseResource> child = children[i];
        if (child->get_data_id() == p_id)
        {
            return child;
        }
    }
    return nullptr;
}

bool DatabaseResource::has_child(Ref<DatabaseResource> p_child) const
{
    return children.has(p_child);
}

bool DatabaseResource::has_child_with_id(StringName p_id) const
{
    for (int i = 0; i < children.size(); i++)
    {
        Ref<DatabaseResource> child = children[i];
        if (child->get_data_id() == p_id)
        {
            return true;
        }
    }
    return false;
}

Ref<DatabaseResource> DatabaseResource::get_root()
{
    Ref<DatabaseResource> next = this;
    while (next.is_valid())
    {
        next = next->get_parent();
    }
    return next;
}

Object *DatabaseResource::instantiate()
{
    Object* ret = nullptr;
    GDVIRTUAL_CALL(_instantiate, ret);
    return ret;
}



bool Database::has_data(StringName p_group, StringName p_path)
{
    ERR_FAIL_COND_V(not has_group(p_group), false);

    Vector<String> split = ((String)p_path).split("/", false);

    Ref<DatabaseResource> current = groups[p_group];
    for (int i = 0; i < split.size(); i++)
    {
        current = current->get_child_with_id((StringName)split[i]);
        if (current.is_valid())
        {
            if (i < split.size() - 1) { continue; }
            else { return true; }
        }
        else
        {
            return false;
        }
    }

    return false;
}

Ref<DatabaseResource> Database::get_data(StringName p_group, StringName p_path)
{
    ERR_FAIL_COND_V(not has_group(p_group), nullptr);

    Vector<String> split = ((String)p_path).split("/", false);

    Ref<DatabaseResource> current = groups[p_group];
    for (int i = 0; i < split.size(); i++)
    {
        current = current->get_child_with_id((StringName)split[i]);
        if (current.is_valid())
        {
            if (i < split.size() - 1) { continue; }
            else { return current; }
        }
        else
        {
            return nullptr;
        }
    }

    return nullptr;
}

void Database::get_all_data(Ref<DatabaseResource> p_from, TypedArray<Ref<DatabaseResource>> &r_ret) const
{
    // sometimes this gets triggered which fucks up the database file, look into it
    ERR_FAIL_NULL(p_from);
    r_ret.append_array(p_from->get_children());
    for (int i = 0; i < p_from->get_child_count(); i++)
    {
        get_all_data(p_from->get_child(i), r_ret);
    }
}

TypedArray<Ref<DatabaseResource>> Database::get_all_data(Ref<DatabaseResource> p_from) const
{
    TypedArray<Ref<DatabaseResource>> array;
    get_all_data(p_from, array);
    return array;
}


void Database::recursive_save()
{
    if (get_path().is_empty()) { return; }

    ResourceSaver::save(this);

    for (int i = 0; i < get_groups().size(); i++)
    {
        Ref<DatabaseResource> group = get_groups()[get_groups().keys()[i]]; 

        TypedArray<Ref<DatabaseResource>> all_data = get_all_data(group);
        for (int j = 0; j < all_data.size(); j++)
        {
            ResourceSaver::save(all_data[j]);
        }
    }

}

void Database::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_set_groups", "groups"), &Database::set_groups);
    ClassDB::bind_method(D_METHOD("_get_groups"), &Database::get_groups);
    ClassDB::bind_method(D_METHOD("_set_group_order", "order"), &Database::set_group_order);
    ClassDB::bind_method(D_METHOD("_get_group_order"), &Database::get_group_order);

    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "groups", PROPERTY_HINT_TYPE_STRING, vformat("%d:;%d:DatabaseGroup", Variant::STRING_NAME, PROPERTY_HINT_RESOURCE_TYPE), PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR), "_set_groups", "_get_groups");
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "group_order", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR), "_set_group_order", "_get_group_order");
}