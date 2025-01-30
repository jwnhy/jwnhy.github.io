---
title: x86 4-level and 5-level pagetable on Linux
---

The structures of different page tables on Linux.

## Theory

Intel supports 5-level paging, supporting over 128 PB of memory.
However, this makes implementation of Linux a bit werid. So I
write this to help myself remember. The kernel I use is Linux 6.1.38.

Overall, there is also a 5-level structure in Linux as follows.

> CR3 (128PB) -> pgd (256TB) -> p4d (512GB) -> pud (1GB) -> pmd (2MB) -> pte (4KB)

Interestingly, structures like `struct pgd_t` are in fact *entries*
inside pagetables. For instance, the following code returns the
corresponding `pgd` entry for a specific address.

```c
static inline pgd_t *pgd_offset_pgd(pgd_t *pgd, unsigned long address)
{
    return (pgd + pgd_index(address));
};
```

However, this causes a divergence in terms of semantics of `*_offset`
functions.

For `pgd_t` and `pgd_t` only, the `pgd_offset_*` functions
perform *same* level offseting (`pgd_t` -> `pgd_t`).

For other levels, the `*_offset` function return the pagetable for
*next* level (e.g., `pgd_t` -> `p4d_t` for `p4d_offset()`)

Subject to the exact configuration, `p4d` and `pud` may not exist,
but `pgd` always exists. In the cases, where `p4d` or `pud` do not
exist. Their macros are replaced with dummy implementation.

For example, the macro `p4d_offset` is as follows. When only 4-level paging
is activated, it directly returns `pgd` (converted to `p4d`);

```c
static inline p4d_t *p4d_offset(pgd_t *pgd, unsigned long address)
{
    if (!pgtable_l5_enabled())
        return (p4d_t *)pgd;
    // Note `*pgd` is used here, extracting the `pgd` entry.
    return (p4d_t *)pgd_page_vaddr(*pgd) + p4d_index(address);
}
```

Similarly for macros like `p4d_index`, these dummy macros just simply
return 0 when `p4d` is folded. This is presumably for a unified implementation
of pagetable walking (same implementation for {3,4,5}-level paging).

The following picture shows how Linux uses different structures to handle
pagetable hierarchy, epspecially `p4d = (p4d_t*)(*pgd) + p4d_index(addr)`.
![image.png](https://pic4.58cdn.com.cn/nowater/webim/big/n_v28cb2886a12a544cd941848d7986907e4.png)

Particularly, when only 4-level paging is enabled, the `p4d_index(*)` *always*
returns 0, that is `pgd` directly points to different `pud`.

## Practice

The reason I dig into this is that I need to map an *unused* part of
[virtual address space](https://www.kernel.org/doc/html/v6.1/x86/x86_64/mm.html)
of Linux and use it for my own purpose.

Specifically, I want to use the 2TB hole from `fffffc0000000000` to
`fffffdffffffffff`, and this should be shared between *all* kernel
thread, meaning it should be injected into `init_mm`, the address space
of `init` process.

> This design only works for 4-level paging w.o. Kernel Pagetable Isolation (KPTI).

![pagetable.drawio.png](https://pic2.58cdn.com.cn/nowater/webim/big/n_v25836460d75744cb38f67e2b07ee66bdd.png)

In practice here, we only care about 4-level paging, meaning `p4d`'s
are always folded as shown in above figure.

```c
void init_x(void) {
  /* other code */
  top_pgd = init_mm.pgd;
  pgd = pgd_offset_pgd(top_pgd, addr);
  p4d = p4d_offset(pgd, addr);   // dummy transition

  /* we assume 4 page level, pgd = p4d*/
  BUG_ON(p4d != (p4d_t *)pgd);
  nr_pgd = (MOAT_END - MOAT_START) >> 39;
  for (i = 0; i < nr_pgd; i++) {
    BUG_ON(!moat_pud_alloc(&init_mm, p4d, addr));
    addr = pgd_addr_end(addr, MOAT_END);
    pgd = pgd_offset_pgd(top_pgd, addr);
    p4d = p4d_offset(pgd, addr); // dummy transition.
  }
}
```

After this function is executed, four additional `pud` are allocated,
`pgd[504-507]` pointing to four different `pud` table.
