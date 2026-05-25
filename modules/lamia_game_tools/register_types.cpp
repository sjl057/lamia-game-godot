#include "register_types.h"

#include "core/object/class_db.h"
#include "core/core_constants.cpp"

#include "general/defs.h"

#include "databaser/database_select_dialog.h"
#include "databaser/databaser.h"
#include "modules/register_module_types.h"

#ifdef TOOLS_ENABLED
#include "databaser/editor_plugin.h"
#endif

static Databaser *_databaser = nullptr;

void initialize_lamia_game_utils_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_CORE)
    {
        BIND_CORE_ENUM_CONSTANT(LGT_PROPERTY_HINT_DATABASE_ID_SELECT);
        BIND_CORE_ENUM_CONSTANT(LGT_PROPERTY_HINT_DATABASE_SELECT);
        BIND_CORE_ENUM_CONSTANT(LGT_PROPERTY_HINT_MAX);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        // General
        GDREGISTER_INTERNAL_CLASS(FilterEdit);
        GDREGISTER_INTERNAL_CLASS(FilterTree);

        // Databaser
        GDREGISTER_CLASS(DatabaseResource);
        GDREGISTER_CLASS(Database)

        GDREGISTER_INTERNAL_CLASS(DatabaseTree);
        GDREGISTER_CLASS(DatabaseSelectDialog);
		GDREGISTER_CLASS(Databaser)
        _databaser = memnew(Databaser);
        Engine::get_singleton()->add_singleton(Engine::Singleton("Databaser", _databaser));

        // Eventer

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
    }
#endif
}

void uninitialize_lamia_game_utils_module(ModuleInitializationLevel p_level) {}
