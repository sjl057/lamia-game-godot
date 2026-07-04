#ifdef TOOLS_ENABLED

#include "core/math/vector2.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "scene/gui/tab_bar.h"
#include "scene/main/canvas_item.h"
#include "core/math/rect2i.h"
#include "core/math/vector2i.h"
#include "editor/editor_string_names.h"
#include "modules/lamia_game_tools/eventer/builtin_commands.h"
#include "modules/lamia_game_tools/eventer/eventer_db.h"
#include "modules/lamia_game_tools/general/filter_edit.h"
#include "modules/lamia_game_tools/general/filter_tree.h"
#include "scene/gui/box_container.h"
#include "servers/display/display_server.h"
#include "core/input/input_event.h"
#include "core/math/math_defs.h"
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
#include "editor/editor_main_screen.h"
#include "scene/resources/theme.h"
#include "editor_plugin.h"
#include "core/object/object.h"
#include "editor/editor_node.h"
#include "scene/gui/popup_menu.h"
#include "core/object/callable_method_pointer.h"
#include "modules/lamia_game_tools/general/defs.h"
#include "core/math/math_funcs.h"
#include "core/object/class_db.h"
#include "editor/editor_undo_redo_manager.h"
#include "core/io/resource.h"
#include "core/templates/local_vector.h"
#include "core/variant/dictionary.h"
#include "modules/lamia_game_tools/eventer/sequence.h"
#include "core/input/input_enums.h"
#include "core/string/print_string.h"

EventerAddCommandTree::EventerAddCommandTree()
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
    tree->connect("button_clicked", callable_mp(this, &EventerAddCommandTree::_on_tree_item_button_clicked));

    filter_edit = memnew(FilterEdit);
    filter_edit->set_h_size_flags(SIZE_EXPAND_FILL);
    filter_edit->connect("text_changed", callable_mp(tree, &FilterTree::set_filter));

    vbox->add_child(filter_edit);
    vbox->add_child(tree);
}

void EventerAddCommandTree::_bind_methods()
{
    ADD_SIGNAL(MethodInfo("command_added", PropertyInfo(Variant::OBJECT, "command", PROPERTY_HINT_RESOURCE_TYPE, "EVBase")));
}

void EventerAddCommandTree::setup(Ref<EventerSequence> p_sequence)
{
    ERR_FAIL_NULL(p_sequence);

    auto theme = EditorNode::get_singleton()->get_editor_theme();
    // auto icon = theme->get_icon("EVBase", EditorStringName(EditorIcons));
    auto add_icon = theme->get_icon("Add", EditorStringName(EditorIcons));

    AHashMap<StringName, TreeItem*> categories;

    tree->clear();
    tree->create_item();

    PackedStringArray commands = EventerDB::get_commands();
    // global classes

    for (const String &command : commands)
    {
        Ref<EVCommand> command_instance = EventerDB::get_command_instance(command);
        if (not command_instance.is_valid() or not command_instance->show_in_add_tree())
        {
            continue;
        }

        StringName category = command_instance->get_command_category();
        TreeItem *category_parent = nullptr;
        if (not category.is_empty())
        {
            if (not categories.has(category))
            {
                category_parent = tree->create_item();
                category_parent->set_text(0, category);
                categories.insert(category, category_parent);
            }
            else
            {
                category_parent = categories[category];
            }
        }

        TreeItem *command_tree_item = nullptr;
        
        if (category_parent)
        {
            command_tree_item = category_parent->create_child();
        }
        else
        {
            command_tree_item = tree->create_item();
        }

        command_tree_item->set_text(0, command_instance->get_command_text());
        command_tree_item->set_metadata(0, command_instance);
        command_tree_item->add_button(0, add_icon);
        command_tree_item->set_icon(0, command_instance->get_command_icon());
        auto color = command_instance->get_command_color();
        if (not color.is_equal_approx(Color(1.0, 1.0, 1.0, 0.0)))
        {
            command_tree_item->set_custom_color(0, color);
        }
    }
}

