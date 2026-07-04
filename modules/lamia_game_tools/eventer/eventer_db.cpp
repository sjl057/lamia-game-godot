#include "eventer_db.h"
#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/script_language.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "modules/lamia_game_tools/general/string_names.h"
#include "modules/lamia_game_tools/general/utility.h"

AHashMap<StringName, StringName> EventerDB::builtin_commands;
AHashMap<StringName, Ref<Script>> EventerDB::script_commands;

void EventerDB::_bind_methods()
{
    BIND_STA(D_METHOD("get_builtin_commands"), &EventerDB::get_builtin_commands);
    BIND_STA(D_METHOD("get_script_commands"), &EventerDB::get_script_commands);
}

Ref<EVBase> EventerDB::get_command_instance(const String &p_class_or_script)
{
    ERR_FAIL_COND_V(p_class_or_script.is_empty(), nullptr);

    Ref<EVBase> ret;

    if (EventerDB::is_builtin_console_command(p_class_or_script))
    {
        ret = ClassDB::instantiate(builtin_commands[p_class_or_script]);
    }
    else if (EventerDB::is_script_console_command(p_class_or_script))
    {
        Ref<Script> scr = script_commands[p_class_or_script];

        Variant inst = ClassDB::instantiate(scr->get_instance_base_type());
        Object *obj = inst;
        ERR_FAIL_NULL_V(obj, nullptr);

        if (not obj->is_class("EVBase"))
        {
            memdelete(obj);
            return nullptr;
        }

        ret.reference_ptr(Object::cast_to<EVBase>(obj));
        ret->set_script(scr);
    }
    return ret;
}

bool EventerDB::is_builtin_console_command(const String &p_class)
{
    return builtin_commands.has(p_class);
}

bool EventerDB::is_script_console_command(const String &p_script)
{
    return script_commands.has(p_script);
}

bool EventerDB::is_console_command(const String &p_class_or_script)
{
    return script_commands.has(p_class_or_script) or builtin_commands.has(p_class_or_script);
}

void EventerDB::scan_script_command_paths()
{
    PackedStringArray paths = GLOBAL_GET(LGTStringName(EventerGlobalCommandDirectories));
    if (not paths.is_empty())
    {
        for (const String &path : paths)
        {
            LGTUtility::get_singleton()->collect_scripts_from_dir(path, &script_commands);
        }
    }
}

PackedStringArray EventerDB::get_builtin_commands()
{
    PackedStringArray ret;
    for (const KeyValue<StringName, StringName> &KV : builtin_commands)
    {
        ret.append(KV.key);
    }
    return ret;
}

PackedStringArray EventerDB::get_script_commands()
{
    PackedStringArray ret;
    for (const KeyValue<StringName, Ref<Script>> &KV : script_commands)
    {
        ret.append(KV.key);
    }
    return ret;
}

PackedStringArray EventerDB::get_commands()
{
    PackedStringArray ret;
    ret.append_array(get_builtin_commands());
    ret.append_array(get_script_commands());
    return ret;
}