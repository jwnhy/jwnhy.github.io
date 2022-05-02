---
title: Planning to Write a Unikernel in Rust
category: System
---

## Why wrting a unikernel in Rust?

1. I have experience in writing some toy kernel in Rust, but they kind of not
fitting my wish, no filesystem, limited syscalls, etc.
2. It may be useful in cases like virtualization/TEEs.

## What it consists of?

1. Base libaries
    - Memory management
    - Scheduler
    - Common syscalls
2. Filesystem
    - `virt-block` compatiable
3. Network stack
    - `virtio` compatiable

## What it is?

1. Running on virtualized/special machine environment
2. Easy compartmentalization in both `code` and `data`
3. Small yet secure

## What it is not?

1. Chuck of drivers
2. Running on baremetal board
3. Dealing with tedious details