void EventerAddCommandTree::_on_tree_item_button_clicked(const TreeItem* p_item, const int &p_column, const bool &p_id, const int &p_mouse_button_index)
{
    if (not p_item) { return; }

    Ref<EVCommand> command = p_item->get_metadata(0);
    if (command.is_valid())
    {
        emit_signal("command_added", command->duplicate(true));
    }
}



EventerEditorTree::EventerEditorTree()
{
    set_anchors_preset(PRESET_FULL_RECT);
    set_h_size_flags(SIZE_EXPAND_FILL);
    set_v_size_flags(SIZE_EXPAND_FILL);
    set_process_shortcut_input(true);

    hsplit = memnew(HSplitContainer);
    hsplit->set_anchors_preset(PRESET_FULL_RECT);
    hsplit->set_h_size_flags(SIZE_EXPAND_FILL);
    hsplit->set_v_size_flags(SIZE_EXPAND_FILL);
    hsplit->connect("drag_ended", callable_mp(this, &EventerEditorTree::_on_hsplit_drag_ended));
    add_child(hsplit);

    add_command_tree = memnew(EventerAddCommandTree);
    add_command_tree->set_h_size_flags(SIZE_EXPAND_FILL);
    add_command_tree->set_v_size_flags(SIZE_EXPAND_FILL);
    add_command_tree->connect("command_added", callable_mp(this, &EventerEditorTree::_on_command_added));
    hsplit->add_child(add_command_tree);

    tree = memnew(Tree);
    tree->set_h_size_flags(SIZE_EXPAND_FILL);
    tree->set_allow_rmb_select(true);
    tree->set_hide_root(true);
    tree->set_select_mode(Tree::SELECT_MULTI);
    tree->set_drag_forwarding(callable_mp(this, &EventerEditorTree::get_drag_data_fw), callable_mp(this, &EventerEditorTree::can_drop_data_fw), callable_mp(this, &EventerEditorTree::drop_data_fw));
    tree->connect("item_activated", callable_mp(this, &EventerEditorTree::_on_item_activated));
    tree->connect("item_mouse_selected", callable_mp(this, &EventerEditorTree::_on_item_mouse_selected));
    tree->connect("nothing_selected", callable_mp(tree, &Tree::deselect_all));
    hsplit->add_child(tree);

    popup_menu = memnew(PopupMenu);
    popup_menu->connect("id_pressed", callable_mp(this, &EventerEditorTree::_on_popup_menu_id_pressed));
    add_child(popup_menu);
}

void EventerEditorTree::_notification(int p_what)
{
    if (p_what == NOTIFICATION_VISIBILITY_CHANGED)
    {
        if (is_visible_in_tree())
        {
            refresh();
        }
    }
}

void EventerEditorTree::_bind_methods()
{
    BIND(D_METHOD("_refresh"), &EventerEditorTree::refresh);
    BIND(D_METHOD("_copy_selected_commands"), &EventerEditorTree::copy_selected_commands);

    ADD_SIGNAL(MethodInfo("saved"));
    ADD_SIGNAL(MethodInfo("closed"));
    ADD_SIGNAL(MethodInfo("separation_changed", PropertyInfo(Variant::INT, "new_separation")));

    ADD_SIGNAL(MethodInfo("copy_requested", PropertyInfo(Variant::ARRAY, "copied_commands")));
    ADD_SIGNAL(MethodInfo("paste_requested", PropertyInfo(Variant::OBJECT, "paste_target", PROPERTY_HINT_RESOURCE_TYPE, "EVBase")));
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
    add_command_tree->setup(sequence);
}

