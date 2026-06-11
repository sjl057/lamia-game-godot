#include "editor/editor_string_names.h"
#include "modules/lamia_game_tools/eventer/eventer_db.h"
#include "modules/lamia_game_tools/general/filter_edit.h"
#include "modules/lamia_game_tools/general/filter_tree.h"
#include "scene/gui/box_container.h"
#ifdef TOOLS_ENABLED

#include "core/input/input_event.h"
#include "core/math/math_defs.h"
#include "core/os/keyboard.h"
#include "editor/editor_interface.h"
#include "scene/gui/label.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/tree.h"
#include "core/error/error_macros.h"
#include "core/io/resource_loader.h"
#include "core/os/memory.h"
#include "core/templates/a_hash_map.h"
#include "scene/gui/control.h"
#include "scene/gui/flow_container.h"
#include "core/config/project_settings.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"
#include "editor/editor_main_screen.h"
#include "scene/resources/theme.h"
#include "editor_plugin.h"
#include "core/object/object.h"
#include "editor/editor_node.h"
#include "scene/gui/popup_menu.h"
#include "scene/main/node.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/script_language.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "core/math/math_funcs.h"
#include "core/object/class_db.h"
#include "editor/editor_undo_redo_manager.h"
#include "modules/lamia_game_tools/eventer/ev.h"
#include "modules/lamia_game_tools/eventer/eventer.h"
#include "core/io/resource.h"
#include "core/templates/local_vector.h"
#include "core/variant/dictionary.h"
#include "modules/lamia_game_tools/eventer/sequence.h"

EventerPickerPanel::EventerPickerPanel()
{
    set_anchors_and_offsets_preset(PRESET_FULL_RECT);
    set_h_size_flags(SIZE_EXPAND_FILL);
    set_v_size_flags(SIZE_EXPAND_FILL);
    
    auto vbox = memnew(VBoxContainer);
    vbox->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
    vbox->set_h_size_flags(SIZE_EXPAND_FILL);
    vbox->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(vbox);

    tree = memnew(FilterTree);
    tree->set_h_size_flags(SIZE_EXPAND_FILL);
    tree->set_v_size_flags(SIZE_EXPAND_FILL);
    tree->set_hide_root(true);
    tree->connect(SNAME("button_clicked"), callable_mp(this, &EventerPickerPanel::_on_tree_item_button_clicked));

    filter_edit = memnew(FilterEdit);
    filter_edit->set_h_size_flags(SIZE_EXPAND_FILL);
    filter_edit->connect(SNAME("text_changed"), callable_mp(tree, &FilterTree::set_filter));

    vbox->add_child(filter_edit);
    vbox->add_child(tree);
}

void EventerPickerPanel::_bind_methods()
{
    ADD_SIGNAL(MethodInfo("command_added", PropertyInfo(Variant::OBJECT, "command", PROPERTY_HINT_RESOURCE_TYPE, "EVBase")));
}

