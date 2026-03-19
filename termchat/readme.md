
# Chat Server (poll-based) — Deep Explanation

This README explains **exactly what every variable, struct, vector, and file descriptor is doing** in your server. No fluff — just mechanics.

---

# 1. Core Concept (What you are actually managing)

Your server is managing **multiple connections at the same time**.

Each connection is represented by:

```
fd (file descriptor)
```

Think of `fd` as:

> A unique integer ID given by the OS to represent a socket (connection)

---

# 2. Important Variables (You MUST understand these)

## (A) `sock_fd`

```
int sock_fd;
```

* This is the **server socket**
* It listens for new connections
* Created using `socket()`
* Used in `bind()` and `listen()`

👉 It does NOT send/receive chat messages
👉 It ONLY accepts new clients

---

## (B) `fds` (VERY IMPORTANT)

```
std::vector<pollfd> fds;
```

This is your **main tracking system**

Each element:

```
pollfd {
  fd       → file descriptor
  events   → what we want (POLLIN = ready to read)
  revents  → what actually happened
}
```

So `fds` contains:

| Type          | fd value |
| ------------- | -------- |
| Server socket | sock_fd  |
| Client 1      | 4        |
| Client 2      | 5        |
| Client 3      | 6        |

👉 `poll()` watches ALL of these together

---

## (C) `fd` vs `fds` (your confusion)

### `fd`

* A **single integer**
* Represents ONE connection

Example:

```
fd = 5  → client socket
```

---

### `fds`

* A **vector (list)** of all sockets
* Used by `poll()` to monitor multiple connections

---

## (D) `Client` struct

```
struct Client {
  int fd;
  std::string username;
  int room;
};
```

This is your **application-level data**

It maps:

```
fd → username + room
```

Example:

```
fd = 5 → "nitish", room 101
fd = 6 → "aman", room 101
fd = 7 → "raj", room 202
```

---

## (E) `clients`

```
std::vector<Client> clients;
```

This stores ALL registered users.

👉 This is separate from `fds`

| Vector  | Purpose                       |
| ------- | ----------------------------- |
| fds     | sockets (network layer)       |
| clients | user data (application layer) |

---

# 3. How Everything Connects

This is the key mapping:

```
fds[i].fd  → socket
         ↓
get_client(fd)
         ↓
Client { username, room }
```

---

# 4. Full Flow (Step-by-Step Execution)

## STEP 1 — Server starts

```
socket()
bind()
listen()
```

* Server is ready
* `sock_fd` added to `fds`

---

## STEP 2 — poll()

```
poll(fds)
```

* OS waits until ANY fd has activity

---

## STEP 3 — New client connects

Condition:

```
fds[i].fd == sock_fd
```

Action:

```
accept()
```

* New socket created → `new_client`
* Added to `fds`

Now:

```
fds = [sock_fd, client1_fd, client2_fd]
```

---

## STEP 4 — Client sends data

Condition:

```
fds[i].revents & POLLIN
```

Action:

```
read(fd, buffer)
```

---

## STEP 5 — Identify WHO sent message

```
Client* client = get_client(fd, clients);
```

Two cases:

---

### Case A: New client (not registered)

```
client == nullptr
```

Message expected:

```
JOIN nitish 101
```

Parsing:

```
command = JOIN
username = nitish
room = 101
```

Store:

```
clients.push_back({fd, username, room});
```

---

### Case B: Existing client

```
client != nullptr
```

Now you know:

```
client->username
client->room
```

---

## STEP 6 — Message routing

Loop through all fds:

```
for each fd in fds
```

Skip:

```
- server socket
- sender itself
```

Then:

```
other = get_client(fd)
```

Condition:

```
other->room == client->room
```

If true:

```
send message
```

---

# 5. Data Flow (Very Important)

## Incoming

```
Client → TCP → kernel → read() → buffer → string
```

---

## Processing

```
fd → get_client → username + room
```

---

## Outgoing

```
send() → kernel → TCP → other client
```

---

# 6. Disconnection

When:

```
read() returns <= 0
```

Meaning:

* client closed connection

You must:

```
close(fd)
remove from fds
remove from clients
```

---

# 7. Common Mistakes (YOU made these)

## ❌ Confusing global vs per-fd logic

Wrong:

```
if (clients == nullptr)
```

Correct:

```
if (get_client(fd) == nullptr)
```

---

## ❌ Mixing fds and clients

Remember:

```
fds     → sockets
clients → user info
```

They are linked ONLY by `fd`

---

## ❌ Thinking server "knows" username

It does NOT.

You explicitly:

```
receive → parse → store
```

---

# 8. Mental Model (Fix this in your head)

Think like this:

```
[ OS gives fd ]
        ↓
[ You store fd in fds ]
        ↓
[ Client sends message ]
        ↓
[ You map fd → Client ]
        ↓
[ Use username + room ]
```

---

# 9. One-Line Summary

```
fds = network layer (who is connected)
clients = application layer (who they are)
fd = bridge between both
```

---

# 10. If you're still confused

Then isolate and print:

```
printf("FD: %d\n", fds[i].fd);
printf("Username: %s\n", client->username.c_str());
printf("Room: %d\n", client->room);
```

Trace everything manually.

---

This system only becomes clear when you track:

```
fd → client → message → routing
```

If that chain breaks in your head, everything feels random.
