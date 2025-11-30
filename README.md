EMC
===

# 1. Description

EMC is a library that encapsulates a lightweight protocol for communication between
devices and processes as well as the basic software building blocks to support and extend it.
Core design principles are as follows:
- human readable I/O, but not exclusively (support high-bandwidth data transfers as well,
  either directly or by sidecar connections);
- operate on top of a lower level P2P or broadcast protocol (network, UART or even SPI) but
  do not monopolize it - i.e. allow other traffic on the interface as well;
- a single EMC pipeline is allowed on the same interface;
- low overhead;
- accept both buffered and unbuffered input;
- allow support for encryption, trancoding, compression, filtering, etc. on the interface.

# 2. Operation

EMC processes messages in a _pipeline_, comprising of one or more independent _stages_.
A _reactor_ is responsible for instantiating and managing a pipeline. A reactor allows an
arbitrary number of stages to be attached to it and linked into a pipeline.
Messages in a pipeline flow from one stage to the next. The default stage object will simply
relay any inbound messages to the next stage. By overriding this behaviour, stages can be
defined to perform any custom operation on the inbound or outbound messages or extend the
base protocol with new services.

## 2.2. Roles

Depending on the _function_ of the message pipeline - i.e. whether it is meant to _serve_
particular functionality to a controlling device or act as a bridge towards an EMC-enabled
server, a pipeline can have one of the following major _roles_:
- _host_ role: serve a set of functions to a connecting instance, called an _agent_;
- _user_ role: drive a connection to a remote EMC-enabled device.

A hybrid, _proxy_ role can be defined by externally connecting two pipelines (one with a _host_
and one with an _user_ role) back to back.

## 2.2. Message flow

On the forward path, messages flow sequentially from the first to the last stage of the
pipeline. Each stage is responsible for passing the message along to the next stage.

```
     Stage1 Stage2 Stage3    ...      StageN
       |      |      |                  |
       |----->|      |                  |
       |      |----->|                  |
       |      |      |----->            |
      ...    ...    ...      ...       ...
       |      |      |            ----->|

```

On the return path, messages flow sequentially from the originating stage of the pipeline to
the previous. Each stage is responsible for passing the return message along to the previous
stage.

```
     Stage1 Stage2 Stage3    ...      StageN
       |      |      |                  |
       |      |      |            <-----|
      ...    ...    ...      ...       ...
       |      |      |<-----            |
       |      |<-----|                  |
       |<-----|      |                  |

```

## 2.3. Pipelines

The reactor component instantiates a `raw` pipline. As the name implies, raw pipeline
operates with raw messages - i.e. by default it does not expect inbound messages to comply with
any particular format - EMC or otherwise. This pipeline is useful for defining custom protocol
streams (i.e. filter stages, encoding stages, data encryption stages).

The `emc` pipeline, which implements the EMC protocol is branched out from the `raw` pipeline,
at the end of the transcoding chain (if any).
An `emc` pipeline can be enabled by attaching a `gateway` stage to the `raw` pipeline.

### 2.3.1. Stage callbacks

The `raw` pipeline operates with the following events:
- `attach`: triggered for individual stages when they are added onto the pipeline;
- `resume`: triggered for every stage when the reactor resumed operation;
- `join`: triggered for every stage when the reactor is ready to communicate on a network or bus;
- `proto_up`: called for protocol stages when the handshake for a higher level protocol is successful;
<!-- - `recv`: inbound message received onto the error stream (i.e. process`stderr`, where applicable);
- `feed`: inbound message received onto the main stream;
- `send`: outbound message to be sent out on the return path; -->
- `proto_down`: called for protocol stages when the protocol is disabled or critically fails;
- `drop`: triggered for every stage when the reactor is no longer able or ready to communicate on a
network or bus;
- `suspend`: triggered for every stage when the reactor suspended operation;
- `detach`: triggered on individual stages when they are removed from the pipeline;
- `sync`: called periodically

### 2.3.2. The stage API

