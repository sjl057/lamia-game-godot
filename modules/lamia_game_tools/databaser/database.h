#pragma once

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/templates/vector.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"

class DatabaseResource : public Resource
{
    GDCLASS(DatabaseResource, Resource)

    friend class Database;
    friend class DatabaseTree;

private:
    StringName data_id;
    String data_name;

    Ref<DatabaseResource> parent = nullptr;
    TypedArray<Ref<DatabaseResource>> children;
    StringName path;

    void _ensure_child_ids_are_unique();

protected:
    void recursive_save();
    GDVIRTUAL0R_REQUIRED(Object*, _instantiate)
    static void _bind_methods();

public:
    void set_data_id(const StringName p_data_id) { data_id = p_data_id; emit_changed(); }
    StringName get_data_id() const { return data_id; }
    void set_data_name(const String p_data_name) { data_name = p_data_name; emit_changed(); }
    String get_data_name() const { return data_name; };

    void set_data_id_no_signal(const StringName p_data_id) { data_id = p_data_id; }

    void get_all_data(Ref<DatabaseResource> p_from, TypedArray<Ref<DatabaseResource>> &r_ret) const;

    void reparent(Ref<DatabaseResource> p_parent);
    void set_parent(Ref<DatabaseResource> p_parent) { parent = p_parent; }
    Ref<DatabaseResource> get_parent() const { return parent; }
    bool can_add_child(Ref<DatabaseResource> p_child) const;
    void add_child(Ref<DatabaseResource> p_child);
    void remove_child(Ref<DatabaseResource> p_child);
    void move_child(Ref<DatabaseResource> p_child, int p_to_index);
    void set_children(const TypedArray<Ref<DatabaseResource>> &p_children);
    const TypedArray<Ref<DatabaseResource>> &get_children() const { return children; }
    Ref<DatabaseResource> get_child(int p_idx) const;
    int get_child_count() const { return children.size(); }
    Ref<DatabaseResource> get_child_with_id(const StringName &p_id) const;
    bool has_child(Ref<DatabaseResource> p_child) const;
    bool has_child_with_id(const StringName &p_id) const;
    bool is_root() const { return not parent.is_valid(); }
    Ref<DatabaseResource> get_root();
    void set_database_path(StringName p_path) { path = p_path; }
    StringName get_database_path() const { return path; }

    virtual Object* instantiate();
};

class Database : public Resource
{
    GDCLASS(Database, Resource)
    
private:
    AHashMap<StringName, Ref<DatabaseResource>> groups;
    Vector<StringName> group_order;

protected:
    static void _bind_methods();

public:
    void set_group(const StringName &p_name, Ref<DatabaseResource> p_group);
    void remove_group(const StringName &p_name);
    bool has_group(StringName p_group) const { return groups.has(p_group); }
    bool has_data(StringName p_group, StringName p_path);
    Ref<DatabaseResource> get_data(StringName p_group, StringName p_path);
    void get_all_data(Ref<DatabaseResource> p_from, TypedArray<Ref<DatabaseResource>> &r_ret) const;
    TypedArray<Ref<DatabaseResource>> get_all_data(Ref<DatabaseResource> p_from) const;

    void set_groups(const Dictionary &p_groups);
    Dictionary get_groups() const;
    void set_group_order(const PackedStringArray &p_order);
    PackedStringArray get_group_order() const;

    void refresh_groups();
    void recursive_save();
};