#pragma once

#include "database.h"
#include "../general/filter_tree.h"
#include "core/object/object.h"
#include "scene/gui/control.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/tree.h"

class DatabaseTree : public Control
{
    GDCLASS(DatabaseTree, Control)

private:
    LineEdit *id_editor = nullptr;
    FilterTree *tree = nullptr;
    PopupMenu *menu = nullptr;
    Ref<DatabaseResource> root;
    bool edit_enabled = false;

    bool is_edit_enabled() const;

    void popup_context_menu();
    void popup_context_menu(TreeItem* p_on_item);

    Ref<DatabaseResource> _normalize_drop_data(const Variant &p_data) const;
    void _add_from_dbr(Ref<DatabaseResource> p_from, StringName path, TreeItem* p_parent);
    void _on_menu_id_pressed(const int &p_id);
    void _on_item_selected();
    void _on_item_activated();
    void _on_item_mouse_selected(const Vector2 &p_mouse_position, const MouseButton &p_mouse_button_index);
    void _on_nothing_selected();

    enum MenuOptions
    {
        OPTION_SET_ID,
        OPTION_COPY_PATH, // allow this one without edit enabled
        OPTION_SHOW_IN_FILESYSTEM,
        OPTION_REMOVE
    };

protected:
    static void _bind_methods();

public:
    Variant _get_drag_data_fw(const Vector2 &p_at_position) const;
    bool _can_drop_data_fw(const Vector2 &p_at_position, const Variant &p_data) const;
    void _drop_data_fw(const Vector2 &p_at_position, const Variant &p_data);

    void set_edit_enabled(bool p_enabled) { edit_enabled = p_enabled; }
    bool get_edit_enabled() const { return edit_enabled; }
    void set_group(Ref<DatabaseResource> p_group);
    void set_filter(String p_filter);
    FilterTree* get_tree() const;
    void refresh();
    void clear() { tree->clear(); }

    StringName get_selected_data_path() const;
    Ref<DatabaseResource> get_selected_data() const;

    DatabaseTree();
};