void EventerEditorTree::add_commands_from(Ref<EVBase> p_command, TreeItem *parent)
{
    auto theme = EditorNode::get_singleton()->get_editor_theme();
    auto missing_icon = theme->get_icon(SNAME("MissingNode"), EditorStringName(EditorIcons));

    for (const Ref<EVBase> child : p_command->get_children())
    {
        auto tree_item = tree->create_item(parent);
        tree_item->set_text(0, child->get_command_text());
        tree_item->set_suffix(0, child->get_command_suffix());
        if (child->is_enabled()) // and not has disabled ancestor
        {
            tree_item->set_icon(0, child->get_command_icon());
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

        if (not child->get_configuration_warnings().is_empty())
        {

        }

        tree_item->set_metadata(0, child);
        add_commands_from(child, tree_item);
    }
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
    emit_signal("saved");  
}

void EventerEditorTree::close()
{
    if (sequence.is_valid())
    {
        // save();
        auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
        undo_redo->clear_history(undo_redo->get_history_id_for_object(this));
    }

    emit_signal("closed");
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
    Ref<EVBase> command = get_selected_command();
    if (not command.is_valid()) { return; }
    
    Ref<Theme> theme = EditorNode::get_singleton()->get_editor_theme();

    popup_menu->clear();
    popup_menu->add_icon_item(theme->get_icon("Script", EditorStringName(EditorIcons)), "Edit Script", ACTION_EDIT_SCRIPT);
    popup_menu->add_separator();
    popup_menu->add_icon_item(theme->get_icon("ActionCut", EditorStringName(EditorIcons)), "Cut", ACTION_CUT);
    popup_menu->add_icon_item(theme->get_icon("ActionCopy", EditorStringName(EditorIcons)), "Copy", ACTION_COPY);
    popup_menu->add_icon_item(theme->get_icon("ActionPaste", EditorStringName(EditorIcons)), "Paste", ACTION_PASTE);
    popup_menu->add_icon_item(theme->get_icon("Duplicate", EditorStringName(EditorIcons)), "Duplicate", ACTION_DUPLICATE);
    popup_menu->add_separator();
    popup_menu->add_icon_item(theme->get_icon("Remove", EditorStringName(EditorIcons)), "Delete", ACTION_DELETE);
    
    if (not command->get_edit_flags().has_flag(EVBase::EDIT_FLAGS_CAN_MODIFY))
    {
        // popup_menu->set_item_disabled(1, true);
        popup_menu->set_item_disabled(2, true);
        popup_menu->set_item_disabled(3, true);
        popup_menu->set_item_disabled(4, true);
        popup_menu->set_item_disabled(5, true);
        popup_menu->set_item_disabled(7, true);
    }

    popup_menu->popup(Rect2i(DisplayServer::get_singleton()->mouse_get_position(), Size2i(90, 180)));
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

Ref<EVBase> EventerEditorTree::get_command_at_position(const Point2 &at_position) const
{
    auto item = tree->get_item_at_position(at_position);
    if (item)
    {
        Ref<EVBase> command = item->get_metadata(0);
        if (command.is_valid())
        {
            return command;
        }
    }
    return nullptr;
}

void EventerEditorTree::cut_selected_commands()
{
    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Cut Command(s)");

    for (Ref<EVCommand> command : selected_commands)
    {
        if (not command.is_valid()) { continue; }
        if (not command->get_edit_flags().has_flag(EVBase::EDIT_FLAGS_CAN_MODIFY)) { continue; }

        undo_redo->add_do_method(this, "_copy_selected_commands");
        undo_redo->add_do_method(command->get_parent().ptr(), "_remove_child", command);
        undo_redo->add_undo_method(command->get_parent().ptr(), "_add_child", command);
        undo_redo->add_undo_method(command->get_parent().ptr(), "_move_child", command, command->get_child_index());
    }

    undo_redo->add_do_method(this, "_refresh");
    undo_redo->add_undo_method(this, "_refresh");

    undo_redo->commit_action();
}

void EventerEditorTree::copy_selected_commands()
{
    Array copied_commands;

    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    for (const Ref<EVBase> &command : selected_commands)
    {
        if (not command.is_valid()) { continue; }
        if (not command->get_edit_flags().has_flag(EVBase::EDIT_FLAGS_CAN_MODIFY)) { continue; }

        copied_commands.push_back(command->duplicate_deep());
    }

    emit_signal("copy_requested", copied_commands);
}

void EventerEditorTree::paste_commands()
{
    Ref<EVBase> paste_to = get_root_command();
    if (get_selected_command().is_valid())
    {
        paste_to = get_selected_command();
    }
    emit_signal("paste_requested", paste_to);
}

void EventerEditorTree::duplicate_selected_commands()
{
    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Duplicate Command(s)");

    Ref<EVBase> duplicate_to = get_root_command();
    if (get_selected_command().is_valid())
    {
        duplicate_to = get_selected_command();
    }

    for (const Ref<EVBase> &command : selected_commands)
    {
        if (not command->get_edit_flags().has_flag(EVBase::EDIT_FLAGS_CAN_MODIFY)) { continue; }

        auto duplicate = command->duplicate_deep();
        undo_redo->add_do_method(duplicate_to.ptr(), "_add_child", duplicate);
        undo_redo->add_undo_method(duplicate_to.ptr(), "_remove_child", duplicate);
    }

    undo_redo->add_do_method(this, "_refresh");
    undo_redo->add_undo_method(this, "_refresh");

    undo_redo->commit_action();
}

void EventerEditorTree::delete_selected_commands()
{
    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Delete Command(s)");

    for (Ref<EVCommand> command : selected_commands)
    {
        if (not command.is_valid()) { continue; }
        if (command->is_root()) { continue; } // shouldn't be possible
        if (not command->get_edit_flags().has_flag(EVBase::EDIT_FLAGS_CAN_MODIFY)) { continue; }

        Ref<EVCommand> command_parent = command->get_parent();
        if (not command_parent.is_valid())
        {
            ERR_PRINT("Command doesn't have a parent, this shouldn't be possible");
            continue;
        } 

        undo_redo->add_do_method(command_parent.ptr(), "_remove_child", command);
        undo_redo->add_undo_method(command_parent.ptr(), "_add_child", command);
        undo_redo->add_undo_method(command_parent.ptr(), "_move_child", command, command->get_child_index());
    }

    undo_redo->add_do_method(this, "_refresh");
    undo_redo->add_undo_method(this, "_refresh");

    undo_redo->commit_action();
}

void EventerEditorTree::edit_selected_command_script()
{
    auto selected_command = get_selected_command();
    if (not selected_command.is_valid()) { return; }

    if (selected_command->get_script())
    {
        EditorInterface::get_singleton()->edit_resource(selected_command->get_script());
    }
    else
    {
        print_line("Command doesn't have a script, can't open");
    }
}

void EventerEditorTree::shortcut_input(const Ref<InputEvent> &p_event)
{
    Ref<InputEventKey> event = p_event;
    if (event.is_valid())
    {
        if (event->is_pressed() and not event->is_echo())
        {
            if (event->is_action("ui_cut"))
            {
                cut_selected_commands();
                accept_event();
            }
            if (event->is_action("ui_copy"))
            {
                copy_selected_commands();
                accept_event();
            }
            if (event->is_action("ui_paste"))
            {
                paste_commands();
                accept_event();
            }
            if (event->is_action("ui_graph_duplicate"))
            {
                duplicate_selected_commands();
                accept_event();
            }
            if (event->is_action("ui_text_delete"))
            {
                delete_selected_commands();
                accept_event();
            }
		}
    }
}

TypedArray<Ref<EVBase>> EventerEditorTree::normalize_drag_data(const Variant &p_data) const
{
    TypedArray<Ref<EVBase>> ret;

    // TODO drag and drop scripts because why not

    Dictionary drag_data = p_data;
    if (drag_data.is_empty()) { return ret; }

    if (drag_data.has("commands")) // dragging from EventerEditorTree
    {
        ret.append_array(drag_data.get("commands", Variant()));
    }

    return ret;
}

Variant EventerEditorTree::get_drag_data_fw(const Point2 &at_position) const
{
    auto selected_commands = get_selected_commands();
    if (selected_commands.is_empty()) { return Variant(); }

    if (not tree->get_item_at_position(at_position)) { return Variant(); }

    auto preview_vbox = memnew(VBoxContainer);

    Dictionary ret;
    TypedArray<Ref<EVBase>> commands;
    for (const Ref<EVBase> &command : selected_commands)
    {
        if (not command->get_edit_flags().has_flag(EVBase::EDIT_FLAGS_CAN_MOVE)) { continue; }

        commands.push_back(command);

        auto label = memnew(Label);
        label->set_text(command->get_command_text());
        preview_vbox->add_child(label);
    }

    ret["commands"] = commands;

    tree->set_drag_preview(preview_vbox);
    
    return ret;
}

bool EventerEditorTree::can_drop_data_fw(const Point2 &at_position, const Variant &p_data)
{
    auto commands = normalize_drag_data(p_data);
    if (commands.is_empty()) { return false; }

    tree->set_drop_mode_flags(Tree::DROP_MODE_INBETWEEN | Tree::DROP_MODE_ON_ITEM);

    Ref<EVBase> target_command = get_command_at_position(at_position);
    if (target_command.is_valid() and not commands.has(target_command))
    {
        Ref<EVBase> target_command_parent = target_command->get_parent();

        if (target_command_parent.is_valid())
        {
            switch (tree->get_drop_section_at_position(at_position))
            {
                case 0:
                    return target_command->can_add_children();
                case -1:
                case 1:
                    return target_command_parent->can_add_children();
                default:
                    return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        // drop to root
        return get_root_command()->can_add_children();
    }

    return false;
}

void EventerEditorTree::drop_data_fw(const Point2 &at_position, const Variant &p_data)
{
    auto commands = normalize_drag_data(p_data);

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Reparent Command(s)");

    Ref<EVBase> target_command = get_command_at_position(at_position);
    if (target_command.is_valid() and not commands.has(target_command))
    {
        switch (tree->get_drop_section_at_position(at_position))
        {
            case -1: // parent to parent of target command, above
                for (Variant &dragged_command_i : commands)
                {
                    Ref<EVCommand> dragged_command = dragged_command_i;
                    if (not dragged_command.is_valid()) { continue; }

                    if (target_command->get_parent() != dragged_command->get_parent())
                    {
                        undo_redo->add_do_method(target_command->get_parent().ptr(), "_add_child", dragged_command);
                    }

                    undo_redo->add_do_method(target_command->get_parent().ptr(), "_move_child", dragged_command, target_command->get_child_index() - 1);
                    if (dragged_command->get_parent().is_valid())
                    {
                        undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_add_child", dragged_command);
                        undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_move_child", dragged_command, dragged_command->get_child_index());
                    }
                    else
                    {
                        undo_redo->add_undo_method(target_command->get_parent().ptr(), "_remove_child", dragged_command);
                    }
                }
                break;

            case 0: // parent to target command
                for (Variant &dragged_command_i : commands)
                {
                    Ref<EVCommand> dragged_command = dragged_command_i;
                    if (not dragged_command.is_valid()) { continue; }

                    undo_redo->add_do_method(target_command.ptr(), "_add_child", dragged_command);

                    if (dragged_command->get_parent().is_valid())
                    {
                        undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_add_child", dragged_command);
                    }
                    else
                    {
                        undo_redo->add_undo_method(target_command.ptr(), "_remove_child", dragged_command);
                    }
                }
                break;

            case 1: // parent to parent of target command, below
                for (Variant &dragged_command_i : commands)
                {
                    Ref<EVCommand> dragged_command = dragged_command_i;
                    if (not dragged_command.is_valid()) { continue; }

                    // if dragged below a parentable object, add it to its children at idx 0
                    if (target_command->can_add_children())
                    {
                        undo_redo->add_do_method(target_command.ptr(), "_add_child", dragged_command);
                        undo_redo->add_do_method(target_command.ptr(), "_move_child", dragged_command, 0);
                        if (dragged_command->get_parent().is_valid())
                        {
                            undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_add_child", dragged_command);
                            undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_move_child", dragged_command, dragged_command->get_child_index());
                        }
                        else
                        {
                            undo_redo->add_undo_method(target_command.ptr(), "_remove_child", dragged_command);
                        }
                    }
                    else
                    {
                        if (target_command->get_parent() != dragged_command->get_parent())
                        {
                            undo_redo->add_do_method(target_command->get_parent().ptr(), "_add_child", dragged_command);
                        }

                        undo_redo->add_do_method(target_command->get_parent().ptr(), "_move_child", dragged_command, target_command->get_child_index());
                        if (dragged_command->get_parent().is_valid())
                        {
                            undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_add_child", dragged_command);
                            undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_move_child", dragged_command, dragged_command->get_child_index());
                        }
                        else
                        {
                            undo_redo->add_undo_method(target_command->get_parent().ptr(), "_remove_child", dragged_command);
                        }
                    }
                }
                break;
        }
    }
    else
    {
        Ref<EVCommand> root_command = get_root_command();
        for (Variant &dragged_command_i : commands)
        {
            Ref<EVCommand> dragged_command = dragged_command_i;
            if (not dragged_command.is_valid()) { continue; }

            undo_redo->add_do_method(root_command.ptr(), "_add_child", dragged_command);

            if (dragged_command->get_parent().is_valid())
            {
                undo_redo->add_undo_method(dragged_command->get_parent().ptr(), "_add_child", dragged_command);
            }
            else
            {
                undo_redo->add_undo_method(root_command.ptr(), "_remove_child", dragged_command);
            }
        }
    }

    undo_redo->add_do_method(this, "_refresh");
    undo_redo->add_undo_method(this, "_refresh");

    undo_redo->commit_action();
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
        if (selected_command->can_add_children())
        {
            undo_redo->add_do_method(selected_command.ptr(), "_add_child", p_command);
            undo_redo->add_undo_method(selected_command.ptr(), "_remove_child", p_command);
        }
        else
        {
            undo_redo->add_do_method(selected_command->get_parent().ptr(), "_add_child", p_command);
            undo_redo->add_do_method(selected_command->get_parent().ptr(), "_move_child", p_command, selected_command->get_child_index() + 1);
            undo_redo->add_undo_method(selected_command->get_parent().ptr(), "_remove_child", p_command);
        }
    }
    else
    {
        undo_redo->add_do_method(get_root_command().ptr(), "_add_child", p_command);
        undo_redo->add_undo_method(get_root_command().ptr(), "_remove_child", p_command);
    }

    undo_redo->add_do_method(this, "_refresh");
    undo_redo->add_undo_method(this, "_refresh");

    undo_redo->commit_action();
}

void EventerEditorTree::_on_popup_menu_id_pressed(const int &p_id)
{
    switch (static_cast<MenuAction>(p_id))
    {
		case ACTION_CUT:
            cut_selected_commands();
            break;
		case ACTION_COPY:
            copy_selected_commands();
            break;
		case ACTION_PASTE:
            paste_commands();
            break;
		case ACTION_DUPLICATE:
            duplicate_selected_commands();
            break;
		case ACTION_EDIT_SCRIPT:
            edit_selected_command_script();
            break;
		case ACTION_DELETE:
            delete_selected_commands();
			break;
	}
}

void EventerEditorTree::_on_item_activated()
{
    auto selected_command = get_selected_command();
    if (not selected_command.is_valid()) { return; }
    if (not selected_command->get_edit_flags().has_flag(EVBase::EDIT_FLAGS_CAN_MODIFY)) { return; }

    EditorInterface::get_singleton()->edit_resource(selected_command);
}

void EventerEditorTree::_on_item_mouse_selected(const Vector2 &p_mouse_position, const MouseButton &p_mouse_button_index)
{
    if (p_mouse_button_index == MouseButton::RIGHT)
    {
        show_popup_menu();
    }
}

void EventerEditorTree::_on_hsplit_drag_ended()
{
    emit_signal("separation_changed", hsplit->get_split_offset(0));
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
    tabs->connect("tab_button_pressed", callable_mp(this, &EventerEditor::_on_tab_button_pressed));
    tabs->get_tab_bar()->set_close_with_middle_mouse(true);
    tabs->get_tab_bar()->connect("tab_close_pressed", callable_mp(tabs->get_tab_bar(), &TabBar::remove_tab));
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
    for (Variant &child : tabs->get_children())
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
        tab->set_focus_mode(FOCUS_CLICK);
        tab->set_separation(separation);
        tab->connect("closed", callable_mp(this, &EventerEditor::_on_tab_closed), CONNECT_ONE_SHOT);
        tab->connect("separation_changed", callable_mp(this, &EventerEditor::set_separation));
        tab->connect("copy_requested", callable_mp(this, &EventerEditor::copy_commands));
        tab->connect("paste_requested", callable_mp(this, &EventerEditor::paste_commands), CONNECT_APPEND_SOURCE_OBJECT);

        tabs->add_child(tab);
        tabs->set_tab_button_icon(tabs->get_child_count() - 2, EditorNode::get_singleton()->get_editor_theme()->get_icon(SNAME("Close"), EditorStringName(EditorIcons)));

        show_editor();
    }
    else
    {
        tabs->set_current_tab(i);
    }
}

void EventerEditor::copy_commands(Array p_commands)
{
    clipboard.clear();

    for (const Variant &command : p_commands)
    {
        Ref<EVBase> command_inst = command;
        if (command_inst.is_valid())
        {
            clipboard.push_back(command);
        }
    }
}

void EventerEditor::paste_commands(Ref<EVBase> p_paste_target, EventerEditorTree *p_tree)
{
    ERR_FAIL_COND_MSG(not p_tree, "No tree to paste to");
    ERR_FAIL_NULL(p_paste_target);
    if (clipboard.is_empty()) { return; }

    auto undo_redo = EditorInterface::get_singleton()->get_editor_undo_redo();
    if (undo_redo->is_committing_action()) { return; }
    undo_redo->create_action("Paste Command(s)");

    for (Ref<EVBase> &command : clipboard)
    {
        if (not command.is_valid()) { continue; }
        
        if (p_paste_target->can_add_children())
        {
            undo_redo->add_do_method(p_paste_target.ptr(), "_add_child", command);
            undo_redo->add_undo_method(p_paste_target.ptr(), "_remove_child", command);
        }
        else
        {
            Ref<EVBase> valid_parent = p_paste_target->get_parent();
            while (valid_parent->get_parent().is_valid() and not valid_parent->can_add_children())
            {
                valid_parent = valid_parent->get_parent();
            }

            if (valid_parent.is_valid())
            {
                undo_redo->add_do_method(valid_parent.ptr(), "_add_child", command);
                undo_redo->add_undo_method(valid_parent.ptr(), "_remove_child", command);
            }
            else
            {
                undo_redo->add_do_method(p_tree->get_root_command().ptr(), "_add_child", command);
                undo_redo->add_undo_method(p_tree->get_root_command().ptr(), "_remove_child", command);
            }
        }
    }

    undo_redo->add_do_method(p_tree, "_refresh");
    undo_redo->add_undo_method(p_tree, "_refresh");

    undo_redo->commit_action();
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

void EventerEditor::close_tab(const int &p_tab)
{
    ERR_FAIL_INDEX(p_tab, tabs->get_child_count());

    EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(tabs->get_child(p_tab));
    if (tab)
    {
        tab->close();
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
        for (const Variant &tab : tabs->get_children())
        {
            EventerEditorTree *tab_inst = Object::cast_to<EventerEditorTree>(tab);
            if (tab_inst)
            {
                if (not tab_inst->get_sequence()->get_path().is_empty())
                {
                    ret.append(tab_inst->get_sequence()->get_path());
                }
            }
        }
    }

    return ret;
}

void EventerEditor::_on_tab_button_pressed(const int &p_tab)
{
    EventerEditorTree *tab = Object::cast_to<EventerEditorTree>(tabs->get_tab_control(p_tab));
    if (tab)
    {
        // TODO confirm save dialog
        tab->close();
    }
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

        PackedStringArray paths = p_layout->get_value("Eventer", "open_file_paths", PackedStringArray());
        if (not paths.is_empty())
        {
            for (const String &path : paths)
            {
                bool failed = true;
                if (ResourceLoader::exists(path))
                {
                    Ref<EventerSequence> sequence = ResourceLoader::load(path);
                    if (sequence.is_valid())
                    {
                        failed = false;
                        editor->open_sequence(sequence);
                    }
                }
                
                if (failed)
                {
                    ERR_PRINT(vformat("Can't reopen sequence: %s", path));
                }
            }
        }
    }
}

void EditorPluginEventer::get_window_layout(Ref<ConfigFile> p_layout)
{
    p_layout->set_value("Eventer", "separation", editor->get_separation());
    p_layout->set_value("Eventer", "open_file_paths", editor->get_open_tab_paths());
}

EditorPluginEventer::EditorPluginEventer()
{
    editor = memnew(EventerEditor);
    editor->hide();
    EditorInterface::get_singleton()->get_editor_main_screen()->add_child(editor);
    EventerDB::scan_script_command_paths();
}

#endif
