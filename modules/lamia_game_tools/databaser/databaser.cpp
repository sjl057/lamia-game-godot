#include "databaser.h"
#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/io/resource_loader.h"
#include "core/math/vector2i.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/script_language.h"
#include "core/string/print_string.h"
#include "core/string/string_name.h"
#include "core/variant/callable.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "database.h"
#include "database_select_dialog.h"
#include "editor/editor_interface.h"
#include "modules/lamia_game_tools/general/defs.h"

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
    BIND(D_METHOD("get_database"), &Databaser::get_database);
    BIND_STA(D_METHOD("get_database_path"), &Databaser::get_database_path);
    BIND_STA(D_METHOD("get_type_list"), &Databaser::get_type_list);
    BIND_STA(D_METHOD("get_type_count"), &Databaser::get_type_count);
    BIND_STA(D_METHOD("has_type", "type"), &Databaser::has_type);
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
    PackedStringArray ret;

    LocalVector<StringName> global_classes;
    ScriptServer::get_global_class_list(global_classes);

    StringName base_class = DatabaseResource::get_class_static();
    for (const StringName &E : global_classes)
    {
        StringName native_base = ScriptServer::get_global_class_native_base(E);
        if (native_base == base_class)
        {
            ret.append(E);
        }
    }

    return ret;
}

TypedArray<Dictionary> Databaser::get_type_dict()
{
    TypedArray<Dictionary> ret;
    LocalVector<StringName> global_classes;
    ScriptServer::get_global_class_list(global_classes);

    StringName base_class = DatabaseResource::get_class_static();
    for (const StringName &E : global_classes)
    {
        StringName native_base = ScriptServer::get_global_class_native_base(E);
        if (native_base == base_class)
        {
            Dictionary dict;
            dict.set("class", E);
            dict.set("path", ScriptServer::get_global_class_path(E));
            dict.set("language", ScriptServer::get_global_class_language(E));
            dict.set("base", ScriptServer::get_global_class_base(E));
            ret.append(dict);
        }
    }

    return ret;
}

bool Databaser::has_type(const StringName p_type)
{
    LocalVector<StringName> global_classes;
    ScriptServer::get_global_class_list(global_classes);

    StringName base_class = DatabaseResource::get_class_static();
    for (const StringName &E : global_classes)
    {
        if (E == p_type)
        {
            StringName native_base = ScriptServer::get_global_class_native_base(E);
            if (native_base == base_class)
            {
                return true;
            }
        }
    }

    return false;
}

int Databaser::get_type_count()
{
    unsigned int count = 0;

    LocalVector<StringName> global_classes;
    ScriptServer::get_global_class_list(global_classes);

    StringName base_class = DatabaseResource::get_class_static();
    for (const StringName &E : global_classes)
    {
        StringName native_base = ScriptServer::get_global_class_native_base(E);
        if (native_base == base_class)
        {
            count++;
        }
    }

    return count;
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
#ifdef DEBUG_ENABLED
    ERR_FAIL_COND(not database->get_groups().has(p_type));
    if (select_dialog and select_dialog->is_visible()) { return; }

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
#endif // DEBUG_ENABLED
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