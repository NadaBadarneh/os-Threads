
# OS Threads – Multi-threaded Web Server

This repository contains the implementation of a multi-threaded web server, developed as part of the Operating Systems course at Technion.

## Overview

The project extends a basic single-threaded HTTP web server into a fully functioning multi-threaded server that:
- Handles both static and dynamic content.
- Supports both regular (`GET`) and priority (`REAL`) requests.
- Implements a thread pool with a dedicated VIP thread.
- Enforces several overload handling policies.
- Tracks and reports usage statistics per request and per thread.

## Features

- **Thread Pool**: A fixed number of worker threads handle incoming requests concurrently.
- **VIP Thread**: A separate thread handles `REAL` requests with high priority, blocking others while it has work.
- **Overload Handling**: Supports `block`, `dt` (drop tail), `dh` (drop head), `bf` (block flush), and `random` scheduling policies.
- **.skip Suffix**: Requests ending in `.skip` trigger handling of the most recent waiting request immediately after.
- **Usage Statistics**: Collected per request and per thread, including request arrival and dispatch time, and counters for each thread's work.

## Build Instructions

```bash
make          # Builds the server, client, and output.cgi
make clean    # Cleans compiled files
 
