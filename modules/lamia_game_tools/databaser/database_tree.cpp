#include "database_tree.h"
#include "core/error/error_macros.h"
#include "core/input/input_enums.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "database.h"
#include "databaser.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_interface.h"
#include "editor/docks/filesystem_dock.h"
#endif

#include "modules/lamia_game_tools/general/filter_tree.h"

DatabaseTree::DatabaseTree()
{
    set_h_size_flags(SIZE_EXPAND_FILL);
    set_v_size_flags(SIZE_EXPAND_FILL);

    menu = memnew(PopupMenu);
    menu->connect("id_pressed", callable_mp(this, &DatabaseTree::_on_menu_id_pressed));
    add_child(menu);

    tree = memnew(FilterTree);
    tree->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
    tree->set_select_mode(Tree::SELECT_ROW);
    tree->set_allow_rmb_select(true);
    tree->set_hide_root(true);
    tree->set_columns(2);
    tree->set_column_titles_visible(true);
    // titles cause a font not found error? IT'S FIXED BY CHANGING THE SELECT MODE?????
    tree->set_column_title(0, "ID");
    tree->set_column_title(1, "Name");
    tree->set_column_expand(0, true);
    tree->set_column_expand_ratio(0, 2);
    tree->connect("item_selected", callable_mp(this, &DatabaseTree::_on_item_selected));
    tree->connect("item_activated", callable_mp(this, &DatabaseTree::_on_item_activated));
    tree->connect("item_mouse_selected", callable_mp(this, &DatabaseTree::_on_item_mouse_selected));
    tree->connect("nothing_selected", callable_mp(this, &DatabaseTree::_on_nothing_selected));
    tree->set_drag_forwarding(callable_mp(this, &DatabaseTree::_get_drag_data_fw), callable_mp(this, &DatabaseTree::_can_drop_data_fw), callable_mp(this, &DatabaseTree::_drop_data_fw));
    add_child(tree);

    id_editor = memnew(LineEdit);
    id_editor->set_context_menu_enabled(false);
    id_editor->connect("text_submitted", callable_mp(this, &DatabaseTree::_on_id_edit_submitted));
    id_editor->connect("focus_exited", callable_mp(this, &DatabaseTree::_on_id_edit_focus_exited));
    id_editor->hide();
    add_child(id_editor);
}

void DatabaseTree::_bind_methods()
{
    ADD_SIGNAL(MethodInfo("data_selected"));
    ADD_SIGNAL(MethodInfo("data_activated"));

    ADD_SIGNAL(MethodInfo("data_changed"));
}

bool DatabaseTree::is_edit_enabled() const
{
    if (Engine::get_singleton()->is_editor_hint())
    {
        return edit_enabled;
    }
    else
    {
        return false;
    }
}

void DatabaseTree::popup_context_menu()
{
    if (not edit_enabled) { return; }

    menu->clear();
}

void DatabaseTree::popup_context_menu(TreeItem* p_on_item)
{
    if (not is_edit_enabled()) { return; }
    if (not p_on_item) { return; }

    menu->clear();

    menu->add_item("Set ID", OPTION_SET_ID);
    menu->add_item("Copy Path", OPTION_COPY_PATH);
    menu->add_item("Show in Filesystem", OPTION_SHOW_IN_FILESYSTEM);
    menu->add_item("Remove", OPTION_REMOVE);

    menu->popup(Rect2i(DisplayServer::get_singleton()->mouse_get_position(), Size2i(50.0f, 90.0f)));
}

void DatabaseTree::set_group(Ref<DatabaseResource> p_group)
{
    ERR_FAIL_NULL(p_group);
    root = p_group;
    refresh();
}

void DatabaseTree::set_filter(String p_filter)
{
    tree->set_filter(p_filter);
}

FilterTree* DatabaseTree::get_tree() const
{
    return tree;
}

void DatabaseTree::refresh()
{
    tree->clear();
    if (not root.is_valid()) { return; }
    TreeItem *tree_item_root = tree->create_item();
    tree_item_root->set_text(0, root->get_data_id());
    tree_item_root->set_text(1, root->get_data_name());
    tree_item_root->set_metadata(0, StringName());
    tree_item_root->set_metadata(1, root);
    _add_from_dbr(root, StringName(), nullptr);
}

