#pragma once

#include "core/io/resource.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/typedefs.h"
#include "modules/lamia_game_tools/eventer/ev.h"
#include "modules/lamia_game_tools/eventer/whiteboard.h"
#include "modules/lamia_game_tools/general/defs.h"

class EventerSequenceInstance;

class EventerSequence : public Resource
{
    GDCLASS(EventerSequence, Resource);

private:
    Ref<EVBase> root;

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ void set_root(const Ref<EVBase> &p_root) { root = p_root; }
    _FORCE_INLINE_ Ref<EVBase> get_root() const { return root; }
    Ref<EventerSequenceInstance> instantiate() const;
    EventerSequence();
};

class EventerSequenceInstance : public RefCounted
{
    GDCLASS(EventerSequenceInstance, RefCounted);

private:
    Ref<EVBase> root;
    Ref<EventerWhiteboard> whiteboard;
    Ref<EVBase> next_override;

protected:
    GDVIRTUAL0(_on_start);
    static void _bind_methods();

public:
    void setup(Ref<EventerSequence> p_sequence);
    _FORCE_INLINE_ Ref<EVBase> get_root() const { return root; }
    Ref<EVBase> get_next_in_tree(Ref<EVBase> p_from = nullptr);
    Ref<EVBase> get_next_sibling(Ref<EVBase> p_from = nullptr);

    virtual void on_start();
};

class EventerSequenceInterpreter : public RefCounted
{
    GDCLASS(EventerSequenceInterpreter, RefCounted);
private:
    Ref<EventerSequence> sequence;
    Ref<EventerSequenceInstance> sequence_instance;
    Ref<EVBase> current_command;
    bool running = false;

    void _next_in_tree();
    void _next_sibling();

protected:
    static void _bind_methods();

public:
    _FORCE_INLINE_ Ref<EventerSequence> get_sequence() const { return sequence; };
    _FORCE_INLINE_ bool is_running() const { return running; }

    void start(Ref<EventerSequence> p_sequence = nullptr);
    EVResult update(const float &p_delta);
    void stop();
};