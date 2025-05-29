# francOS

Efficient and barebone C++ middleware for robotics in embedded projects.

## Features

- Callback-based pub/sub design over topics
- Intraprocess communication only (for now).
- Templated for easy user-defined message types.
- Minimal API: threads, publishers, subscribers and topics.
- Full control over the threading model and callback dispatching.
- Low latency with futexes (linux only).
- Highly-portable by replacing thread implementation.

## Upcoming features

- Topics's message queues replace by reference-counted circular-buffers on shared memory will replace soon per-subscriber quueues.