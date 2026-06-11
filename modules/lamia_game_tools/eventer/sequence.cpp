#include "sequence.h"
#include "core/object/object.h"
#include "modules/lamia_game_tools/eventer/ev.h"
#include "modules/lamia_game_tools/general/defs.h"

EventerSequence::EventerSequence()
{

}

void EventerSequence::_bind_methods()
{
    BIND(D_METHOD("set_root", "root"), &EventerSequence::set_root);
    BIND(D_METHOD("get_root"), &EventerSequence::get_root);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "root", PROPERTY_HINT_RESOURCE_TYPE, "EVBase", PROPERTY_USAGE_INTERNAL | PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_EDITOR_INSTANTIATE_OBJECT), "set_root", "get_root");
}

Ref<EventerSequenceInstance> EventerSequence::instantiate() const
{
    Ref<EventerSequenceInstance> instance;
    instance.instantiate();
    instance->setup(Ref<EventerSequence>(this));
    return instance;
}

void EventerSequenceInstance::_bind_methods()
{
    GDVIRTUAL_BIND(_on_start);
}

void EventerSequenceInstance::setup(Ref<EventerSequence> p_sequence)
{
    whiteboard.instantiate();
    root = Ref<EVBase>(p_sequence->get_root()->duplicate());

    Ref<EVBase> next = root;
    while (next.is_valid())
    {
        // next->save_default_properties();
        next->setup();
        next = next->get_next_in_tree(true);
    }
}

Ref<EVBase> EventerSequenceInstance::get_next_in_tree(Ref<EVBase> p_from)
{
    if (next_override.is_valid())
    {
        auto override = next_override;
        next_override.unref();
        return override;
    }
    if (p_from == nullptr or not p_from.is_valid()) { return nullptr; }

    Ref<EVBase> next = p_from->get_next_in_tree(true);
    if (next.is_valid())
    {
        //next->load_default_properties();
    }

    return next;
}

Ref<EVBase> EventerSequenceInstance::get_next_sibling(Ref<EVBase> p_from)
{
    if (next_override.is_valid())
    {
        auto override = next_override;
        next_override.unref();
        return override;
    }
    if (p_from == nullptr or not p_from.is_valid()) { return nullptr; }

    Ref<EVBase> next = p_from->get_next_sibling(true);
    if (next.is_valid())
    {
        //next->load_default_properties();
    }

    return next;
}

void EventerSequenceInstance::on_start()
{
    GDVIRTUAL_CALL(_on_start);
}

void EventerSequenceInterpreter::_next_in_tree()
{
    if (not running) { return; }

    current_command = sequence_instance->get_next_in_tree(current_command);
    if (current_command.is_valid())
    {
        current_command->start();
    }
    else
    {
        stop();
    }
}

void EventerSequenceInterpreter::_next_sibling()
{
    if (not running) { return; }

    current_command = sequence_instance->get_next_sibling(current_command);
    if (current_command.is_valid())
    {
        current_command->start();
    }
    else
    {
        stop();
    }
}

void EventerSequenceInterpreter::_bind_methods()
{
    BIND(D_METHOD("get_sequence"), &EventerSequenceInterpreter::get_sequence);
    BIND(D_METHOD("is_running"), &EventerSequenceInterpreter::is_running);
    BIND(D_METHOD("start", "sequence"), &EventerSequenceInterpreter::start);
    BIND(D_METHOD("update", "delta"), &EventerSequenceInterpreter::update);
    BIND(D_METHOD("stop"), &EventerSequenceInterpreter::stop);

    ADD_SIGNAL(MethodInfo("updated", PropertyInfo(Variant::INT, "last_result", PROPERTY_HINT_ENUM, "EVResult")));
    ADD_SIGNAL(MethodInfo("done"));
}

void EventerSequenceInterpreter::start(Ref<EventerSequence> p_sequence)
{
    if (running) { WARN_PRINT("Interpreter is already running"); return; }
    if (p_sequence.is_valid())
    {
        if (p_sequence->get_root()->get_child_count() == 0)
        {
            emit_signal("done");
            return;
        }
        else
        {
            sequence = p_sequence;
        }
    }

    sequence_instance = sequence->instantiate();
    current_command = sequence_instance->get_root();
    running = true;
    _next_in_tree();
}

EVResult EventerSequenceInterpreter::update(const float &p_delta)
{
    if (not running) { return EVResult::EV_INVALID; }
    ERR_FAIL_COND_V_MSG(not current_command.is_valid(), EV_INVALID, "Invalid command");

    EVResult status = current_command->update(p_delta);
    switch (status)
    {
		case EV_NEXT_IN_TREE:
            _next_in_tree();
            break;
		case EV_NEXT_SIBLING:
            _next_sibling();
            break;
		case EV_ABORT:
            stop();
			break;
		default:
			break;
	}

	emit_signal("updated", status);
    return status;
}

void EventerSequenceInterpreter::stop()
{
    if (not running) { return; }
    running = false;
    sequence.unref();
    sequence_instance.unref();
    current_command.unref();
    emit_signal("done");
}
