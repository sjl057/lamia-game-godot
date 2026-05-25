#ifdef TOOLS_ENABLED

#include "editor_plugin.h"

#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "database_tree.h"
#include "databaser.h"
#include "database.h"
#include "editor/editor_interface.h"
#include "editor/editor_node.h"
#include "editor/file_system/editor_file_system.h"
#include "../general/defs.h"
#include "scene/main/node.h"
#include "editor/editor_string_names.h"
#include "core/string/print_string.h"

DatabaseEditorDock::DatabaseEditorDock()
{
    set_title("Database");
    set_default_slot(DOCK_SLOT_BOTTOM);
    set_available_layouts(DOCK_LAYOUT_ALL);

    tab_menu = memnew(PopupMenu);
    tab_menu->connect("id_pressed", callable_mp(this, &DatabaseEditorDock::_on_menu_id_pressed));
    add_child(tab_menu);

    VBoxContainer *vbox = memnew(VBoxContainer);
    add_child(vbox);

    filter_edit = memnew(FilterEdit);
    vbox->add_child(filter_edit);

    tab_container = memnew(TabContainer);
    tab_container->set_v_size_flags(SIZE_EXPAND_FILL);
    tab_container->set_drag_to_rearrange_enabled(true);
    tab_container->connect("tab_clicked", callable_mp(this, &DatabaseEditorDock::_on_tab_clicked));
    tab_container->connect("active_tab_rearranged", callable_mp(this, &DatabaseEditorDock::_on_active_tab_rearranged));
    vbox->add_child(tab_container);

    TabBar *tab_bar = tab_container->get_tab_bar();
    tab_bar->connect("tab_rmb_clicked", callable_mp(this, &DatabaseEditorDock::_on_tab_rmb_clicked));

    EditorFileSystem *fs = EditorInterface::get_singleton()->get_resource_filesystem();
    fs->connect("script_classes_updated", callable_mp(this, &DatabaseEditorDock::_refresh_tabs));
    _refresh_tabs();
}

void DatabaseEditorDock::popup_tab_menu(int p_tab_idx)
{
    ERR_FAIL_INDEX(p_tab_idx, tab_container->get_child_count());
    tab_menu->clear();
    tab_menu->add_item("Delete Orphaned Group", MenuAction::OPTION_DELETE_GROUP);

    tab_menu->popup(Rect2i(DisplayServer::get_singleton()->mouse_get_position(), Vector2i(30.0f, 50.0f)));
}

void DatabaseEditorDock::_refresh_tabs()
{
    for (int i = 0; i < tabs.size(); i++)
    {
        DatabaseTree *tab = tabs[i];

        if (filter_edit->is_connected("text_changed", callable_mp(tab, &DatabaseTree::set_filter)))
        {
            filter_edit->disconnect("text_changed", callable_mp(tab, &DatabaseTree::set_filter));
        }

        tab_container->remove_child(tab);
        tab->queue_free();
    }
    tabs.clear();

    if (editing.is_valid())
    {
        if (editing->is_connected("changed", callable_mp(this, &DatabaseEditorDock::_refresh_current_tab)))
        {
            editing->disconnect("changed", callable_mp(this, &DatabaseEditorDock::_refresh_current_tab));
        }
    }

    if (not is_visible()) { return; }

    Ref<Database> database = Databaser::get_singleton()->get_database();

    if (not database.is_valid()) { return; }

    TypedArray<Dictionary> type_dict = Databaser::get_type_dict();
    for (int i = 0; i < type_dict.size(); i++)
    {
        Dictionary type = type_dict[i];
        if (not database->get_groups().has(type["class"]))
        {
            Ref<DatabaseResource> root;
            root.instantiate();
            root->set_path(vformat("%s::%s", database->get_path(), Resource::generate_scene_unique_id()), true);
            root->set_script(ResourceLoader::load(type["path"]));
            Dictionary d = database->get_groups();
            d.set(type["class"], root);
            database->set_groups(d);
        }
        if (not database->get_group_order().has(type["class"]))
        {
            PackedStringArray order = database->get_group_order();
            order.append(type["class"]);
            database->set_group_order(order);
        }
    }
    ResourceSaver::save(database);
    
    for (int i = 0; i < database->get_groups().size(); i++)
    {
        Ref<DatabaseResource> root = database->get_groups()[database->get_group_order()[i]];

        DatabaseTree *tab = memnew(DatabaseTree);
        tab->set_edit_enabled(true);
        tab->set_name(database->get_group_order()[i]);
        tab->set_group(root);
        tab->connect("data_activated", callable_mp(this, &DatabaseEditorDock::_on_data_activated).bind(tab->get_name()));
        tab_container->add_child(tab);
        tabs.push_back(tab);
        filter_edit->connect("text_changed", callable_mp(tab, &DatabaseTree::set_filter));
    }

    if (current_tab >= 0)
    {
        tab_container->set_current_tab(current_tab);
    }
    else
    {
        current_tab = 0;
    }
}

