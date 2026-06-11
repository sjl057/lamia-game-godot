#include "eventer.h"
#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/object/class_db.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/general/defs.h"

// Eventer *Eventer::singleton = nullptr;

void Eventer::_bind_methods()
{
}

/* Eventer::Eventer()
{
    singleton = this;
}

Eventer::~Eventer()
{
    singleton = nullptr;
}

Eventer* Eventer::get_singleton()
{
    return singleton;
} */

void Eventer::collect_scripts_from_path(String p_path, PackedStringArray &r_ret)
{
    Error err;
    Ref<DirAccess> dir = DirAccess::open(p_path, &err);

    if (err != OK) { return; }

    dir->list_dir_begin();
    String path = dir->get_next();

    while (not path.is_empty())
    {
        String full_path = vformat("%s/%s", p_path, path);
        if (dir->current_is_dir() && path != "." && path != ".." && path != "./")
        {
            collect_scripts_from_path(full_path, r_ret);
        }
        else
        {
            if (path.get_extension() == "gd")
            {
                r_ret.append(full_path);
            }
        }
        path = dir->get_next();
    }
    dir->list_dir_end();
}

PackedStringArray Eventer::get_command_script_paths()
{
    PackedStringArray ret;
    PackedStringArray paths = GLOBAL_GET("eventer/commands/command_directories");
    if (not paths.is_empty())
    {
        for (const String &path : paths)
        {
            collect_scripts_from_path(path, ret);
        }
    }
    return ret;
}
