lion
====

A library for UNIX domain IPC using a publish/subscribe system.

LionClients connect to a LionBroker and subscribe to message types they are interested in.

Intended for Haisan project, LionClients and LionBrokers both feature an egress queue where
pending messages are kept waiting to be sent.