void DatabaseEditorDock::_refresh_current_tab()
{
    if (not is_visible()) { return; } 
    if (current_tab < 0) { return; }
    DatabaseTree *tree = Object::cast_to<DatabaseTree>(tab_container->get_child(current_tab));
    if (tree)
    {
        tree->refresh();
    }
}

void DatabaseEditorDock::_on_data_activated(StringName p_path, Ref<DatabaseResource> p_data, StringName p_group)
{
    EditorInterface::get_singleton()->edit_resource(p_data);

    if (editing.is_valid())
    {
        if (editing->is_connected("changed", callable_mp(this, &DatabaseEditorDock::_refresh_current_tab)))
        {
            editing->disconnect("changed", callable_mp(this, &DatabaseEditorDock::_refresh_current_tab));
        }
    }
    editing = p_data;
    if (editing.is_valid())
    {
        editing->connect("changed", callable_mp(this, &DatabaseEditorDock::_refresh_current_tab));
    }
}

void DatabaseEditorDock::_on_tab_clicked(int p_tab)
{
    current_tab = p_tab;
    
}

void DatabaseEditorDock::_on_tab_rmb_clicked(int p_tab)
{
    Ref<Database> database = Databaser::get_singleton()->get_database();
    if (not Databaser::get_singleton()->has_type((tab_container->get_tab_control(p_tab)->get_name())))
    {
        popup_tab_menu(p_tab);
    }
}

void DatabaseEditorDock::_on_active_tab_rearranged(int p_idx_to)
{
    Ref<Database> database = Databaser::get_singleton()->get_database();

    Vector<String> new_order;
    int size = tab_container->get_child_count();
    new_order.resize(size);
    for (int i = 0; i < size; i++)
    {
        new_order.set(i, (String)(tab_container->get_child(i)->get_name()));
    }
    database->set_group_order(new_order);

    _refresh_tabs();
}

void DatabaseEditorDock::_on_menu_id_pressed(int p_id)
{
    switch (static_cast<MenuAction>(p_id))
    {
        case OPTION_DELETE_GROUP:
            Ref<Database> database = Databaser::get_singleton()->get_database();

            database->get_groups().erase(tab_container->get_current_tab_control()->get_name());
            PackedStringArray order = database->get_group_order();
            order.erase(tab_container->get_current_tab_control()->get_name());
            database->set_group_order(order);
            database->recursive_save();
            tab_container->get_current_tab_control()->queue_free();
            break;
    }
}


EditorDatabaseSelect::EditorDatabaseSelect()
{
    add_theme_constant_override(SNAME("separation"), 0);

    select_button = memnew(Button);
    select_button->set_text("Select...");
    select_button->set_h_size_flags(SIZE_EXPAND_FILL);
    select_button->set_clip_text(true);
    select_button->set_expand_icon(true);
    select_button->connect("pressed", callable_mp(this, &EditorDatabaseSelect::_on_select_button_pressed));
    add_child(select_button);

    menu_button = memnew(MenuButton);
    menu_button->set_flat(true);
    menu_button->get_popup()->add_item("Clear", ACTION_CLEAR);
    menu_button->get_popup()->add_item("Copy", ACTION_COPY);
    menu_button->get_popup()->add_item("Edit", ACTION_EDIT);
    menu_button->get_popup()->add_item("Open Resource", ACTION_OPEN_RESOURCE);
    menu_button->get_popup()->connect("id_pressed", callable_mp(this, &EditorDatabaseSelect::_on_menu_id_pressed));
    menu_button->get_popup()->connect("about_to_popup", callable_mp(this, &EditorDatabaseSelect::_refresh_menu));
    add_child(menu_button);

    Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();
    if (theme.is_valid())
    {
        menu_button->set_button_icon(theme->get_icon(SNAME("GuiTabMenuHl"), EditorStringName(EditorIcons)));
        menu_button->get_popup()->set_item_icon(0, theme->get_icon(SNAME("Clear"), EditorStringName(EditorIcons)));
        menu_button->get_popup()->set_item_icon(1, theme->get_icon(SNAME("ActionCopy"), EditorStringName(EditorIcons)));
        menu_button->get_popup()->set_item_icon(2, theme->get_icon(SNAME("Edit"), EditorStringName(EditorIcons)));
        menu_button->get_popup()->set_item_icon(3, theme->get_icon(SNAME("File"), EditorStringName(EditorIcons)));
    }

    path_edit = memnew(LineEdit);
    path_edit->set_h_size_flags(SIZE_EXPAND_FILL);
    path_edit->hide();
    path_edit->connect("focus_exited", callable_mp(this, &EditorDatabaseSelect::_on_path_edit_text_focus_exited));
    path_edit->connect("text_submitted", callable_mp(this, &EditorDatabaseSelect::_on_path_edit_text_submitted));
    add_child(path_edit);
}

