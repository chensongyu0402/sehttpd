# sehttpd
###### tags: `github_project`

# seHTTPd

`seHTTPd` implements a small and efficient web server with 1K lines of C code.
I/O multiplexing is achieved using [epoll](http://man7.org/linux/man-pages/man7/epoll.7.html).

## Features
### original version
* Single-threaded, non-blocking I/O based on event-driven model
* HTTP persistent connection (HTTP Keep-Alive)
* A timer for executing the handler after having waited the specified time
### Improvement
* Using epoll API to monitor multiple file descriptors to see if I/O is possible on any of them
* Use sendfile() for zero-copy
* Use multithreading to achieve high performance
* Use threadpool to reduce thread creation overhead
* Use memorypool to reduce http_request creation overhead
* Use wrk and htstress to evaluate sehttpd web server with keep-alive and without keep-alive performance
### Result
* Compare performance between original version and final version
* ![](https://i.imgur.com/dykxiQS.png)
* ![](https://i.imgur.com/3SUft44.png)
## High-level Design

```text
+----------------------------------------------+
|                                              |
|  +-----------+   wait   +-----------------+  |  copy   +---------+
|  |           +---------->                 +------------>         |
|  | IO Device |    1     | Kernel's buffer |  |   2     | Process |
|  |           <----------+                 <------------+         |
|  +-----------+          +-----------------+  |         +---------+
|                                              |
+----------------------------------------------+
```

## Build from Source

At the moment, `seHTTPd` supports Linux based systems with epoll system call.
Building `seHTTPd` is straightforward.
```shell
$ make
```

### Default server 
```shell
./sehttpd
```

### Specify the port
```shell
./sehttpd -p 8082 
```

Specify the port number with `-p` flag, by default the server accepts connections on port 8081.

### Specify the web root
```shell
./sehttpd -w ./www
```

Specify the web root with `-w` flag, by default the web root is "./www".

## License
`seHTTPd` is released under the MIT License. Use of this source code is governed
by a MIT License that can be found in the LICENSE file.