void EventerPickerPanel::setup(Ref<EventerSequence> p_sequence)
{
    ERR_FAIL_NULL(p_sequence);

    auto theme = EditorNode::get_singleton()->get_editor_theme();
    auto add_icon = theme->get_icon("Add", EditorStringName(EditorIcons));

    AHashMap<StringName, TreeItem*> categories;

    PackedStringArray user_command_paths = EventerDB::get_script_command_paths();

    tree->clear();
    tree->create_item();

    // builtin classes, global classes, then paths
    // add a function that adds an item from a script

    for (const String &path : user_command_paths)
    {
        Ref<Script> command_script = ResourceLoader::load(path);
        if (command_script->is_abstract() or not command_script->is_tool())
        {
            continue;
        }

        StringName base_type = command_script->get_instance_base_type();
        if (base_type == StringName())
        {
            command_script->reload(true);
            base_type = command_script->get_instance_base_type();
        }

        Object *object = ClassDB::instantiate(base_type);
        if (not object->is_class(base_type))
        {
            memdelete(object);
            return;
        }

        Ref<EVBase> command_instance;
        command_instance.reference_ptr(Object::cast_to<EVBase>(object));
        command_instance->set_script(command_script);

        StringName category = command_instance->get_command_category();
        if (not category.is_empty())
        {
            if (not categories.has(category))
            {
                auto category_tree_item = tree->create_item();
                category_tree_item->set_text(0, category);
                categories.insert(category, category_tree_item);
            }

            auto category_tree_item = categories[category];
            auto command_tree_item = category_tree_item->create_child();
            command_tree_item->set_text(0, command_instance->get_command_text());
            command_tree_item->set_metadata(0, command_script);
            command_tree_item->add_button(0, add_icon);
            auto color = command_instance->get_command_color();
            if (not color.is_equal_approx(Color(1.0, 1.0, 1.0, 0.0)))
            {
                command_tree_item->set_custom_color(0, color);
            }
        }
        else
        {
            auto command_tree_item = tree->create_item();
            command_tree_item->set_text(0, command_instance->get_command_text());
            command_tree_item->set_metadata(0, command_script);
            command_tree_item->add_button(0, add_icon);
            auto color = command_instance->get_command_color();
            if (not color.is_equal_approx(Color(1.0, 1.0, 1.0, 0.0)))
            {
                command_tree_item->set_custom_color(0, color);
            }
        }
    }
}

void EventerPickerPanel::_on_tree_item_button_clicked(const TreeItem* p_item, const int &p_column, const bool &p_id, const int &p_mouse_button_index)
{
    if (not p_item) { return; }

    Ref<Script> script = p_item->get_metadata(0);
    if (script->is_valid())
    {
        StringName base_type = script->get_instance_base_type();
        Object *object = ClassDB::instantiate(base_type);

        Ref<EVBase> script_instance;
        script_instance.reference_ptr(Object::cast_to<EVBase>(object));
        script_instance->set_script(script);
        emit_signal(SNAME("command_added"), script_instance);
    }
}


EventerEditorTree::EventerEditorTree()
{
    set_anchors_preset(PRESET_FULL_RECT);
    set_h_size_flags(SIZE_EXPAND_FILL);
    set_v_size_flags(SIZE_EXPAND_FILL);

    hsplit = memnew(HSplitContainer);
    hsplit->set_anchors_preset(PRESET_FULL_RECT);
    hsplit->set_h_size_flags(SIZE_EXPAND_FILL);
    hsplit->set_v_size_flags(SIZE_EXPAND_FILL);
    // hsplit->set_split_offset(200, 0);
    hsplit->connect(SNAME("drag_ended"), callable_mp(this, &EventerEditorTree::_on_hsplit_drag_ended));
    add_child(hsplit);

    picker_panel = memnew(EventerPickerPanel);
    picker_panel->set_h_size_flags(SIZE_EXPAND_FILL);
    picker_panel->set_v_size_flags(SIZE_EXPAND_FILL);
    picker_panel->connect(SNAME("command_added"), callable_mp(this, &EventerEditorTree::_on_command_added));
    hsplit->add_child(picker_panel);

    tree = memnew(Tree);
    tree->set_h_size_flags(SIZE_EXPAND_FILL);
    tree->set_allow_rmb_select(true);
    tree->set_hide_root(true);
    tree->set_select_mode(Tree::SELECT_MULTI);
    tree->set_drag_forwarding(callable_mp(this, &EventerEditorTree::get_drag_data_fw), callable_mp(this, &EventerEditorTree::can_drop_data_fw), callable_mp(this, &EventerEditorTree::drop_data_fw));
    hsplit->add_child(tree);

    popup_menu = memnew(PopupMenu);
    add_child(popup_menu);
}

