#include "register_types.h"

#include "core/config/project_settings.h"
#include "core/object/class_db.h"
#include "core/core_constants.cpp"
#include "core/object/object.h"
#include "core/variant/variant.h"

#include "modules/lamia_game_tools/general/defs.h"
#include "modules/lamia_game_tools/databaser/database_select_dialog.h"
#include "modules/lamia_game_tools/databaser/databaser.h"
#include "modules/lamia_game_tools/console/console.h"
#include "modules/lamia_game_tools/console/console_command.h"
#include "modules/lamia_game_tools/console/console_db.h"
#include "modules/lamia_game_tools/eventer/builtin_commands.h"
#include "modules/lamia_game_tools/eventer/eventer_db.h"
#include "modules/lamia_game_tools/eventer/sequence.h"
#include "modules/lamia_game_tools/eventer/whiteboard.h"
#include "modules/lamia_game_tools/general/string_names.h"
#include "modules/lamia_game_tools/general/utility.h"
#include "modules/register_module_types.h"

#ifdef TOOLS_ENABLED
#include "modules/lamia_game_tools/databaser/editor_plugin.h"
#include "modules/lamia_game_tools/eventer/editor_plugin.h"
#endif

static LGTUtility *_lgt_utility = nullptr;
static Databaser *_databaser = nullptr;

void initialize_lamia_game_tools_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_CORE)
    {
        LGTStringNames::create();

        BIND_CORE_ENUM_CONSTANT(LGT_PROPERTY_HINT_DATABASE_ID_SELECT);
        BIND_CORE_ENUM_CONSTANT(LGT_PROPERTY_HINT_MAX);

        BIND_CORE_ENUM_CONSTANT(EV_INVALID)
        BIND_CORE_ENUM_CONSTANT(EV_RUNNING)
        BIND_CORE_ENUM_CONSTANT(EV_NEXT_IN_TREE)
        BIND_CORE_ENUM_CONSTANT(EV_NEXT_SIBLING)
        BIND_CORE_ENUM_CONSTANT(EV_EXIT_BRANCH)
        BIND_CORE_ENUM_CONSTANT(EV_ABORT)
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS)
    {
        GDREGISTER_CLASS(LGTUtility);
        _lgt_utility = memnew(LGTUtility);
        Engine::get_singleton()->add_singleton(Engine::Singleton("LGTUtility", _lgt_utility));

		GDREGISTER_CLASS(Databaser)
        _databaser = memnew(Databaser);
        Engine::get_singleton()->add_singleton(Engine::Singleton("Databaser", _databaser));
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        // General
        GDREGISTER_CLASS(FilterEdit);
        GDREGISTER_CLASS(FilterTree);

        // Console
        GLOBAL_DEF_BASIC(PropertyInfo(Variant::PACKED_STRING_ARRAY, LGTStringName(ConsoleGlobalCommandDirectories), PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:", Variant::STRING, PROPERTY_HINT_DIR)), PackedStringArray());
        GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, LGTStringName(ConsoleGlobalHistoryPath)), "console_history.txt");
        GLOBAL_DEF_BASIC(PropertyInfo(Variant::INT, LGTStringName(ConsoleGlobalHistoryMaxLines)), 1000);
        GDREGISTER_CLASS(ConsoleDB)
        GDREGISTER_CLASS(ConsoleCommandResult)
        GDREGISTER_VIRTUAL_CLASS(ConsoleCommand);
        GDREGISTER_CLASS(Console);

        // Console Commands
        CONSOLE_REGISTER_COMMAND(ConsoleCommandHelp);
        CONSOLE_REGISTER_COMMAND(ConsoleCommandClear);
        CONSOLE_REGISTER_COMMAND(ConsoleCommandListCommands);
        CONSOLE_REGISTER_COMMAND(ConsoleCommandClearHistory);

        // Databaser
        GDREGISTER_CLASS(DatabaseResource);
        GDREGISTER_CLASS(Database)
        GDREGISTER_INTERNAL_CLASS(DatabaseTree);
        GDREGISTER_CLASS(DatabaseSelectDialog);

        // Eventer
        GLOBAL_DEF_BASIC(PropertyInfo(Variant::PACKED_STRING_ARRAY, LGTStringName(EventerGlobalCommandDirectories), PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:", Variant::STRING, PROPERTY_HINT_DIR)), PackedStringArray());
        GDREGISTER_CLASS(EventerDB);
        GDREGISTER_ABSTRACT_CLASS(EVBase);
        GDREGISTER_CLASS(EventerWhiteboard)
        GDREGISTER_CLASS(EventerSequence);
        GDREGISTER_CLASS(EventerSequenceInstance)
        GDREGISTER_CLASS(EventerSequenceInterpreter);
        GDREGISTER_CLASS(EVCommand);

        // Eventer Default Types
        EVREGISTER_COMMAND(EVConsolePrint);
        EVREGISTER_COMMAND(EVVarCheck);
        EVREGISTER_COMMAND(EVVarErase);
        EVREGISTER_COMMAND(EVVarExists);
        EVREGISTER_COMMAND(EVVarSet);

        // AIGraph


    }

#ifdef TOOLS_ENABLED
    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR)
    {
        // Databaser
        GDREGISTER_INTERNAL_CLASS(DatabaseEditorDock);
        GDREGISTER_INTERNAL_CLASS(EditorDatabaseSelect);
        GDREGISTER_INTERNAL_CLASS(EditorPropertyDatabaseSelect)
        GDREGISTER_INTERNAL_CLASS(EditorInspectorPluginDatabaser)
        GDREGISTER_INTERNAL_CLASS(EditorPluginDatabaser)
        EditorPlugins::add_by_type<EditorPluginDatabaser>();

        // Eventer
        GDREGISTER_INTERNAL_CLASS(EventerAddCommandTree);
        GDREGISTER_INTERNAL_CLASS(EventerEditorTree);
        GDREGISTER_INTERNAL_CLASS(EventerEditor);
        GDREGISTER_INTERNAL_CLASS(EditorPluginEventer);
        EditorPlugins::add_by_type<EditorPluginEventer>();
    }
#endif
}

void uninitialize_lamia_game_tools_module(ModuleInitializationLevel p_level)
{
    // LGTStringNames::free();
}
