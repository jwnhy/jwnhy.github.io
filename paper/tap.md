# A Formal Foundation for Secure Remote Execution of Enclaves

## 基本信息
- 会议：CCS '17
- 关键词：Formal Method
- 贡献：提出了 Trusted Abstract Platform (TAP) 的概念，对 SGX 与 Sanctum 的 **远程可信执行** 做了证明。
- Fair Warning：This article involves quite a lot MATH.

## Notations

### Enclave Side
- Enclave: $e=(init_e, config_e)$
  - Initial State: $init_e = [page_c, page_d]$
    - Code Page: $page_c$
    - Data Page: $page_d$
  - Configurations: $config_e = [entrypoint, evrange, acl]$
    - Virtual Range: $evrange$
    - Permissions: $acl$
- Machine State: $\sigma\in\Sigma$
  - Enclave State: $E_e(\sigma)=[vmem, regs, pc, config_e]$
    - Virtual Memory: $vmem: VA \rightarrow W$
    - General Registers: $regs: \mathbb{N} \rightarrow W$
    - Program Counter: $pc \in VA$
    - Copied Configuration: $config_e$
> - $W$ means the set of all machine words (instruction/data). 
> - $VA$ means the set of all virtual addresses.
> - $\mathbb{N}$ means the set of all natural numbers.
- Enclave Input: $I_e(\sigma) = \langle I_e^R(\sigma), I_e^U(\sigma)\\rangle$
  - Random Entropy Source: $I_e^R(\sigma)$
  - Readable Machine State: $I_e^U(\sigma)$
- Enclave Output: $O_e(\sigma)$
- Attack State(unreadable for $e$): $A_e(\sigma)$
- Enclave Execution: $\leadsto: E_e(\sigma)\times I_e(\sigma)\rightarrow \Sigma$
> - A valid transition is denoted as $(\sigma_i, \sigma_j)\in \leadsto$
> - $\sigma_i \leadsto \sigma_j$

### Attacker Side
- Valid Set of Tampering: $tamper={(\sigma_1, \sigma_2): E_e(\sigma_1)=E_e(\sigma_2)}$
> - Attacker shouldn't be able to change the state of enclaves. (integrity).
- Set of Observation: $obs_e(\sigma)=(O_e(\sigma))$