The above events correspond to function callbacks into the `stage` interface:
```cpp
  void  emc_raw_attach(reactor*) noexcept;
  bool  emc_raw_resume(reactor*) noexcept;
  void  emc_raw_join() noexcept;
  void  emc_raw_proto_up(const char*, const char*, unsigned int) noexcept;
  void  emc_raw_recv(std::uint8_t*, std::size_t) noexcept;
  int   emc_raw_feed(std::uint8_t*, std::size_t) noexcept;
  int   emc_raw_send(std::uint8_t*, std::size_t) noexcept;
  void  emc_raw_proto_down() noexcept;
  void  emc_raw_drop() noexcept;
  void  emc_raw_suspend(reactor*) noexcept;
  void  emc_raw_detach(reactor*) noexcept;
  void  emc_raw_sync(float) noexcept;
```

### 2.3.3. Reactor events

- `ev_accept`:
- `ev_fee`:
- `ev_pendin`:
- `ev_listenin`:
- `ev_runnin`:
- `ev_joi`:
- `ev_connec`:
- `ev_send`:
- `ev_recv_std`:
- `ev_recv_aux`:
- `ev_progress`:
- `ev_disconnect`:
- `ev_terminating`:
- `ev_drop`:
- `ev_hup`:
- `ev_close_request`:
- `ev_reset_request`:
- `ev_abort`:
- `ev_terminated`:
- `ev_soft_fault`:
- `ev_hard_fault`:

<!-- ### 2.3.2. The `emc` pipeline

The `emc` pipeline operates with the following events:

- `join`: relayed from the raw pipeline when a socket or device connection becomes
  available to a client; Pipelines in the `_host_` role are considered to be implicitely
  "connected", so this event will is not required to be fired for them;
- `connect`: EMC handshake successful (i.e. the `INFO` response received) in the _user_ role;
- `process_message`: callback for a received message, in string form (useful for monitors and
  loggers)
- `process_error`: callback for a received stderr message, in the string form (useful for
  monitors and loggers)
- `process_request`: received an inbound query decoded into an EMC request (for pipelines in
  the _host_ role);
- `process_response` received an inbound query decoded into an EMC response (for pipelines in
  the _user_ role);
- `process_comment`: callback for a received query that is neither a request nor a response in
  EMC message format;
- `process_packet`: received a data packet;
- `return_message`: response to an inbound message to be returned (on the return flow);
- `return_packet`: response to an inbound packet to be returned (on the return flow);
- `disconnect`: connection to the server still up, but EMC interface no longer responding
  (_user_ role);
- `drop`: connection to the server closed or lost.


The events are accessible via the `emcstage` interface:
```
  bool  emc_std_resume(gateway*) noexcept
  void  emc_std_join()
  void  emc_std_connect(const char*, const char*, int)
  void  emc_std_process_message(const char*, int)
  void  emc_std_process_error(const char*, int)
  int   emc_std_process_request(int, const sys::argv&)
  int   emc_std_process_response(int, const sys::argv&)
  void  emc_std_process_comment(const char*, int)
  int   emc_std_process_packet(int, int, std::uint8_t*)
  int   emc_std_return_message(const char*, int)
  int   emc_std_return_packet(int, int, std::uint8_t*)
  void  emc_std_disconnect()
  void  emc_std_drop()
  void  emc_std_suspend(gateway*) noexcept
  void  emc_std_event(int, void*)
  void  emc_std_sync(float)
``` -->

## 2.4. Rings

- `emi_ring_network`: peer is running remotely, no shm, ipc or direct filesystem access
  available between peers
- `emi_ring_machine`: peer is attached to the same machine - shm and ipc mechanisms available,
  filesystem paths are absolute and restricted
- `emi_ring_session`: peer is attached to the same session - shm and ipc mechanisms available,
  filesystem paths can be relative to the session path
- `emi_ring_process`: peer is within the same address space

## 2.5. Controllers and Layers

Controllers are a subset of EMC stages which provide a services to one or more agents.
through the command line interface. Each controller extends the base protocol with an unique set
of commands through which it can be interacted with, called a _Layer_.

- `get_layer_name()`: name for the controller/layer; if `nullptr`, the service will be hidden from the
  support list
- `get_enabled()`: indicates whether or not the service is active

## 2.6. Mappers and Streams

Mappers provide a way to serve high bandwith binary interfaces (_streams_) to one or more agents.
As opposed to controllers, mappers do *not* need to define and expose their own commands, but are instead accessible via a
common set of commands, described by the "**map**" layer.

