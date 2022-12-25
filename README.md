# 1. Description
EMC is a support library that encapsulates a lightweight protocol for communication between devices.

# 2. Principle of operation

EMC is a broadcast protocol, similar to MQTT. EMCC has not intrinsic concept of a peer, all the communication is directed towards 'the bridge' - the supporting physical layer of the
communication channel. EMC is agnostic of whethere is one, more or no devices actively listening onto the communication channel.

## 2.1. Protocol

### 2.1.1. Protocol states

### 2.1.1. Services

### 2.1.2. Standard Requests

### 2.1.3. Standard Events

?p+ support_name
?p- support_name

### 2.1.4. Standard Triggers and Responses

]i ...
]s+ svc_name
]s- svc_name
]0  ready

### 2.1.5. Channels

## 2.x. Message flows

### 2.x.1. The Sync sequence
```
< INFO
< + <service_0> <properties>...
< + <service_1> <properties>...
...
< + <service_N> <properties>...
< READY
```
### 2.x.2. 

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

Servives that are made available to the server through the bridge -->