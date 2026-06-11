#pragma once

#include "modules/lamia_game_tools/general/filter_edit.h"
#ifdef TOOLS_ENABLED

#include "scene/resources/texture.h"
#include "core/object/object.h"
#include "core/templates/local_vector.h"
#include "editor/plugins/editor_plugin.h"
#include "modules/lamia_game_tools/eventer/ev.h"
#include "modules/lamia_game_tools/eventer/sequence.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/tree.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "modules/lamia_game_tools/general/filter_tree.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/split_container.h"

class EventerPickerPanel : public Control
{
    GDCLASS(EventerPickerPanel, Control);

private:
    FilterEdit *filter_edit = nullptr;
    FilterTree *tree = nullptr;

    void _on_tree_item_button_clicked(const TreeItem* p_item, const int &p_column, const bool &p_id, const int &p_mouse_button_index);
    // void _on_button_pressed(Ref<Script> p_script);

protected:
    static void _bind_methods();

public:
    void setup(Ref<EventerSequence> p_sequence);

    EventerPickerPanel();
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
    EventerPickerPanel *picker_panel = nullptr;
    Tree *tree = nullptr;
    PopupMenu *popup_menu = nullptr;

    Ref<EventerSequence> sequence;
    bool unsaved = false;

    Ref<EVBase> last_selected;
    Vector<Ref<EVBase>> clipboard;

    void refresh();
    void add_commands_from(Ref<EVBase> p_command, TreeItem *parent = nullptr);
    void show_popup_menu();

    Ref<EVBase> get_root_command() const;
    Ref<EVBase> get_selected_command() const;
    LocalVector<Ref<EVBase>> get_selected_commands() const;
    void cut_selected_commands();
    void copy_selected_commands();
    void paste_commands();
    void duplicate_selected_commands();
    void delete_selected_commands();

	void gui_input_fw(const Ref<InputEvent> &p_event);
    void get_drag_data_fw(const Point2 &at_position) const;
    void can_drop_data_fw(const Point2 &at_position, const Variant &p_data);
    void drop_data_fw(const Point2 &at_position, const Variant &p_data);

    void _on_command_added(Ref<EVBase> p_command);
    void _on_hsplit_drag_ended();

protected:
    void notification(int p_what);
    static void _bind_methods();

public:
    Ref<EventerSequence> get_sequence() const { return sequence; }

    void setup(Ref<EventerSequence> p_sequence);
    void save();
    void close();
    void set_separation(int p_separation);
    int get_separation() const;

    EventerEditorTree();
};

// class EventerEditorTab : public Control
// {

// };

class EventerEditor : public Control
{
    GDCLASS(EventerEditor, Control);

private:
    int separation = 0;

    PanelContainer *open_file_panel = nullptr;
    Label *open_file_label = nullptr;
    TabContainer *tabs = nullptr;

    void show_editor();
    void hide_editor();

    void _on_tab_closed();

protected:
    static void _bind_methods() {};

public:
    void open_sequence(Ref<EventerSequence> p_sequence);
    void save_current();
    void save_all();
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
//gui_input_fw
	// virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event) override { return path2d_editor->forward_gui_input(p_event); }

    virtual String get_plugin_name() const override { return "Eventer"; }
    bool has_main_screen() const override { return true; }
    const virtual Ref<Texture2D> get_plugin_icon() const override;
    virtual void edit(Object *p_object) override;
    virtual bool handles(Object *p_object) const override { return Object::cast_to<EventerSequence>(p_object); };
    virtual void make_visible(bool p_visible) override;
    virtual void save_external_data() override;
	virtual void set_window_layout(Ref<ConfigFile> p_layout) override;
	virtual void get_window_layout(Ref<ConfigFile> p_layout) override;
    virtual void set_state(const Dictionary &p_state) override;
    virtual Dictionary get_state() const override;

    EditorPluginEventer();
};

#endif