void DatabaseTree::_add_from_dbr(Ref<DatabaseResource> p_from, StringName path, TreeItem* p_parent)
{
    for (int i = 0; i < p_from->get_child_count(); i++)
    {
        Ref<DatabaseResource> child = p_from->get_child(i);
        TreeItem *item = tree->create_item(p_parent);
        String name = child->get_data_name();
        StringName id = child->get_data_id();

        item->set_metadata(0, (String)path + id);
        item->set_metadata(1, child);
        
        if (name.is_empty()) { name = "<no name>"; }
        item->set_text(0, id);
        item->set_text(1, name);

        if (child->get_child_count() > 0)
        {
            String new_path = not path.is_empty() ? vformat("%s%s/", path, id) : vformat("%s/", id);
            _add_from_dbr(child, new_path, item);
        }
    }
}

StringName DatabaseTree::get_selected_data_path() const
{
    if (tree->get_selected() and tree->get_selected()->get_metadata(0))
    {
        return tree->get_selected()->get_metadata(0);
    }
    return StringName();
}

Ref<DatabaseResource> DatabaseTree::get_selected_data() const
{
    if (tree->get_selected() and tree->get_selected()->get_metadata(1))
    {
        return tree->get_selected()->get_metadata(1);
    }
    return nullptr;
}

Ref<DatabaseResource> DatabaseTree::_normalize_drop_data(const Variant &p_data) const
{
    if (p_data.get_type() == Variant::DICTIONARY)
    {
        Dictionary data = (Dictionary)p_data;
        if (data.has("type"))
        {
            if (data["type"] == "database_tree_item" and data.has("data"))
            {
                Ref<DatabaseResource> res = Object::cast_to<DatabaseResource>(data["data"]);
                if (res.is_valid())
                {
                    return res;
                }
            }
            else if (data["type"] == "files" and data.has("files"))
            {
                PackedStringArray files = data["files"];
                if (files.is_empty() or files.size() > 1) { return nullptr; }
                if (ResourceLoader::exists(files[0], "DatabaseResource"))
                {
                    Ref<Resource> res = ResourceLoader::load(files[0], "DatabaseResource");
                    if (res.is_valid() and Object::cast_to<DatabaseResource>(res.ptr()))
                    {
                        return res;
                    }
                }
            }
        } 
    }
    return nullptr;
}

Variant DatabaseTree::_get_drag_data_fw(const Vector2 &p_at_position) const
{
    if (not is_edit_enabled()) { return Variant(); }

    TreeItem *item = tree->get_item_at_position(p_at_position);
    if (item)
    {
        Ref<DatabaseResource> data = Object::cast_to<DatabaseResource>(item->get_metadata(1));
        
        Label *preview = memnew(Label);
        preview->set_text(data->get_data_name());
        tree->set_drag_preview(preview);

        Dictionary drag_data;
        drag_data.set("type", "database_tree_item");
        drag_data.set("path", item->get_metadata(0));
        drag_data.set("data", data);

        return drag_data;
    }

    return Variant();
}

bool DatabaseTree::_can_drop_data_fw(const Vector2 &p_at_position, const Variant &p_data) const
{
    if (not is_edit_enabled()) { return false; }

    tree->set_drop_mode_flags(Tree::DROP_MODE_INBETWEEN | Tree::DROP_MODE_ON_ITEM);

    Ref<DatabaseResource> normalized_data = _normalize_drop_data(p_data);
    if (normalized_data.is_valid())
    {
        TreeItem *item = tree->get_item_at_position(p_at_position);
        if (not item)
        {
            item = tree->get_root();
        }
        Ref<DatabaseResource> child = Object::cast_to<DatabaseResource>(item->get_metadata(1));
        if (child.is_valid())
        {
            return child->can_add_child(normalized_data);
        }
    }
    
    return false;
}

