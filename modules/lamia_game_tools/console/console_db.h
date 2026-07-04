#pragma once

#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/script_language.h"
#include "core/string/string_name.h"
#include "core/templates/a_hash_map.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/console/console_command.h"

class ConsoleDB : public Object
{
    GDCLASS(ConsoleDB, Object);

private:
    static AHashMap<StringName, StringName> builtin_commands;
    static AHashMap<StringName, Ref<Script>> script_commands;

    // static void collect_scripts_from_dir(String p_path, AHashMap<StringName, Ref<Script>> *r_ret);

protected:
    static void _bind_methods();

public:
    template<class T>
    static void register_command()
    {
        StringName class_name = T::get_class_static();
        ERR_FAIL_COND_MSG(builtin_commands.has(class_name), vformat("Command already registered: %s", class_name));
        ERR_FAIL_COND_MSG(not ClassDB::is_parent_class(class_name, "ConsoleCommand"), vformat("Command doesn't inherit from ConsoleCommand: %s", class_name));
        builtin_commands.insert(String(class_name).trim_prefix("ConsoleCommand").to_snake_case(), class_name);
    }

    static Ref<ConsoleCommand> get_command_instance(const String &p_class_or_script);
    static bool is_builtin_console_command(const String &p_class);
    static bool is_script_console_command(const String &p_script);
    static bool is_console_command(const String &p_class_or_script);
    static void scan_script_command_paths();

    static PackedStringArray get_builtin_commands();
    static PackedStringArray get_script_commands();
    static PackedStringArray get_commands();
};

#define CONSOLE_REGISTER_COMMAND(m_class)     \
if constexpr (GD_IS_CLASS_ENABLED(m_class))   \
{                                             \
    GDREGISTER_CLASS(m_class);                \
    ::ConsoleDB::register_command<m_class>(); \
};
;