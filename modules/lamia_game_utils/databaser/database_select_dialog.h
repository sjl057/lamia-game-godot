#ifndef __DATABASE_SELECT_DIALOG_H__
#define __DATABASE_SELECT_DIALOG_H__

#include "core/object/object.h"
#include "database.h"
#include "../general/filter_edit.h"
#include "database_tree.h"
#include "scene/gui/dialogs.h"

class DatabaseSelectDialog : public ConfirmationDialog
{
    GDCLASS(DatabaseSelectDialog, ConfirmationDialog)

private:
    FilterEdit* filter_edit;
    DatabaseTree *tree;

    void _on_selected(const StringName &p_path, Ref<DatabaseResource> p_data);
    void _on_cancelled();
    void _on_ok_pressed();

protected:
    static void _bind_methods();

public:
    void setup(Ref<DatabaseResource> p_group, const StringName &p_default_path, const String &p_title);
    void popup_database_group_select(Ref<DatabaseResource> p_group, const StringName &p_default_path, const String &p_title);

    DatabaseSelectDialog();
};

#endif // __DATABASE_SELECT_DIALOG_H__