#ifndef __DATABASE_H__
#define __DATABASE_H__

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"

class DatabaseResource : public Resource
{
    GDCLASS(DatabaseResource, Resource)

private:
    StringName data_id;
    String data_name;

    Ref<DatabaseResource> parent = nullptr;
    TypedArray<Ref<DatabaseResource>> children;
    // StringName path;

    void _ensure_child_ids_are_unique();

protected:
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
    TypedArray<Ref<DatabaseResource>> get_children() const { return children; }
    Ref<DatabaseResource> get_child(int p_idx) const;
    int get_child_count() const { return children.size(); }
    Ref<DatabaseResource> get_child_with_id(StringName p_id) const;
    bool has_child(Ref<DatabaseResource> p_child) const;
    bool has_child_with_id(StringName p_id) const;
    bool is_root() const { return not parent.is_valid(); }
    Ref<DatabaseResource> get_root();
/*     void set_database_path(StringName p_path) { path = p_path; }
    StringName get_database_path() const { return path; } */

    virtual Object* instantiate();
};

class Database : public Resource
{
    GDCLASS(Database, Resource)

private:
    Dictionary groups;
    // Dictionary dead_groups;
    PackedStringArray group_order;
    // PackedStringArray dead_group_order;

protected:
    static void _bind_methods();

public:
    bool has_group(StringName p_group) const { return groups.has(p_group); }
    bool has_data(StringName p_group, StringName p_path);
    Ref<DatabaseResource> get_data(StringName p_group, StringName p_path);
    void get_all_data(Ref<DatabaseResource> p_from, TypedArray<Ref<DatabaseResource>> &r_ret) const;
    TypedArray<Ref<DatabaseResource>> get_all_data(Ref<DatabaseResource> p_from) const;

    void set_groups(Dictionary p_groups) { print_line(p_groups); groups = p_groups; }
    Dictionary get_groups() const { return groups; }
/*     void set_dead_groups(Dictionary p_dead_groups) { dead_groups = p_dead_groups; }
    Dictionary get_dead_groups() const { return dead_groups; } */
    void set_group_order(PackedStringArray p_order) { group_order = p_order; }
    PackedStringArray get_group_order() const { return group_order; }
    // void set_dead_group_order(PackedStringArray p_order) { dead_group_order = p_order; }
    // PackedStringArray get_dead_group_order() const { return dead_group_order; }
    void refresh_groups();

    void recursive_save();
};

#endif // __DATABASE_H__