Onto the agent side, objects called `Streams` are responsible for interfacing and mantaining a healthy connection with the
mappers.

For this purpose, the EMC library implements a set of conventions pertaining to mapper/device communication.

<!-- How are mappers advertised to the agent??? -->

<!-- ## 2.6. Devices and Streams

Devices offer a high bandwith interface to _streams_ on the controlling device (agent). As opposed to
generic controllers, Devices do *not* need to define their own commands, but are instead accessible
through a predefined set of commends (the "**dev**" layer) which they implement:


EMC manages devices and streams via special entity called a _mapper_, which can be further specialized
for a multitude of device types and functions. -->

## 2.7. Services and Interfaces

Services are bits of functionality that an _agent_ can make available to an EMC device in order to fulfill certain tasks;

        agent      |      device

                  EMC
      [service] <-----> [interface]

<!-- - `get_layer_name`: name for the controller/layer; if `nullptr`, the service will be hidden from the
  support list
- `get_enabled`: indicates whether or not the service is active -->

Services are advertised by the agent upon connect (as part of the `sync` sequence) and via the `s*` event.

# 3. The EMC Protocol

## 3.1. Query format
```
  RQID := [A-Za-z?]+      ; request identifier
  RSID := [A-Za-z0-9]+    ; response identifier
  SPC  := [\s\t]+
  EOL  := [\r] | [\n] | [\r\n]
```

## 3.2. Standard Requests/Events

### 3.2.1. General form
```
  REQUEST := '?' RQID ... EOL
```

### 3.2.2. `?i`: the info request
### 3.2.3. `?g`: the ping request
### 3.2.4. `?z`: the bye request
### 3.2.5. `?s+`: service resume
### 3.2.6. `?s-`: service suspend

## 3.3. Standard Responses/Triggers

### 3.3.1. General form
```
  RESPONSE := ']' RSID ... EOL
```

### 3.3.2. `]i`: the info response
```
  PROTOCOL  := [0-9a-z_-]+
  DECIMAL   := [0-9]
  VERSION   := DECIMAL '.' DECIMAL DECIMAL
  NAME      := [0-9A-Za-z_-]{1..23}
  TYPE      := [0-9A-Za-z_-]{1..7}
  ARCHITECTURE  := IDENT
  BYTE_ORDER    := "le" | "be"
  MTU       := [0-9A-Fa-f]+

  RESPONSE  := ']' 'i' SPC PROTOCOL SPC 'v' VERSION SPC NAME SPC TYPE SPC ARCHITECTURE '_' ORDER SPC MTU EOL
```

### 3.3.3. `]c`: the "caps" response
```
  LAYER     := [A-Za-z_][0-9A-Za-z_]*
  RESPONSE  := ']' 'c' SPC [LAYER [SPC LAYER]...] EOL
```

### 3.4.1. `]g` - the 'pong' response
```
  RESPONSE := ']' 'g' SPC [^SPC]+ EOL
```

### 3.3.5. `]0` - the `OK` response

### 3.3.6. `]\x+` - the error response
```
  HEX := [0-9A-F]
  RESPONSE := ']' HEX{2} .* EOL
```

### 3.3.7. `]z` - the `BYE` response
```
  RESPONSE := ']' 'z' .* EOL
```
## 3.6. Services

## 3.7. Channels
```
  HEX := [0-9A-Fa-f]
  SIZE := BYTE{3}
  RESPONSE := [\x80-\xfe] SIZE DATA{SIZE} EOL
```
  CHANNEL = EOF - C

## 3.8. The Sync sequence

- INFO
- CAPS
- ...
- EOD

# 4. Standard layers

## 4.1. The `map` layer

### 4.1.1. The `support` request

### 4.1.2. The `describe` request

### 4.1.3. The `open` request

> o <channel> <device> [<option>...]
> o * <device> [<options>...]
> o 0 <device> [<options>...]

### 4.1.4. The `control` request

> ctl <channel> +sync
> ctl <channel> -sync

### 4.1.5. The `read` request/event

> r <channel> <offset> <size>

### 4.1.6. The `write` request/event

> w <channel> <offset>

### 4.1.7. The `close` request

> x <channel>

### 4.1.8. The `sync` request

