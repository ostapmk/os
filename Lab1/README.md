# Tasks synchronization and parallelization

## Abstract

Program implements computation of two functions using processes
and notification about result via unnamed pipe. Also it supports
multiple simultaneous clients using only one thread, because of
async I/O.

## Requirements

* C++17 aware compiler with `<charconv>` implemented
* Boost v1.74.0+
* CMake 3.14+

## Build

```bash
$ git clone https://github.com/ostapmk/os
$ cd os
$ mkdir build && cd build
$ cmake -DBUILD_LAB1 ..
$ make -j
```

## Usage

### Server

#### Check available options

```bash
$ lab1 --help
```

#### Launch

```
$ ./lab1 --listen 127.0.0.1 --port 20002
```

#### Terminate

You can ask server to terminate gracefully by sending `SIGTERM`.

### Client

#### Launch

```bash
$ nc 127.0.0.1 20002
```

#### Terminate
```
Ctrl + C
```