void EventerEditorTree::refresh()
{
    if (not sequence.is_valid()) { return; }

    sequence->emit_changed();
    tree->clear();
    tree->create_item();
    tree->get_root()->set_metadata(0, sequence->get_root());
    add_commands_from(sequence->get_root());
    if (sequence->get_path().is_empty() or sequence->get_path().contains("::"))
    {
        EditorInterface::get_singleton()->mark_scene_as_unsaved();
    }
    picker_panel->setup(sequence);
}

void EventerEditorTree::add_commands_from(Ref<EVBase> p_command, TreeItem *parent)
{
    auto theme = EditorNode::get_singleton()->get_editor_theme();
    auto icon = theme->get_icon(SNAME("Node"), EditorStringName(EditorIcons));
    auto missing_icon = theme->get_icon(SNAME("MissingNode"), EditorStringName(EditorIcons));

    for (const Ref<EVBase> child : p_command->get_children())
    {
        auto tree_item = tree->create_item(parent);
        tree_item->set_text(0, child->get_command_text());
        tree_item->set_suffix(0, child->get_command_suffix());
        if (child->is_enabled()) // and not has disabled ancestor
        {
            tree_item->set_icon(0, icon);
            if (child->get_command_color() != Color::get_named_color(Color::find_named_color("TRANSPARENT")))
            {
                tree_item->set_icon_modulate(0, child->get_command_color());
                tree_item->set_custom_color(0, child->get_command_color());
            }
        }
        else
        {
            tree_item->set_icon(0, missing_icon);
            tree_item->set_custom_color(0, Color::get_named_color(Color::find_named_color("WEB_GRAY")));
        }

        /* if (not child->get_configuration_warnings().is_empty())
        {

        } */

        tree_item->set_metadata(0, child);
        add_commands_from(child, tree_item);
    }
}

void EventerEditorTree::notification(int p_what)
{
    
}

void EventerEditorTree::_bind_methods()
{
    BIND(D_METHOD("_refresh"), &EventerEditorTree::refresh);
    BIND(D_METHOD("_copy_selected_commands"), &EventerEditorTree::copy_selected_commands);

    ADD_SIGNAL(MethodInfo("saved"));
    ADD_SIGNAL(MethodInfo("closed"));
    ADD_SIGNAL(MethodInfo("separation_changed", PropertyInfo(Variant::INT, "new_separation")));
}

void EventerEditorTree::setup(Ref<EventerSequence> p_sequence)
{
    ERR_FAIL_NULL(p_sequence);
    sequence = p_sequence;
    refresh();
}

void EventerEditorTree::save()
{
    if (not sequence.is_valid()) { return; }

    EditorNode::get_singleton()->save_resource(sequence);

    unsaved = false;
    emit_signal(SNAME("saved"));  
}

