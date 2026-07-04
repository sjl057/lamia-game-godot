#pragma once

#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/script_language.h"
#include "modules/lamia_game_tools/eventer/sequence.h"

class EventerDB : public Object
{
    GDCLASS(EventerDB, Object)

private:
    static AHashMap<StringName, StringName> builtin_commands;
    static AHashMap<StringName, Ref<Script>> script_commands;

protected:
    static void _bind_methods();

public:
    template<class T>
    static void register_command()
    {
        StringName class_name = T::get_class_static();
        ERR_FAIL_COND_MSG(builtin_commands.has(class_name), vformat("Command already registered: %s", class_name));
        ERR_FAIL_COND_MSG(not ClassDB::is_parent_class(class_name, "EVBase"), vformat("Command doesn't inherit from EVBase: %s", class_name));
        builtin_commands.insert(String(class_name).trim_prefix("EV"), class_name);
    }

    static Ref<EVBase> get_command_instance(const String &p_class_or_script);
    static bool is_builtin_console_command(const String &p_class);
    static bool is_script_console_command(const String &p_script);
    static bool is_console_command(const String &p_class_or_script);
    static void scan_script_command_paths();

    static PackedStringArray get_builtin_commands();
    static PackedStringArray get_script_commands();
    static PackedStringArray get_commands();
};

#define EVREGISTER_COMMAND(m_class)           \
if constexpr (GD_IS_CLASS_ENABLED(m_class))   \
{                                             \
    GDREGISTER_CLASS(m_class);                \
    ::EventerDB::register_command<m_class>(); \
};
;