EMC
===

# 1. Description

EMC is a support library that encapsulates a lightweight protocol for communication between devices.

# 2. Principle of operation

EMC is a broadcast protocol, similar to MQTT. EMCC has no intrinsic concept of a peer, all the communication is directed towards 'the bridge' - the supporting physical layer of the
communication channel. EMC is agnostic of whether there is one, more or no devices actively listening onto the communication channel.

## 2.1. Protocol

### 2.1.1. Query format
```
  RQID := [A-Za-z?]+      ; request identifier
  RSID := [A-Za-z0-9]+    ; response identifier
  SPC  := [\s\t]+
  EOL  := [\r\n]
```

### 2.1.2. Protocol states

### 2.1.3. Standard Requests

```
  REQUEST := '?' RQID ... EOL
```

- '!' - the help command
- '@' - the sync command
- 'i' - the info request
- 'g' - the ping request
- 'z' - the bye request

### 2.1.4. Standard Events

- ?p+ support_name
- ?p- support_name

### 2.1.5. Standard Triggers and Responses

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
  ORDER    := "le" | "be"
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

### '0' - OK response

### 'e' - the error response

### 'z' - the bye response

### 2.1.6. Services

### 2.1.7. Channels
```
  HEX := [0-9A-Fa-f]
  SIZE := HEX{3}
  RESPONSE := [\x80-\xfe] SIZE DATA{SIZE} EOL
```
  CHANNEL = EOF - C

## 2.2. Message flows

### 2.2.1. The Sync sequence
```
< INFO
< + <service_0> <properties>...
< + <service_1> <properties>...
...
< + <service_N> <properties>...
< READY
```
### 2.2.2. 

# 3. API

## 3.1. Core components

### 3.1.1. `reactor`

### 3.1.2. `session`

### 3.1.3. `gateway`

### 3.1.4. `monitor`

## 3.2. Helper classes

### 3.2.1. `host`

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
