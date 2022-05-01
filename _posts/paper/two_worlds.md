# A Tale of Two Worlds: Assessing the Vulnerability of Enclave Shielding Runtimes

## TL;DR
本文通过**人工**分析 8 个广泛使用的 TEE Runtime，总结出了涉及 ABI 与 API 中常见的 10 种安全疏忽。【<del> 只要我 PhD 够多就是可以为所欲为啦 </del>】

尽管本文采用的方法论过于暴力，但其提出的十种类型的安全问题还是值得学习和研究的。

## Background

### TEE 的类型
文章中提到两种 TEE 的类型，SGX 嵌入用户地址空间的 TEE 与 Keystone/TrustZone 独立使用自己的地址空间的 TEE。【<del>所以真的没人用农企的 SEV 对吗？</del>】

### TEE 状态切换
SGX 中有 `ocall`，`ecall`，`AEX`，`SSA` 等概念，不了解的可以看一下本公众号之前关于 SGX 的文章，例如 `SmashEx`。

这些复杂的概念通常都是为了支持 `enclave` 能够嵌入到非安全用户地址空间之中，并且由操作系统进行各类系统调用，异常处理之类的，尽管这样实现的 TEE 从功能性来说比独立地址空间的要好，但也需要复杂的设计，例如在进入和退出 TEE 时的原子性，CPU 状态切换等。

对于独立地址空间的 TEE，例如 Keystone，因为其 Runtime 完全与操作系统独立，因此复用原有的机制即可。

## ABI 疏忽

### 进入时硬件状态检查【<del>SGX专享</del>】
<del>臭名昭著</del> 的 x86 体系架构具有复杂的 CPU Flags 机制，其中很多都能影响一个程序的正确性，文章作者检查了所有软件可以设置的 Flags，找到了下面这些 <del>小可爱</del>。

- `#AC`: Alignment Check，x86 支持非对齐访问，尽管非常影响性能，因此当 enclave 内部如果使用非对齐访问的代码，但用户程序在进入 TEE 时开启了 `#AC` 那么 enclave 每次涉及非对齐访问都会出现异常，引发侧信道问题。
- `#DF`: Direction Flag，x86 有针对字符串进行的特殊循环指令 <del>x86真是太好了</del>，而 `#DF` 会决定这类指令执行的方向，从前向后/从后向前，如果在进入时改变该 Flag，可能导致 TEE 针对字符串/内存的行为出现异常。

### 调用栈维护【<del>好像又是SGX专享</del>】
这个问题比较简单，其实就是 enclave 需要维护自己专用的栈。在每次 `ocall` 退出 enclave时，需要将自己的栈帧保存在 enclave 内部，然后在重入时，从被保护的区域还原栈帧。

>> 据作者所说，该问题在工业界 Runtime 中几乎没有，但在学术界 Runtime 中较为常见。例如 `Graphene-SGX`，`SGX-LKL`，`Sancus`。 <del>Nobody uses them anyway.</del>

### 退出时状态清理【<del>SGX-LKL 行不行</del>】
这个问题也比较简单，其实就是 enclave 跑完了需要清理自身状态，只有 `SGX-LKL` 没有清理自身状态。

>> 这里有一件有意思的事，因为 SGX 退出 TEE 的方式包括由硬件支持的 `AEX`，据作者观察，`AEX` 并不会清理所有的硬件状态，例如前面提到的 `#DF`，这导致了可能的侧信道问题。

## API 疏忽
### 进入时指针检查
只要不安全部分和 enclave 分享至少一部分他们的地址空间，一个非安全的代码总是能通过传递一个不安全的指针来造成威胁。
如果不进行指针检查，可能带来的威胁有下面两种（Confused Duputy）。

- 程序因不合法的访问崩溃，带来可能的侧信道问题。
- 程序正常写入，造成任意写入漏洞。

对于这类安全问题，Intel 提出了利用自动代码生成检查的方式 `edger8r`。其自动生成一系列针对函数参数指针的检查。但是其生成的代码在人工检查下仍然能发现问题。

```c
OE_ECALL void ecall_hello ( hello_args_t * p_host_args ) {
oe_result_t __result = OE_FAILURE ;
 if (! p_host_args || ! oe_is_outside_enclave ( p_host_args ,
 sizeof (* p_host_args ) ) )
 goto done ;
 ...
 done :
 // 下面这行有问题
  if ( p_host_args ) p_host_args - > _result = __result ;
 }
```
尽管生成的代码进行了安全检查，但是在函数结束的代码中，仍然对不安全的指针进行了写入。
除此之外，部分 **built-in** 的 `ecall` 入口也并未被该自动生成器保护，这类接口也可以被用来进行攻击。

除开常规的函数参数，类似于环境变量 `envp` 以及 `argv` 也是潜在的攻击对象。

### 字符串参数检查
enclave 不应该对任何不信任的字符串进行 `strlen` 长度计算，这会被利用成为一个侧信道手段，计算内存中所有 `0x00` 的位置。
作者利用一个简单的 PoC 加密程序验证了这类攻击是可行的，思路大概是利用 `0x00` 的位置缩小暴力破解的搜索空间，然后通过枚举可能的明文【<del>严重怀疑在真实加密程序上的可行性</del>】。
具体的攻击内容在此不多做介绍。

### 动态长度 Buffer 检查
一般来说 enclave 对于输入的 Buffer Size 都需要加额外检查，即 `base+size<range`。
但由于长度是由攻击者控制，因此，如果攻击者传入一个大于 `INT_MAX` 的值，就可以造成整型溢出，导致 enclave 程序出错。
但有意思的是，这类问题通常对于共享地址空间的 TEE 威胁较大，例如一个整型溢出的漏洞可能在 SGX 造成对任意地址的写入，而对于 Keystone 这类 TEE， 结果可能只是异常报错。

### $Outside \neq \neg Inside$
在一些 Runtime 实现中，对于地址检查存在逻辑上的问题，对于不完全在安全内存区域，也不完全在非安全区域中的数据，未能正确的将其判断为非安全输入。

### ToC-ToU 防御
不安全的输入应当在被使用前拷贝，以防止检查后被其他线程修改，造成 ToC-ToU 攻击。例如 `AsyncShock` 与 `SGX-Step`。

### Iago Attack 防御
Iago Attack 利用 malicious OS 返回一个错误的 Buffer Size 来达成攻击目的，由于 SGX 依赖操作系统进行内存管理，因此 Iago Attack 在 enclave 不验证
Buffer Size 的情况下是完全可行的
【<del>这上面两部分建议自行阅读相关论文</del>】

### 清理结构体数据
对于非对齐的结构体，编译器会自动加入 padding，文中提到之前有[工作](https://lwn.net/Articles/417989/)，利用这类 padding 实现攻击。

## Conclusion
通篇读下来，感觉比较有意思的部分都在前面，后面几个给人一种凑数的感觉。
Nevertheless， 这篇文章仍然<del>有效的</del>总结了 TEE 可能存在的攻击面，为未来的设计提供了参考。

但大部分其中的漏洞都过于细节，很容易由于人为疏忽而引入，因此自动化的代码检查工具必不可少。

本文章也比较专注于软件漏洞，针对类似 SmashEx 的软硬件结合漏洞也没有深入讨论。形式化方法针对软件漏洞是不太适用的，需要过多的 Human Labor 来针对代码进行验证<del>What if we have countable infinite PhDs?</del>。