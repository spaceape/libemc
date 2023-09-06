EMC
===

# 1. Description

EMC is a library that encapsulates a lightweight protocol for communication between
devices, as well as the basic software building blocks to support and extend it.
Core design principles are as follows:
- human readable I/O, but not exclusively (support high-bandwidth data transfers as well,
  either directly or by sidecar connections);
- operate on top of a lower level P2P or broadcast protocol (network, UART or even SPI) but
  do not monopolize it - i.e. allow other traffic on the interface as well;
- low overhead;
- accept both buffered and unbuffered input;
- allow support for encryption, trancoding, compression, filtering, etc. on the interface.

# 2. Principle of operation

EMC processes messages in a _pipeline_, comprising of one or more independent _stages_.
A _reactor_ is responsible for instantiating and managing a pipeline. A reactor allows an
arbitrary number of stages to be attached to it and linked into a pipeline.
Messages in a pipeline flow from one stage to the next. The default stage object will simply
relay any inbound messages to the next stage. By overriding this behaviour, stages can be
defined to perform any custom operation on the inbound or outbound messages or extend the
base protocol with new services.

## 2.2. Roles

Depending on the _function_ of the message pipeline - i.e. whether it is meant to _serve_
particular functionality to a connected device or act as a bridge towards an EMC-enabled
server, a pipeline can have one of the following major _roles_:
- _host_ role: serve a set of functions to a connecting instance;
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
      ...    ...    ...      ...  
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

### 2.3.1. The `raw` pipeline

The `raw` pipeline operates with the following events:
- `join`: triggered by _some_ reactors or interface stages when a socket or device
  connection becomes available to a client; Pipelines in the `_host_` role are considered to be implicitely "connected", so this event will is not required to be fired for them;
- `feed`: inbound query received;
- `send`: response to an inbound query to be returned (the return flow);
- `drop`: connection to the server closed or lost.

The events are accessible via the `rawstage` interface:
```
  void  emc_raw_join();
  int   emc_raw_feed(std::uint8_t*, int);
  int   emc_raw_send(std::uint8_t*, int);
  void  emc_raw_drop();
```

### 2.3.2. The `emc` pipeline

The `emc` pipeline operates with the following events:

- `join`: relayed from the raw pipeline when a socket or device connection becomes
  available to a client; Pipelines in the `_host_` role are considered to be implicitely
  "connected", so this event will is not required to be fired for them;
- `connect`: EMC handshake successful (i.e. the `INFO` response received) in the _user_ role;
- `process_request`: received an inbound query decoded into an EMC request (for pipelines in
  the _host_ role);
- `process_response` received an inbound query decoded into an EMC response (for pipelines in
  the _user_ role);
- `process_comment`: received a random inbound query that does not conform to either the
  request or the response syntax;
- `process_packet`: received a data packet;
- `return_message`: response to an inbound message to be returned (on the return flow);
- `return_packet`: response to an inbound packet to be returned (on the return flow);
- `disconnect`: connection to the server still up, but EMC interface no longer responding;
- `drop`: connection to the server closed or lost.


The events are accessible via the `emcstage` interface:
```
  void  emc_std_join();
  void  emc_std_connect(const char*, const char*, int);
  int   emc_std_process_request(int, const sys::argv&);
  int   emc_std_process_response(int, const sys::argv&);
  void  emc_std_process_comment(const char*, int);
  int   emc_std_process_packet(int, int, std::uint8_t*);
  int   emc_std_return_message(const char*, int);
  int   emc_std_return_packet(int, int, std::uint8_t*);
  void  emc_std_disconnect();
  void  emc_std_drop();
```

### 2.3.3. Rings

- `emi_ring_session`: peer is attached to the same session - shm and ipc mechanisms available, filesystem paths can be relative to session
- `emi_ring_machine`: peer is attached to the same machine - shm and ipc mechanisms available, filesystem paths should be absolute
- `emi_ring_network`: peer is running remotely, no shm, ipc or direct filesystem access available between the two


### 2.3.4. Features
### 2.3.5. Services
### 2.3.6. Sessions (EMP documentation)
### 2.3.6. Controllers (Session Services - EMP documentation)

# 3. The EMC Protocol

## 3.1. Query format
```
  RQID := [A-Za-z?]+      ; request identifier
  RSID := [A-Za-z0-9]+    ; response identifier
  SPC  := [\s\t]+
  EOL  := [\r\n]
```

## 3.2. Protocol states

## 3.3. Standard Requests

```
  REQUEST := '?' RQID ... EOL
```

- '!' - the help command
- '@' - the sync command
- 'i' - the info request
- 'g' - the ping request
- 'z' - the bye request

## 3.4. Standard Events

- ?p+ support_name
- ?p- support_name

## 3.5. Standard Triggers and Responses

```
  RESPONSE := ']' RSID ... EOL
```

- ]i ...
- ]s+ svc_name
- ]s- svc_name
- ]0  ready

### 'i' - the info response
```
  PROTOCOL := [0-9a-z_-]+
  DECIMAL  := [0-9]
  VERSION  := DECIMAL '.' DECIMAL DECIMAL
  NAME     := [0-9A-Za-z_-]{1..23}
  TYPE     := [0-9A-Za-z_-]{1..7}
  ARCHITECTURE := IDENT
  BITS     := [0-9]+
  BYTE_ORDER   := "le" | "be"
  MTU      := [0-9A-Fa-f]+

  RESPONSE := ']' 'i' SPC PROTOCOL SPC 'v' VERSION SPC NAME SPC TYPE SPC ARCHITECTURE '_' BITS '_' ORDER SPC MTU EOL
```
### 's' - the support response
```
  SERVICE  := [A-Za-z_][0-9A-Za-z_]*
  RESPONSE := ']' 's' SPC {[+-]SERVICE}* EOL
```
### 'g' - the 'pong' response
```
  RESPONSE := ']' 'g' SPC XXXXXXXX EOL
```

### '0' - The `OK` response

### 'e' - the error response

### 'z' - the `BYE` response

## 3.6. Services

## 3.7. Channels
```
  HEX := [0-9A-Fa-f]
  SIZE := HEX{3}
  RESPONSE := [\x80-\xfe] SIZE DATA{SIZE} EOL
```
  CHANNEL = EOF - C

## 3.8. The Sync sequence
```
< INFO
< + <service_0> <properties>...
< + <service_1> <properties>...
...
< + <service_N> <properties>...
< READY
```

# 4. API

[ reactor ]
  session
      stage - stage - stage
              -----
              reactor
                  

[ gateway ] ---> [ raw_stage_t ]
                 [   machine   ]
                        |
                 [ emc_stage_t ]
                        |
                 [ emc_stage_t ]
                        |
                       ...
                        |
                 [ emc_stage_t ]



## 3.1. Core components

### 3.1.1. `reactor`

### 3.1.2. `session`

### 3.1.3. `machine`

### 3.1.4. `monitor`

## 3.2. Helper classes

### 3.2.1. `gateway`

### 3.2.2. `command`

### 3.2.3. `timer`

## 3.3. Error reporting

<!-- # 3. Protocol

## 3.1. Services

Features that the server exposes to the client

## 3.2. Support

Servives that are made available to the server through the bridge

## 3.3. Sessions

-->
