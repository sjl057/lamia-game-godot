#pragma once

#ifdef TOOLS_ENABLED

#include "modules/lamia_game_tools/general/filter_edit.h"
#include "scene/resources/texture.h"
#include "core/object/object.h"
#include "core/templates/local_vector.h"
#include "editor/plugins/editor_plugin.h"
#include "modules/lamia_game_tools/eventer/sequence.h"
#include "scene/gui/control.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/tree.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/general/filter_tree.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/split_container.h"

class EventerAddCommandTree : public Control
{
    GDCLASS(EventerAddCommandTree, Control);

private:
    FilterEdit *filter_edit = nullptr;
    FilterTree *tree = nullptr;

    void _add_command(Ref<EVCommand> p_command, TreeItem *p_parent);

    void _on_tree_item_button_clicked(const TreeItem* p_item, const int &p_column, const bool &p_id, const int &p_mouse_button_index);
    // void _on_button_pressed(Ref<Script> p_script);

protected:
    static void _bind_methods();

public:
    void setup(Ref<EventerSequence> p_sequence);

    EventerAddCommandTree();
};

class EventerEditorTree : public Control
{
    GDCLASS(EventerEditorTree, Control);

private:
    enum MenuAction
    {
        ACTION_CUT = 0,
        ACTION_COPY,
        ACTION_PASTE,
        ACTION_DUPLICATE,
        ACTION_EDIT_SCRIPT = 10,
        ACTION_DELETE
    };

    HSplitContainer *hsplit = nullptr;
    EventerAddCommandTree *add_command_tree = nullptr;
    Tree *tree = nullptr;
    PopupMenu *popup_menu = nullptr;

    Ref<EventerSequence> sequence;
    bool unsaved = false;

    Ref<EVBase> last_selected;

    void refresh();
    void add_commands_from(Ref<EVBase> p_command, TreeItem *parent = nullptr);
    void show_popup_menu();

    Ref<EVBase> get_selected_command() const;
    LocalVector<Ref<EVBase>> get_selected_commands() const;
    Ref<EVBase> get_command_at_position(const Point2 &at_position) const;

    void cut_selected_commands();
    void copy_selected_commands();
    void paste_commands();
    void duplicate_selected_commands();
    void delete_selected_commands();
    void edit_selected_command_script();

    TypedArray<Ref<EVBase>> normalize_drag_data(const Variant &p_data) const;
    Variant get_drag_data_fw(const Point2 &at_position) const;
    bool can_drop_data_fw(const Point2 &at_position, const Variant &p_data);
    void drop_data_fw(const Point2 &at_position, const Variant &p_data);

    void _on_command_added(Ref<EVBase> p_command);
    void _on_item_activated();
    void _on_item_mouse_selected(const Vector2 &p_mouse_position, const MouseButton &p_mouse_button_index);
    void _on_popup_menu_id_pressed(const int &p_id);
    void _on_hsplit_drag_ended();

protected:
    virtual void shortcut_input(const Ref<InputEvent> &p_event) override;
    void _notification(int p_what);
    static void _bind_methods();

public:
    Ref<EVBase> get_root_command() const;

    Ref<EventerSequence> get_sequence() const { return sequence; }

    void setup(Ref<EventerSequence> p_sequence);
    void save();
    void close();
    void set_separation(int p_separation);
    int get_separation() const;

    EventerEditorTree();
};

class EventerEditor : public Control
{
    GDCLASS(EventerEditor, Control);

private:
    int separation = 0;

    PanelContainer *open_file_panel = nullptr;
    Label *open_file_label = nullptr;
    TabContainer *tabs = nullptr;

    Vector<Ref<EVBase>> clipboard;

    void show_editor();
    void hide_editor();

    void _on_tab_button_pressed(const int &p_tab);
    void _on_tab_closed();

protected:
    void copy_commands(Array p_commands);
    void paste_commands(Ref<EVBase> p_paste_target, EventerEditorTree *p_tree);

    static void _bind_methods() {};

public:
    void open_sequence(Ref<EventerSequence> p_sequence);
    void save_current();
    void save_all();
    void close_tab(const int &p_tab);
    void close_all();

    void set_separation(int p_separation);
    int get_separation() const;

    PackedStringArray get_open_tab_paths() const;

    EventerEditor();
};

class EditorPluginEventer : public EditorPlugin
{
    GDCLASS(EditorPluginEventer, EditorPlugin);

private:
    EventerEditor *editor = nullptr;

protected:
    static void _bind_methods() {};

public:
    virtual String get_plugin_name() const override { return "Eventer"; }
    bool has_main_screen() const override { return true; }
    const virtual Ref<Texture2D> get_plugin_icon() const override;
    virtual void edit(Object *p_object) override;
    virtual bool handles(Object *p_object) const override { return Object::cast_to<EventerSequence>(p_object); };
    virtual void make_visible(bool p_visible) override;
    virtual void save_external_data() override;
	virtual void set_window_layout(Ref<ConfigFile> p_layout) override;
	virtual void get_window_layout(Ref<ConfigFile> p_layout) override;

    EditorPluginEventer();
};

#endif