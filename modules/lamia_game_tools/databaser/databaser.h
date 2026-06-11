#pragma once

#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/variant/callable.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "database.h"
#include "database_select_dialog.h"

class Databaser : public Object
{
    GDCLASS(Databaser, Object)

private:
    DatabaseSelectDialog *select_dialog = nullptr;

    void _on_data_id_selected(const StringName &p_id, Ref<DatabaseResource> p_data, const Callable &p_callback);
    void _call_data_id_callback(const Callable &p_callback, const Variant &p_id, Ref<DatabaseResource> p_data);

protected:
    static Databaser *singleton;
    Ref<Database> database;
    static void _bind_methods();

public:
    static Databaser *get_singleton();
    static String get_database_path();
    static PackedStringArray get_type_list();
    static TypedArray<Dictionary> get_type_dict();
    static int get_type_count();
    static bool has_type(const StringName p_type);
    Ref<Database> get_database();

    void popup_database_id_select(const Callable &p_callback, const StringName &p_type, const StringName p_default_path);

    Databaser();
    ~Databaser();
};