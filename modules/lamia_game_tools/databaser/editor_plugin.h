#pragma once
#ifdef TOOLS_ENABLED

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "database.h"
#include "database_tree.h"
#include "editor/docks/editor_dock.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/plugins/editor_plugin.h"
#include "../general/filter_edit.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/tab_container.h"

class DatabaseEditorDock : public EditorDock
{
    GDCLASS(DatabaseEditorDock, EditorDock)

private:
    PopupMenu *tab_menu = nullptr;
    FilterEdit *filter_edit = nullptr;
    TabContainer *tab_container = nullptr;
    Vector<DatabaseTree*> tabs;
    int current_tab = -1;

    Ref<Resource> editing = nullptr;

    enum MenuAction
    {
        OPTION_DELETE_GROUP
    };

    void popup_tab_menu(int p_tab_idx);

    void _refresh_tabs();
    void _refresh_current_tab();
    void _on_data_activated(StringName p_path, Ref<DatabaseResource> p_data, StringName p_group);
    void _on_tab_clicked(int p_tab);
    void _on_tab_rmb_clicked(int p_tab);
    void _on_active_tab_rearranged(int p_idx_to);

    void _on_menu_id_pressed(int p_id);

protected:
    static void _bind_methods() {};

public:
    DatabaseEditorDock();
};

class EditorDatabaseSelect : public HBoxContainer
{
    GDCLASS(EditorDatabaseSelect, HBoxContainer)

private:
    enum MenuAction
    {
        ACTION_CLEAR,
        ACTION_COPY,
        ACTION_EDIT,
        ACTION_OPEN_RESOURCE
    };

    StringName type;
    StringName path;

    Button *select_button;
    MenuButton *menu_button;
    LineEdit *path_edit;

    void _on_menu_id_pressed(int p_id);
    void _on_select_button_pressed();
    void _on_path_edit_text_focus_exited();
    void _on_path_edit_text_submitted(String p_text);
    void _refresh_menu();

protected:
    static void _bind_methods();

public:
    void set_type(StringName p_type) { type = p_type; }
    void set_path(StringName p_path);
    LineEdit *get_path_edit() const { return path_edit; };

    EditorDatabaseSelect();
};

class EditorPropertyDatabaseSelect : public EditorProperty
{
    GDCLASS(EditorPropertyDatabaseSelect, EditorProperty)

private:
    bool updating = false;
    StringName type;
    EditorDatabaseSelect *select = nullptr;

    void _on_path_changed(StringName p_new_path);

protected:
    static void _bind_methods() {};

public:
    void set_type(const StringName p_type);
    virtual void update_property() override;

    EditorPropertyDatabaseSelect();
};

class EditorInspectorPluginDatabaser : public EditorInspectorPlugin
{
    GDCLASS(EditorInspectorPluginDatabaser, EditorInspectorPlugin)

protected:
    static void _bind_methods() {};

public:
    virtual bool can_handle(Object *p_object) override { return true; };
    virtual bool parse_property(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type, const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide) override;

};

class EditorPluginDatabaser : public EditorPlugin
{
    GDCLASS(EditorPluginDatabaser, EditorPlugin)

private:
    EditorInspectorPluginDatabaser *inspector_plugin = nullptr;
    DatabaseEditorDock *editor_dock = nullptr;

protected:
    void _notification(int p_what);
    static void _bind_methods() {};

public:
    virtual void save_external_data() override;
};

#endif // TOOLS_ENABLED