#include "databaser.h"
#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/io/resource_loader.h"
#include "core/math/vector2i.h"
#include "core/object/object.h"
#include "core/string/print_string.h"
#include "core/variant/callable.h"
#include "core/variant/variant.h"
#include "database.h"
#include "database_select_dialog.h"
#include "editor/editor_interface.h"

Databaser *Databaser::singleton = nullptr;

Databaser::Databaser()
{
    singleton = this;
    if (get_database_path().is_empty())
    {
        WARN_PRINT_ONCE("No database set in project settings");
    }

}

Databaser::~Databaser()
{
    singleton = nullptr;

}

void Databaser::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_database"), &Databaser::get_database);
    ClassDB::bind_static_method("Databaser", D_METHOD("get_database_path"), &Databaser::get_database_path);
    ClassDB::bind_static_method("Databaser", D_METHOD("get_type_list"), &Databaser::get_type_list);
    ClassDB::bind_static_method("Databaser", D_METHOD("get_type_count"), &Databaser::get_type_count);
    ClassDB::bind_static_method("Databaser", D_METHOD("has_type", "type"), &Databaser::has_type);
    ClassDB::bind_static_method("Databaser", D_METHOD("resource_path_to_id", "path"), &Databaser::resource_path_to_id);
}

Databaser *Databaser::get_singleton()
{
    return singleton;
}

String Databaser::get_database_path()
{
    return GLOBAL_GET("databaser/config/database");
}

PackedStringArray Databaser::get_type_list()
{
    TypedArray<Dictionary> global_class_list = ProjectSettings::get_singleton()->get_global_class_list();
    AHashMap<StringName, Dictionary> class_map;
    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary class_dict = global_class_list[i];
        class_map.insert(class_dict["class"], class_dict);
    }

    StringName base_class = DatabaseResource::get_class_static();
    PackedStringArray class_list;
    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary this_class = global_class_list[i];
        StringName base = this_class["base"];
        while (class_map.has(base))
        {
            base = class_map[base]["base"];
        }

        if (base == base_class)
        {
            class_list.push_back(this_class["class"]);
        }
    }

    return class_list;
}

TypedArray<Dictionary> Databaser::get_type_dict()
{
    // this doesn't work that well
//     ERROR: FATAL: Index p_index = 1 is out of bounds (size() = 1).
//    at: get (./core/templates/cowdata.h:193)

    TypedArray<Dictionary> global_class_list = ProjectSettings::get_singleton()->get_global_class_list();
    AHashMap<StringName, Dictionary> class_map;
    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary class_dict = global_class_list[i];
        class_map.insert(class_dict["class"], class_dict);
    }

    StringName base_class = DatabaseResource::get_class_static();
    LocalVector<int> to_remove;
    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary this_class = global_class_list[i];
        StringName base = this_class["base"];
        while (class_map.has(base))
        {
            base = class_map[base]["base"];
        }

        if (base != base_class)
        {
            to_remove.push_back(i);
        }
    }

    for (const int i : to_remove)
    {
        global_class_list.remove_at(i);
    }

    return global_class_list;
}

bool Databaser::has_type(const StringName p_type)
{
    TypedArray<Dictionary> global_class_list = ProjectSettings::get_singleton()->get_global_class_list();
    AHashMap<StringName, Dictionary> class_map;
    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary class_dict = global_class_list[i];
        class_map.insert(class_dict["class"], class_dict);
    }

    StringName base_class = DatabaseResource::get_class_static();

    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary this_class = global_class_list[i];
        if ((StringName)this_class["class"] == p_type)
        {
            StringName base = this_class["base"];
            while (class_map.has(base))
            {
                base = class_map[base]["base"];
            }
                
            if (base == base_class)
            {
                return true;
            }
        }
    }

    return false;
}

int Databaser::get_type_count()
{
    TypedArray<Dictionary> global_class_list = ProjectSettings::get_singleton()->get_global_class_list();
    AHashMap<StringName, Dictionary> class_map;
    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary class_dict = global_class_list[i];
        class_map.insert(class_dict["class"], class_dict);
    }
    StringName base_class = DatabaseResource::get_class_static();

    int count = 0;
    for (int i = 0; i < global_class_list.size(); i++)
    {
        Dictionary this_class = global_class_list[i];
        StringName base = this_class["base"];
        while (class_map.has(base))
        {
            base = class_map[base]["base"];
        }
            
        if (base == base_class)
        {
            count++;
        }
    }

    return count;
}

String Databaser::resource_path_to_id(String p_path)
{
    if (p_path.contains("::"))
    {
        String file_path = p_path.get_slice("::", 0);
        return vformat("%s::%s", file_path.get_file().trim_suffix(file_path.get_extension().rstrip(".")), p_path.get_slice("::", 1));
    }
    else
    {
        return p_path.get_file().trim_suffix(p_path.get_extension()).rstrip(".");
    }
}

Ref<Database> Databaser::get_database()
{
    if (get_database_path().is_empty())
    {
        WARN_PRINT_ONCE("No Database specified");
        return nullptr;
    }

    if (not database.is_valid())
    {
        database = ResourceLoader::load(get_database_path());
    }
    return database;
}

void Databaser::popup_database_id_select(const Callable &p_callback, const StringName &p_type, const StringName p_default_path)
{
#ifdef TOOLS_ENABLED
    ERR_FAIL_COND(not database->get_groups().has(p_type));

    if (not select_dialog)
    {
        select_dialog = memnew(DatabaseSelectDialog);
        select_dialog->set_unparent_when_invisible(true);
    }

    const Callable callback = callable_mp(this, &Databaser::_on_data_id_selected);
    select_dialog->connect(SNAME("selected"), callback.bind(p_callback), CONNECT_DEFERRED);
    select_dialog->setup(get_database()->get_groups()[p_type], p_default_path, "");

    if (Engine::get_singleton()->is_editor_hint())
    {
        EditorInterface::get_singleton()->popup_dialog_centered_clamped(select_dialog, Size2i(700, 800));
    }
    else
    {
        
    }
#endif // TOOLS_ENABLED
}

void Databaser::_on_data_id_selected(const StringName &p_id, Ref<DatabaseResource> p_data, const Callable &p_callback)
{
    const Callable callback = callable_mp(this, &Databaser::_on_data_id_selected);

    select_dialog->disconnect(SNAME("selected"), callback);

    _call_data_id_callback(p_callback, p_id, p_data);
}

void Databaser::_call_data_id_callback(const Callable &p_callback, const Variant &p_id, Ref<DatabaseResource> p_data)
{
    Callable::CallError ce;
    Variant ret;

    const Variant data = p_data;

    const Variant *args[2] = { &p_id, &data };
    p_callback.callp(args, 2, ret, ce);
    if (ce.error != Callable::CallError::CALL_OK)
    {
		ERR_PRINT(vformat("Error calling callback: %s", Variant::get_callable_error_text(p_callback, args, 1, ce)));
	}
}