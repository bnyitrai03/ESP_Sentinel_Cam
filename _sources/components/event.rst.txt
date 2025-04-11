Event Manager
==============
The ``Event Manager`` is responsible for inter-task communication. The event queue operates within the **main task**,
while other tasks publish their events to this queue. 

The main task processes events from the queue and invokes the corresponding event handler.
An event handler is a function that must be registered with the ``Event Manager``.

.. include-build-file:: inc/event_manager.inc