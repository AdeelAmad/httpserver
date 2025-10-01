# HTTP Server and Reverse Proxy

A high-performance HTTP/1.1 server written in C with POSIX threads, designed to handle thousands of requests per second.  
Includes support for reverse proxying and pluggable caching algorithms (LRU, MRU, Random Eviction) to reduce request latency.

---

## 🚀 Features

- HTTP/1.1 support: serves static files and proxies requests  
- Concurrency: handles multiple client requests using a static thread pool  
- Thread safety: protects shared resources with mutexes, semaphores, and condition variables  
- Reverse proxy: forwards requests to backend servers to improve scalability  
- Caching: supports multiple caching strategies  
  - LRU (Least Recently Used)  
  - MRU (Most Recently Used)  
  - Random Eviction  
- High throughput: capable of serving thousands of requests per second  

---

## 🛠️ Technologies Used

- Language: C  
- Concurrency: POSIX Threads (pthreads)  
- Networking: BSD Sockets, HTTP/1.1  
- Synchronization: Mutexes, Semaphores, Conditional Variables  
- Algorithms: LRU / MRU / Random Eviction Cache  