### Execution Representation
- Execution Trace: $\pi=\langle\sigma_0,...\sigma_n\rangle, \sigma_i\leadsto \sigma_{i+1}$
- Privilege Mode Function: $curr(\sigma)\in \{e, u, s, m \}$
- Subsequence in Enclave: $\langle\sigma_0',...,\sigma_n'\rangle, \forall i, curr(\sigma_i')=e$
- Semantic of an enclave $[[e]]$: 

$[[e]]=\{\langle(I_e(\sigma_0',E_e(\sigma_0'),O_e(\sigma_0')))\rangle|init_e(E_e(\sigma_0)) \}$

> $[[e]]$ is uniquely determined by each input sequence. (Single enclave is assumed and entropy source is supplied in $I_e(\sigma_i)$)

> $[[e]]$ is also prefix-closed.
i.e $\forall t\in
[[e]], prefix(t)\in [[e]]$

## Secure Remote Execution
To achieve trustworthy execution of TEE, the authors proposed a definition of "Secure Remote Execution" which is also the only **definition** in this whole paper.
> A remote platform performs secure execution of an enclave program $e$ if any execution trace of $e$ on this platform is contained within $[[e]]$. Furthermore, the platform must guarantee that a privileged software only observes $obs(\sigma_i)$

It does not look like a too complicated property, but math is tricky.
The author decompose the proof into three parts.

- **Secure Measurement**

  The platform must *measure* the program for the user to verify and this measurement should ensure they produce the same trace w.r.t same inputs as long as their measurement is the same.
  
- **Integrity**
  
  The execution of $e$ cannot be affected by a privileged software attacker.
  
- **Confidentiality**

  Two enclaves cannot be distinguished by $obs$ alone.
  
### Secure Measurement
The author here defines a cryptographic function $\mu$ without detailed description.
It satisfies the bijection property.
$
\forall \sigma_1,\sigma_2. init_{e_1}(E_{e_1}(\sigma_1))\wedge init_{e_2}(E_{e_2}(\sigma_2)) \Rightarrow $
$ \mu(e_1)=\mu(e_2) \Leftrightarrow E_{e_1}(\sigma_1)=E_{e_2}(\sigma_2)
$

With this and the determinism property assumed in the definition of $[[e]]$, the secure measurement property is easy.

In math it can be written as the following steps.
  
1. Two traces $\pi_1,\pi_2$ start with the same initial state.
$E_{e_1}(\pi_1[0])=E_{e_2}(\pi_2[0])$
2. Whenever $e_1$ is in enclave, $e_2$ is.
$\forall i. (curr(\pi_1[i])=e_1)\Leftrightarrow(curr(\pi_2[i])=e_2)$
3. Two traces share the same input sequence.
$\forall i. (curr(\pi_1[i])=e_1)\Rightarrow I_{e_1}(\pi_1[i])=I_{e_2}(\pi_2[i])$
4. If the input $I_e$ and current state $\sigma$ is the determined, the next state is determined.

1st property comes from the cryptographic property of $\mu$.

2nd/3rd properties come from the natural inituition. It is ridiculous to require two enclave produce the same trace without the same steps and inputs.

4th property is the **most important property** and comes from the assumption. (not necessary true with multicore)

Then we can use mathmatical induction with the assumption of $[[e]]$ to show their traces are identical.

$
\forall i. E_{e_1}(\pi_1[i])=E_{e_2}(\pi_2[i])\wedge O_{e_1}(\pi_1[i])=O_{e_2}(\pi_2[i])
$
> **Exercise**.  
> Please provide a formal description of the assumption in the definition of $[[e]]$ and gives a proof to show their traces are indeed identical.

### Integrity
Integrity ensures that the execution of the enclave with or without the presence of an attacker.

$
\forall i. E_e(\pi_1[i])=E_e(\pi_2[i])\wedge O_e(\pi_1[i])=O_e(\pi_2[i])
$

As we assume $tamper={(\sigma_1, \sigma_2): E_e(\sigma_1)=E_e(\sigma_2)}$.

The integrity property rightaway follows.

The tampering process is formalized as follows.

![](https://files.mdnice.com/user/23959/e15a645c-e659-4c71-ba6e-993ab09d10fd.png)

As this figure shows, the author set up a simple model of tampering where each one tampering attempt happends between each enclave action.

> **Exercise**  
> Obviously, tampering in real world does not have such good property.  For example, there might be multiple or none tampering steps happends between two enclave actions.  
> Why does this model still reflects the real world scenario?

### Confidentiality
Confidentiality is quite hard to formalize. The author state the following.

> For any two traces that have equivalent attacker operations and equivalent observations of the enclave execution, but possibly different enclave private states and executions, the attacker’s execution (its sequence of states) is identical.

Normally, when considering confidentiality, we consider that for two different traces, the observation $obs$ made by the attacker is the same.

Yet, this is impossible to achieve as the observation includes way too many things. The author uses a weaker model.

> All enclave traces with the same observations, but possibly different enclave states, must be indistinguishable to the attacker.

It allows observation $obs$ as it is inevitable, but apart from observation, two enclave should be indistinguishable.

## Trusted Abstract Platform
The Trusted Abstract Platform (TAP) consists of two parts, *state* and *operation*.

| State | Type |
|-------|------|
|`pc`   |$VA$  |
|`regs` |$\mathbb{N}\rightarrow W$ |
|`mem`  |$PA\rightarrow W$|
|`addr_map`|$VA\rightarrow (ACL\times PA)$|
|`cache`|$(Set\times Way)\rightarrow (\mathbb{B}\times Tag)$|
|`current_eid`|$\mathcal{E}_{id}$|
|`owner`|$PA\rightarrow\mathcal{E}_{id}$|
|`enc_metadata`|$\mathcal{E}_{id}\rightarrow \mathcal{E_M}$|
|`os_metadata`|$\mathcal{E_M}$|

|Operation |Description|
|--------- |-----------|
|`fetch(v)`|Memory operations|
|`load(v)` |**failable** if $ACL$|
|`store(v)`|does not permit|
|`get_map(e,v)`||
|`set_map(e,v,p,perm)`||
|`launch(init,config)`||
|`destroy(e)`||
|`enter(e),resume(e)`||
|`exit(e),pause(e)`||
|`attest(d)`||

> **Note:**  
> The author does not provide a formalization for hardware features like **exception** and assumes the **pause/resume** is atomic.  
> 
> IMHO, this becomes a cover-up for many potential attacks

## Adversary Model
The adversay model of TAP can be divided into two parts, *adversary tampering* and *adversary obervation*.

### Adversary Tampering
The tampering relation can be defined as follows.

1. Unconstraint updates to `pc` and `regs`. This can be modeled by `havoc` statement. (`havoc x` updates `x` with a non-deterministic value)
2. Loads and stores to memory with unconstrained address `va` and data `data` arguments. 
    - $\langle op,hit_f\rangle\leftarrow fetch(va)$
    - $\langle regs[i],hit_l\rangle\leftarrow load(va)$
    - $hit_s\leftarrow store(va,data)$
3. Modify the virtual memory mapping.
    - $set_addr_map(e,v,p,perm)$
    - $regs[i]\leftarrow get_addr_map(e,v)$
4. Enclave management with arbitrary arguments
    - Launch enclaves: $launch(init, config)$
    - Destroy enclaves: $destroy(e)$
    - Enter and resume enclaves: $enter(e)/resumt(e)$
    - Exit and interrupt enclaves: $exit(e)/pause(e)$
    
### Adversary Observation
> You may need some $\lambda$-calculus to read this part.  
> `ITE(if-clause, then, else)` stands for if-then-else.  
> $\perp$ stands for `nil`

**Adversary $M$**.

The weakest adversary can only observe memory that does not belong to an enclave.

$obs_e^M(\sigma)=\lambda p.\text{ITE}(\sigma(owner[p])\neq e,\sigma(mem[p]),\perp)$

**Adversary $MC$**.

With $C$, the adversary gain ability to view whether an **non-enclave** address have been cached before.

$obs_e^M(\sigma)=\lambda p.$
$\text{ITE}(\sigma(owner[p])\neq e,\langle\sigma(mem[p]), cached(\sigma,p)\rangle,\perp)$

**Adversary $MCP$**

With $P$, the adversary can observe the memory mapping of enclaves.

$obs_e^M(\sigma)=\lambda \sigma. \langle obs_e^{MC}(\sigma),\lambda v.\sigma(get\_map(e,v))$

> **Note:**  
> The rest of this paper does not mention the detail of the proof, which seems to be easy.  
> The hard part is to write them in formal language and check them with proof assistant.

## Conclusion
This paper provides a formal foundation for TEEs. However, it neglects many hardware aspects and take atomicity for granted. For example, it omits the proof for multicore systems. Apart from that, it only refines TAP with SGX and Sanctum without discussing *two-world* style TEEs like TrustZone.

## What can we do?
1. Extends this formal model to multicore with memory models.
2. Replace the atomicity assumption with fine-grained exceptions.
3. Formalize *two-world* style TEEs like SGX and TrustZone.