#include "eventer_db.h"
#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/object/class_db.h"
#include "core/string/ustring.h"
#include "modules/lamia_game_tools/general/defs.h"

PackedStringArray EventerDB::builtin_commands;
PackedStringArray EventerDB::script_commands;

void EventerDB::_bind_methods()
{
    BIND_STA(D_METHOD("get_builtin_commands"), &EventerDB::get_builtin_commands);
    BIND_STA(D_METHOD("get_script_command_paths"), &EventerDB::get_script_command_paths);
}

void EventerDB::collect_scripts_from_dir(String p_path, PackedStringArray *r_ret)
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
        String full_path = vformat("%s/%s", p_path, path);
        if (dir->current_is_dir() && path != "." && path != ".." && path != "./")
        {
            collect_scripts_from_dir(full_path, r_ret);
        }
        else
        {
            if (path.get_extension() == "gd" and not r_ret->has(path))
            {
                r_ret->append(full_path);
            }
        }
        path = dir->get_next();
    }
    dir->list_dir_end();
}

void EventerDB::scan_script_command_paths()
{
    PackedStringArray paths = GLOBAL_GET("eventer/commands/command_directories");
    if (not paths.is_empty())
    {
        for (const String &path : paths)
        {
            collect_scripts_from_dir(path, &script_commands);
        }
    }
}

PackedStringArray EventerDB::get_builtin_commands()
{
    return builtin_commands;
}

PackedStringArray EventerDB::get_script_command_paths()
{
    return script_commands;
}
