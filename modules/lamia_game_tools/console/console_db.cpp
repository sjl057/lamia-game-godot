#include "console_db.h"
#include "core/config/project_settings.h"
#include "core/io/dir_access.h"
#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "core/object/script_language.h"
#include "core/string/print_string.h"
#include "core/string/string_name.h"
#include "core/templates/pair.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/console/console_command.h"
#include "modules/lamia_game_tools/general/defs.h"

AHashMap<StringName, StringName> ConsoleDB::builtin_commands;
AHashMap<StringName, Ref<Script>> ConsoleDB::script_commands;

void ConsoleDB::_bind_methods()
{
    // BIND_STA(D_METHOD("get_builtin_commands"), &ConsoleDB::get_builtin_commands);
    // BIND_STA(D_METHOD("get_script_command_paths"), &ConsoleDB::get_script_command_paths);
}

void ConsoleDB::collect_scripts_from_dir(String p_path, AHashMap<StringName, Ref<Script>> *r_ret)
{
    if (p_path.is_empty()) { return; }

    Error err;
    Ref<DirAccess> dir = DirAccess::open(p_path, &err);

    if (err != OK)
    {
        ERR_FAIL_MSG(vformat("Couldn't scan path: %s", p_path));
        return;
    }

    dir->list_dir_begin();
    String path = dir->get_next();
    while (not path.is_empty())
    {
        String full_path = p_path.path_join(path);
        if (dir->current_is_dir() && path != "." && path != ".." && path != "./")
        {
            collect_scripts_from_dir(full_path, r_ret);
        }
        else
        {
            String script_name = path.get_file().get_basename().trim_prefix("res://");

            if (r_ret->has(path))
            {
                int i = 0;
                for (const KeyValue<StringName, Ref<Script>> &KV : *r_ret)
                {
                    String included_script_name = KV.key;
                    included_script_name = included_script_name.get_file().get_basename().trim_prefix("res://");
                    if (script_name.contains(included_script_name))
                    {
                        i++;
                    }
                }
                script_name += vformat("_%d", i);
            }

            if (path.get_extension() == "gd" and not builtin_commands.has(script_name))
            {
                Ref<Script> scr = ResourceLoader::load(p_path.path_join(path));
                if (scr.is_valid())
                {
                    StringName base_type = scr->get_instance_base_type();
                    if (base_type == StringName())
                    {
                        scr->reload(true);
                        base_type = scr->get_instance_base_type();
                    }
                    if (base_type != StringName())
                    {
                        r_ret->insert(script_name, scr);
                    }
                }
            }
        }
        path = dir->get_next();
    }
    dir->list_dir_end();
}

Ref<ConsoleCommand> ConsoleDB::get_command_instance(const String &p_class_or_script)
{
    ERR_FAIL_COND_V(p_class_or_script.is_empty(), nullptr);

    Ref<ConsoleCommand> ret;

    if (ConsoleDB::is_builtin_console_command(p_class_or_script))
    {
        ret = ClassDB::instantiate(builtin_commands[p_class_or_script]);
    }
    else if (ConsoleDB::is_script_console_command(p_class_or_script))
    {
        Ref<Script> scr = script_commands[p_class_or_script];

        Variant inst = ClassDB::instantiate(scr->get_instance_base_type());
        Object *obj = inst;
        ERR_FAIL_NULL_V(obj, nullptr);

        if (not obj->is_class("ConsoleCommand"))
        {
            memdelete(obj);
            return nullptr;
        }

        ret.reference_ptr(Object::cast_to<ConsoleCommand>(obj));
        ret->set_script(scr);
    }
    return ret;
}

bool ConsoleDB::is_builtin_console_command(const String &p_class)
{
    return builtin_commands.has(p_class);
}

bool ConsoleDB::is_script_console_command(const String &p_script)
{
    return script_commands.has(p_script);
}

bool ConsoleDB::is_console_command(const String &p_class_or_script)
{
    return script_commands.has(p_class_or_script) or builtin_commands.has(p_class_or_script);
}

void ConsoleDB::scan_script_command_paths()
{
    PackedStringArray paths = GLOBAL_GET("console/commands/command_directories");
    if (not paths.is_empty())
    {
        for (const String &path : paths)
        {
            collect_scripts_from_dir(path, &script_commands);
        }
    }
}

PackedStringArray ConsoleDB::get_builtin_commands()
{
    PackedStringArray ret;
    for (const KeyValue<StringName, StringName> &KV : builtin_commands)
    {
        ret.append(KV.key);
    }
    return ret;
}

PackedStringArray ConsoleDB::get_script_commands()
{
    PackedStringArray ret;
    for (const KeyValue<StringName, Ref<Script>> &KV : script_commands)
    {
        ret.append(KV.key);
    }
    return ret;
}

PackedStringArray ConsoleDB::get_commands()
{
    PackedStringArray ret;
    ret.append_array(get_builtin_commands());
    ret.append_array(get_script_commands());
    return ret;
}