void EventerEditorTree::close()
{
    if (not sequence.is_valid()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    undo_redo->clear_history(undo_redo->get_history_id_for_object(sequence.ptr()));
    emit_signal(SNAME("closed"));
    queue_free();
}

void EventerEditorTree::set_separation(int p_separation)
{
    hsplit->set_split_offset(p_separation, 0);
}

int EventerEditorTree::get_separation() const
{
    return hsplit->get_split_offset(0);
}

void EventerEditorTree::show_popup_menu()
{
    popup_menu->clear();
}

Ref<EVBase> EventerEditorTree::get_root_command() const
{
    return tree->get_root()->get_metadata(0);
}

Ref<EVBase> EventerEditorTree::get_selected_command() const
{
    auto tree_selected = tree->get_selected();
    if (tree_selected)
    {
        Ref<EVBase> command_selected = tree_selected->get_metadata(0);
        if (command_selected.is_valid())
        {
            return command_selected;
        }
    }
    return nullptr;
}

LocalVector<Ref<EVBase>> EventerEditorTree::get_selected_commands() const
{
    LocalVector<Ref<EVBase>> ret;
    auto next = tree->get_next_selected(nullptr);
    while (next)
    {
        Ref<EVBase> command = next->get_metadata(0);
        if (command.is_valid())
        {
            ret.push_back(command);
        }
        next = tree->get_next_selected(next);
    }
    return ret;
}

void EventerEditorTree::cut_selected_commands()
{
    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Cut Command(s)");

    for (Ref<EVBase> command : selected_commands)
    {
        if (not command.is_valid()) { continue; }

        undo_redo->add_do_method(this, SNAME("_copy_selected_commands"));
        undo_redo->add_do_method(command->get_parent().ptr(), SNAME("_remove_child"), command);
        undo_redo->add_undo_method(command->get_parent().ptr(), SNAME("_add_child"), command);
        undo_redo->add_undo_method(command->get_parent().ptr(), SNAME("_move_child"), command, command->get_child_index());
    }

    undo_redo->add_do_method(this, SNAME("_refresh"));
    undo_redo->add_undo_method(this, SNAME("_refresh"));

    undo_redo->commit_action();
}

void EventerEditorTree::copy_selected_commands()
{
    clipboard.clear();

    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    for (Ref<EVBase> command : selected_commands)
    {
        if (not command.is_valid()) { continue; }
        clipboard.push_back(command);
    }
}

void EventerEditorTree::paste_commands()
{
    if (clipboard.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Paste Command(s)");

    for (Ref<EVBase> command : clipboard)
    {

    }

    undo_redo->add_do_method(this, SNAME("_refresh"));
    undo_redo->add_undo_method(this, SNAME("_refresh"));

    undo_redo->commit_action();
}

void EventerEditorTree::duplicate_selected_commands()
{
    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Duplicate Command(s)");

    for (Ref<EVBase> command : selected_commands)
    {

    }

    undo_redo->add_do_method(this, SNAME("_refresh"));
    undo_redo->add_undo_method(this, SNAME("_refresh"));

    undo_redo->commit_action();
}

void EventerEditorTree::delete_selected_commands()
{
    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Delete Command(s)");

    for (Ref<EVBase> command : selected_commands)
    {
        if (not command.is_valid()) { continue; }

        undo_redo->add_do_method(command->get_parent().ptr(), SNAME("_remove_child"), command);
        undo_redo->add_undo_method(command->get_parent().ptr(), SNAME("_add_child"), command);
        undo_redo->add_undo_method(command->get_parent().ptr(), SNAME("_move_child"), command, command->get_child_index());
    }

    undo_redo->add_do_method(this, SNAME("_refresh"));
    undo_redo->add_undo_method(this, SNAME("_refresh"));

    undo_redo->commit_action();
}

void EventerEditorTree::gui_input_fw(const Ref<InputEvent> &p_event)
{
    Ref<InputEventKey> event = p_event;
    if (event.is_valid())
    {
        if (event->is_pressed() and not event->is_echo() and event->is_command_or_control_pressed())
        {
            switch (event->get_keycode())
            {
				case Key::X:
                    cut_selected_commands();
                    accept_event();
					break;
                case Key::C:
                    copy_selected_commands();
                    accept_event();
                    break;
                case Key::V:
                    paste_commands();
                    accept_event();
                    break;
                case Key::D:
                    duplicate_selected_commands();
                    accept_event();
                    break;
                default:
                    break;
			}
		}
        else
        {
            if (event->get_keycode() == Key::KEY_DELETE)
            {
                delete_selected_commands();
                accept_event();
            }
        }
    }
}

void EventerEditorTree::get_drag_data_fw(const Point2 &at_position) const
{
    
}

void EventerEditorTree::can_drop_data_fw(const Point2 &at_position, const Variant &p_data)
{
    
}

void EventerEditorTree::drop_data_fw(const Point2 &at_position, const Variant &p_data)
{
    
}

void EventerEditorTree::_on_command_added(Ref<EVBase> p_command)
{
    ERR_FAIL_NULL(p_command);
    // GDScript version had an issue with randomization where it'd reuse uids here, could have been overlap with the resource's past ids

    auto selected_command = get_selected_command();

    Math::randomize();
    auto uid = Resource::generate_scene_unique_id();
    p_command->set_path(vformat("%s::Resource_%s", p_command->get_path(), uid));
    p_command->emit_changed();

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Add Command");

    if (selected_command.is_valid())
    {
        if (selected_command->get_child_count() > 0)
        {
            undo_redo->add_do_method(selected_command.ptr(), SNAME("_add_child"), p_command);
            undo_redo->add_undo_method(selected_command.ptr(), SNAME("_remove_child"), p_command);
        }
        else
        {
            undo_redo->add_do_method(selected_command->get_parent().ptr(), SNAME("_add_child"), p_command);
            undo_redo->add_do_method(selected_command->get_parent().ptr(), SNAME("_move_child"), selected_command->get_child_index() + 1);
            undo_redo->add_undo_method(selected_command->get_parent().ptr(), SNAME("_remove_child"), p_command);
        }
    }
    else
    {
        undo_redo->add_do_method(get_root_command().ptr(), SNAME("_add_child"), p_command);
        undo_redo->add_undo_method(get_root_command().ptr(), SNAME("_remove_child"), p_command);
    }

    undo_redo->add_do_method(this, SNAME("_refresh"));
    undo_redo->add_undo_method(this, SNAME("_refresh"));

    undo_redo->commit_action();
}

void EventerEditorTree::_on_hsplit_drag_ended()
{
    emit_signal(SNAME("separation_changed"), hsplit->get_split_offset(0));
}



EventerEditor::EventerEditor()
{
    set_h_size_flags(SIZE_EXPAND_FILL);
    set_v_size_flags(SIZE_EXPAND_FILL);
    set_anchors_and_offsets_preset(PRESET_FULL_RECT);

    open_file_panel = memnew(PanelContainer);
    open_file_panel->set_anchors_preset(PRESET_FULL_RECT);
    add_child(open_file_panel);

    open_file_label = memnew(Label);
    open_file_label->set_text("open a file, forehead");
    open_file_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    open_file_label->set_h_size_flags(SIZE_FILL);
    open_file_label->set_v_size_flags(SIZE_SHRINK_CENTER);
    open_file_panel->add_child(open_file_label);

    tabs = memnew(TabContainer);
    tabs->set_h_size_flags(SIZE_FILL);
    tabs->set_v_size_flags(SIZE_SHRINK_CENTER);
    tabs->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
    tabs->hide();
    add_child(tabs);
}

void EventerEditor::show_editor()
{
    open_file_panel->hide();
    tabs->show();
}

void EventerEditor::hide_editor()
{
    open_file_panel->show();
    tabs->hide();
}

void EventerEditor::open_sequence(Ref<EventerSequence> p_sequence)
{
    ERR_FAIL_NULL(p_sequence);
    int i = 0;
    bool already_open = false;
    for (Variant &child : get_children())
    {
        EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(child);
        if (not tab) { continue; }
        if (tab->get_sequence().ptr() == p_sequence.ptr())
        {
            already_open = true;
            break;
        }
        i++;
    }

    if (not already_open)
    {
        auto tab = memnew(EventerEditorTree);
        if (not p_sequence->get_path().is_empty())
        {
            String title = p_sequence->get_path().get_file();
            tab->set_name(title.get_slice(vformat(".%s", p_sequence->get_path().get_extension()), 0));
        }
        else
        {
            tab->set_name("<Unsaved>");
        }

        tab->setup(p_sequence);
        tab->set_separation(separation);
        tab->connect(SNAME("closed"), callable_mp(this, &EventerEditor::_on_tab_closed), CONNECT_ONE_SHOT);
        tab->connect(SNAME("separation_changed"), callable_mp(this, &EventerEditor::set_separation));

        tabs->add_child(tab);
        tabs->set_tab_button_icon(tabs->get_child_count() - 2, EditorNode::get_singleton()->get_editor_theme()->get_icon(SNAME("Close"), EditorStringName(EditorIcons)));

        show_editor();
    }
    else
    {
        tabs->set_current_tab(i);
    }
}

void EventerEditor::save_current()
{
    if (tabs->get_child_count() > 0)
    {
        EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(tabs->get_current_tab_control());
        if (tab)
        {
            tab->save();
        }
    }
}

void EventerEditor::save_all()
{
    for (Variant &child : get_children())
    {
        EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(child);
        if (tab)
        {
            tab->save();
        }
    }
}

void EventerEditor::close_all()
{
    for (Variant &child : get_children())
    {
        EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(child);
        if (tab)
        {
            tab->close();
        }
    }
}

void EventerEditor::set_separation(int p_separation)
{
    separation = p_separation;
    for (Variant &child : get_children())
    {
        EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(child);
        if (tab)
        {
            tab->set_separation(separation);
        }
    }
}

int EventerEditor::get_separation() const
{
    return separation;
}

PackedStringArray EventerEditor::get_open_tab_paths() const
{
    PackedStringArray ret;

    if (tabs->get_child_count() > 0)
    {
        EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(tabs->get_child(0));
        if (tab)
        {
            if (not tab->get_sequence()->get_path().is_empty())
            {
                ret.append(tab->get_sequence()->get_path());
            }
        }
    }

    return ret;
}

void EventerEditor::_on_tab_closed()
{
    if (tabs->get_child_count() - 1 == 0)
    {
        hide_editor();
    }
}



const Ref<Texture2D> EditorPluginEventer::get_plugin_icon() const 
{
    Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();
    if (theme.is_valid())
    {
        return theme->get_icon("EVBase", EditorStringName(EditorIcons));
    }
    return nullptr;
}

void EditorPluginEventer::edit(Object *p_object)
{
    Ref<EventerSequence> sequence = Object::cast_to<EventerSequence>(p_object);
    if (sequence.is_valid())
    {
        editor->open_sequence(sequence);
    }
}

void EditorPluginEventer::make_visible(bool p_visible)
{
    editor->set_visible(p_visible);
}

void EditorPluginEventer::save_external_data()
{
    editor->save_all();
}

void EditorPluginEventer::set_window_layout(Ref<ConfigFile> p_layout)
{
    if (p_layout->has_section("Eventer"))
    {
        editor->set_separation(p_layout->get_value("Eventer", "separation"));
    }
}

void EditorPluginEventer::get_window_layout(Ref<ConfigFile> p_layout)
{
    p_layout->set_value("Eventer", "separation", editor->get_separation());
}

void EditorPluginEventer::set_state(const Dictionary &p_state)
{
    PackedStringArray paths = p_state.get("open_file_paths", PackedStringArray());
    if (not paths.is_empty())
    {
        for (const String &path : paths)
        {
            if (ResourceLoader::exists(path))
            {
                Ref<EventerSequence> sequence = ResourceLoader::load(path);
                if (sequence.is_valid())
                {
                    editor->open_sequence(sequence);
                }
            }
        }
    }
}

Dictionary EditorPluginEventer::get_state() const 
{
    Dictionary ret;
    ret.set("open_file_paths", editor->get_open_tab_paths());
    return ret;
}

EditorPluginEventer::EditorPluginEventer()
{
    GLOBAL_DEF_BASIC(PropertyInfo(Variant::PACKED_STRING_ARRAY, "eventer/commands/command_directories", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:", Variant::STRING, PROPERTY_HINT_DIR)), PackedStringArray());
    editor = memnew(EventerEditor);
    editor->hide();
    EditorInterface::get_singleton()->get_editor_main_screen()->add_child(editor);
    EventerDB::scan_script_command_paths();
}

#endif
