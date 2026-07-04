#pragma once

#include "core/string/string_name.h"

class LGTStringNames
{
    inline static LGTStringNames *singleton = nullptr;

public:
    static void create() { singleton = memnew(LGTStringNames); }
    static void free()
    {
        memdelete(singleton);
        singleton = nullptr;
    }

    _FORCE_INLINE_ static LGTStringNames *get_singleton() { return singleton; }

    // Eventer
    const StringName EventerGlobalCommandDirectories = StringName("eventer/commands/command_directories");

    // Console
    const StringName ConsoleGlobalCommandDirectories = StringName("console/commands/command_directories");
    const StringName ConsoleGlobalHistoryPath = StringName("console/history/history_path");
    const StringName ConsoleGlobalHistoryMaxLines = StringName("console/history/history_max_lines");

    
};

#define LGTStringName(m_name) LGTStringNames::get_singleton()->m_name
