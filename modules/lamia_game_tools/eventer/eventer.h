#pragma once

#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/eventer/ev.h"
#include <type_traits>

class Eventer : public Object
{
    GDCLASS(Eventer, Object);

private:
    void collect_scripts_from_path(String p_path, PackedStringArray &r_ret);


protected:
    static Eventer *singleton;
    static void _bind_methods();

public:
    static Eventer *get_singleton();
    PackedStringArray get_command_script_paths();


    
    // Eventer();
    // ~Eventer();
};