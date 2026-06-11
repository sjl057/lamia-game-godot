#pragma once

#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/templates/vector.h"

class EventerDB : public Object
{
    GDCLASS(EventerDB, Object)

private:
    static PackedStringArray builtin_commands;
    static PackedStringArray script_commands;

    static void collect_scripts_from_dir(String p_path, PackedStringArray *r_ret);

protected:
    static void _bind_methods();

public:
    template<class T>
    static void register_command()
    {
        if (builtin_commands.has(T::get_class_static()))
        {
            ERR_PRINT(vformat("Command already registered: %s", T::get_class_static()));
            return;
        }
        builtin_commands.push_back(T::get_class_static());
    }

    static void scan_script_command_paths();

    static PackedStringArray get_builtin_commands();
    static PackedStringArray get_script_command_paths();
};

#define EVREGISTER_COMMAND(m_class)           \
if constexpr (GD_IS_CLASS_ENABLED(m_class))   \
{                                             \
    GDREGISTER_CLASS(m_class);                \
    ::EventerDB::register_command<m_class>(); \
};
;