void EditorDatabaseSelect::set_path(StringName p_path)
{
    path = p_path;
    if (not path.is_empty())
    {
        int slice_count = ((String)path).get_slice_count("/");
        if (slice_count > 2)
        {
            select_button->set_text(vformat("...%s/%s", ((String)path).get_slice("/", slice_count - 2), ((String)path).get_slice("/", slice_count - 1)));
        }
        else
        {
            select_button->set_text(path);
        }
        select_button->set_flat(true);
    }
    else
    {
        select_button->set_text("Select...");
        // icon
        select_button->set_flat(false);
    }
    emit_signal("path_changed", path);
}

void EditorDatabaseSelect::_on_menu_id_pressed(int p_id)
{
    switch (static_cast<MenuAction>(p_id))
    {
        case ACTION_CLEAR:
            set_path(StringName());
            break;
        case ACTION_COPY:
            DisplayServer::get_singleton()->clipboard_set(path);
            break;
        case ACTION_EDIT:
            path_edit->set_text(path);
            path_edit->show();
            path_edit->grab_focus();
            select_button->hide();
            menu_button->hide();
            break;
        case ACTION_OPEN_RESOURCE:
            break;
    }
}

void EditorDatabaseSelect::_on_select_button_pressed()
{
    ERR_FAIL_COND_MSG(not Databaser::get_singleton()->get_database().is_valid(), "No Database specified");
    Databaser::get_singleton()->popup_database_id_select(callable_mp(this, &EditorDatabaseSelect::set_path).unbind(1), type, "");
}

void EditorDatabaseSelect::_on_path_edit_text_focus_exited()
{
    _on_path_edit_text_submitted(path_edit->get_text());
}

void EditorDatabaseSelect::_on_path_edit_text_submitted(String p_text)
{
    set_path(p_text);
    path_edit->hide();
    path_edit->set_text(String());
    select_button->show();
    menu_button->show();
}

void EditorDatabaseSelect::_refresh_menu()
{
    menu_button->get_popup()->set_item_disabled(ACTION_CLEAR, path.is_empty());
    menu_button->get_popup()->set_item_disabled(ACTION_COPY, path.is_empty());
    menu_button->get_popup()->set_item_disabled(ACTION_OPEN_RESOURCE, path.is_empty());
}

void EditorDatabaseSelect::_bind_methods()
{
    ADD_SIGNAL(MethodInfo("path_changed"));
}


EditorPropertyDatabaseSelect::EditorPropertyDatabaseSelect()
{
    select = memnew(EditorDatabaseSelect);
    select->set_type(type);
    select->connect("path_changed", callable_mp(this, &EditorPropertyDatabaseSelect::_on_path_changed));
    add_child(select);
    add_focusable(select->get_path_edit());
}

void EditorPropertyDatabaseSelect::set_type(const StringName p_type)
{
    type = p_type;
    select->set_type(type);
}

void EditorPropertyDatabaseSelect::update_property()
{
    updating = true;
    select->set_path(get_edited_object()->get(get_edited_property()));
    updating = false;
}

void EditorPropertyDatabaseSelect::_on_path_changed(StringName p_new_path)
{
    if (updating) { return; }
    emit_changed(get_edited_property(), p_new_path);
}


bool EditorInspectorPluginDatabaser::parse_property(Object *p_object, Variant::Type p_type, const String &p_name, PropertyHint p_hint_type, const String &p_hint_string, BitField<PropertyUsageFlags> p_usage_flags, bool p_wide)
{
    switch (static_cast<LGTPropertyHint>(p_hint_type))
    {
        case LGT_PROPERTY_HINT_DATABASE_ID_SELECT:
            if ((p_type == Variant::STRING_NAME or p_type == Variant::STRING) and Databaser::has_type(p_hint_string))
            {
                EditorPropertyDatabaseSelect *editor = memnew(EditorPropertyDatabaseSelect);
                editor->set_type(p_hint_string);
                add_property_editor(p_name, editor);
                return true;
            }
            return false;
        case LGT_PROPERTY_HINT_DATABASE_SELECT:
            if (p_type == Variant::OBJECT and Databaser::has_type(p_hint_string))
            {
                
                return true;
            }
            return false;
        default:
            return false;
    }
}

void EditorPluginDatabaser::_notification(int p_what)
{
    switch (p_what)
    {   
        case NOTIFICATION_ENTER_TREE:
            GLOBAL_DEF_BASIC(PropertyInfo(Variant::STRING, "databaser/config/database", PROPERTY_HINT_FILE, "*.tres"), "");
            inspector_plugin = memnew(EditorInspectorPluginDatabaser);
            add_inspector_plugin(inspector_plugin);

            editor_dock = memnew(DatabaseEditorDock);
            add_dock(editor_dock);
            break;

        case NOTIFICATION_EXIT_TREE:
            remove_inspector_plugin(inspector_plugin);
            remove_dock(editor_dock);
            editor_dock->queue_free();
            break;
    }
}

void EditorPluginDatabaser::save_external_data()
{
    Ref<Database> database = Databaser::get_singleton()->get_database();
    if (database.is_valid())
    {
        database->recursive_save();
    }
}

#endif // TOOLS_ENABLED