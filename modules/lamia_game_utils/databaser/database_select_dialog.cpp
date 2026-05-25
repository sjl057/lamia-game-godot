#include "database_select_dialog.h"
#include "core/error/error_macros.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include "database_tree.h"

DatabaseSelectDialog::DatabaseSelectDialog()
{
    VBoxContainer *vbox = memnew(VBoxContainer);
    add_child(vbox);

    tree = memnew(DatabaseTree);
    tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
    tree->connect("data_activated", callable_mp(this, &DatabaseSelectDialog::_on_selected));

    filter_edit = memnew(FilterEdit);
    filter_edit->connect("text_changed", callable_mp(tree, &DatabaseTree::set_filter));

    vbox->add_child(filter_edit);
    vbox->add_child(tree);

    get_ok_button()->connect("pressed", callable_mp(this, &DatabaseSelectDialog::_on_ok_pressed));
    get_cancel_button()->connect("pressed", callable_mp(this, &DatabaseSelectDialog::_on_cancelled));
    connect("close_requested", callable_mp(this, &DatabaseSelectDialog::_on_cancelled));
}

void DatabaseSelectDialog::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("popup_database_group_select", "group", "default_path", "title"), &DatabaseSelectDialog::popup_database_group_select);

    ADD_SIGNAL(MethodInfo("selected", PropertyInfo(Variant::STRING_NAME, "path"), PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "DatabaseResource")));
}

void DatabaseSelectDialog::setup(Ref<DatabaseResource> p_group, const StringName &p_default_path, const String &p_title)
{
    ERR_FAIL_NULL(p_group);
    // if (p_title.is_empty())
    // {
    //     p_title = "Select a Resource";
    // }
    set_title(p_title);
    tree->set_group(p_group);
}

void DatabaseSelectDialog::popup_database_group_select(Ref<DatabaseResource> p_group, const StringName &p_default_path, const String &p_title)
{
    setup(p_group, p_default_path, p_title);
    popup_centered();
}

void DatabaseSelectDialog::_on_selected(const StringName &p_path, Ref<DatabaseResource> p_data)
{
    emit_signal(SNAME("selected"), p_path, p_data);
    filter_edit->clear();
    tree->clear();
    hide();
}

void DatabaseSelectDialog::_on_cancelled()
{
    filter_edit->clear();
    tree->clear();
    hide();
}

void DatabaseSelectDialog::_on_ok_pressed()
{
    if (not tree->get_selected_data_path().is_empty() and tree->get_selected_data().is_valid())
    {
        _on_selected(tree->get_selected_data_path(), tree->get_selected_data());
    }
}