void DatabaseTree::_drop_data_fw(const Vector2 &p_at_position, const Variant &p_data)
{
    if (not is_edit_enabled()) { return; }

    Ref<DatabaseResource> normalized_data = _normalize_drop_data(p_data);

    if (normalized_data.is_valid())
    {
        TreeItem *item = tree->get_item_at_position(p_at_position);
        if (not item)
        {
            item = tree->get_root();
        }
        Ref<DatabaseResource> child = Object::cast_to<DatabaseResource>(item->get_metadata(1));
        if (child.is_valid())
        {
            normalized_data->reparent(child);
        }
        refresh();
        root->recursive_save();
    }
}

void DatabaseTree::_on_id_edit_submitted(const String &p_text)
{
    if (id_editor->is_visible())
    {
        TreeItem *selected = tree->get_selected();
        if (selected)
        {
            Ref<DatabaseResource> data = Object::cast_to<DatabaseResource>(selected->get_metadata(1));
            if (data.is_valid())
            {
                data->set_data_id(p_text);
            }
            refresh();
        }

        id_editor->set_text(String());
        id_editor->hide();
    }
}

void DatabaseTree::_on_id_edit_focus_exited()
{
    if (id_editor->is_visible())
    {
        TreeItem *selected = tree->get_selected();
        if (selected)
        {
            Ref<DatabaseResource> data = Object::cast_to<DatabaseResource>(selected->get_metadata(1));
            if (data.is_valid())
            {
                data->set_data_id(id_editor->get_text());
            }
            refresh();
        }

        id_editor->set_text(String());
        id_editor->hide();
    }
}

void DatabaseTree::_on_menu_id_pressed(const int &p_id)
{
    if (not is_edit_enabled()) { return; }
    switch (static_cast<MenuOptions>(p_id))
    {
        case OPTION_SET_ID:
        {
            TreeItem *selected = tree->get_selected();
            Ref<DatabaseResource> data = Object::cast_to<DatabaseResource>(selected->get_metadata(1));
            Rect2 rect = tree->get_item_rect(selected, 0);
            id_editor->set_position(rect.position);
            id_editor->set_size(rect.size);
            id_editor->set_text(data->get_data_id());
            id_editor->show();
            id_editor->grab_focus();
            break;
        }
        case OPTION_COPY_PATH:
        {
            StringName path = StringName(tree->get_selected()->get_metadata(0));
            if (path)
            {   
                DisplayServer::get_singleton()->clipboard_set(path);
            }
            break;
        }
        case OPTION_SHOW_IN_FILESYSTEM:
        {
            if (Engine::get_singleton()->is_editor_hint())
            {
                Ref<DatabaseResource> child = Object::cast_to<DatabaseResource>(tree->get_selected()->get_metadata(1));
                if (child.is_valid())
                {   
                    FileSystemDock *fs_dock = EditorInterface::get_singleton()->get_file_system_dock();
                    fs_dock->navigate_to_path(child->get_path());
                }
            }
            break;
        }
        case OPTION_REMOVE:
        {
            Ref<DatabaseResource> child = Object::cast_to<DatabaseResource>(tree->get_selected()->get_metadata(1));
            if (child.is_valid())
            {
                child->get_parent()->remove_child(child);
            }
            // root->recursive_save();
            refresh();
            break;
        }
    }
}


// make other functions use both path and ref to resource
void DatabaseTree::_on_item_selected()
{
    if (tree->get_selected())
    {
        emit_signal("data_selected", tree->get_selected()->get_metadata(0), tree->get_selected()->get_metadata(1));
    }
}

void DatabaseTree::_on_item_activated()
{
    if (tree->get_selected())
    {
        emit_signal("data_activated", tree->get_selected()->get_metadata(0), tree->get_selected()->get_metadata(1));
    }
}

void DatabaseTree::_on_item_mouse_selected(const Vector2 &p_mouse_position, const MouseButton &p_mouse_button_index)
{
    if (not is_edit_enabled()) { return; }

    if (p_mouse_button_index == MouseButton::RIGHT)
    {
        TreeItem* item = tree->get_item_at_position(p_mouse_position);
        if (item)
        {
            popup_context_menu(item);
        }
        else
        {
            popup_context_menu();
        }
    }
}

void DatabaseTree::_on_nothing_selected()
{
    tree->deselect_all();
}