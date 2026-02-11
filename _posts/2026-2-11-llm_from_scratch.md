---
title: LLM from Scratch (TinyLLaMA)
---

## Introduction

I've been wanting to learn about the LLM for quite a long time, but didn't have
the time to do so until now. So, I decided to implement a simple LLM using my
way of learning, which is to implement it from scratch.

### Ground Rules

I have set some ground rules for myself to follow while implementing the LLM:

- No wheels: I will implement everything (except basic ops like `matmul`) from
scratch.
- Actual LLM: I will aim to implement inference loop for TinyLLaMA, an actual LLM.
- Understand math, not code: I will study the math behind the LLM.

### Table of Contents

I will be covering the following topics:

* TOC
{:toc}

## Tokenization: From Text to Tokens

Languages like English are formed from basic semantic units -- words. For LLMs
to understand and process language, we also need to define a set of basic
semantic units -- LLM's vocabulary. An intuitive choice for these units would
be directly using words. However, this approach has an obvious problem: LLMs
wouldn't be able to recognize any word that is not in its vocabulary, such as
*phishy*. To solve this problem, LLMs like TinyLLaMA use a more delicate algorithm
named Byte Pair Encoding (BPE) to tokenize words into subword units.

The idea behind BPE is that *if certain subword units appear adjacent to each other
frequently, then they are likely to be semantically related and thereby can be
merged.*

Suppose we have the following sentence.

> FloydHub is the fastest way to build, train and deploy deep learning models. Build deep learning models in 
> the cloud. Train deep learning models.


BPE will start from the most basic unit -- letters, and gradually merge adjacent units that appear frequently together. So, in the beginning, BPE compute the frequency of each adjacent pair of letters, yielding the following table.


| Key | Count | Key | Count | Key | Count |
|-----|------|-----|------|-----|------|
| de | 7 | in | 6 | ep | 4 |
| lo | 3 | ee | 3 | le | 3 |
| ea | 3 | ar | 3 | rn | 3 |
| ni | 3 | ng | 3 | mo | 3 |
| od | 3 | el | 3 | ls | 3 |

So, we can merge the most frequent pair `de`, treat it as a single unit, and
represent it as `X`. Then we get the following sentence.

> FloydHub is the fastest way to build, train and Xploy Xep learning moXls. Build Xep learning moXls in 
> the cloud. Train Xep learning moXls.

Then we can repeat the same process, merging the most frequent pair into new
units, until we have reached a preferred vocabulary size $$N$$. In particular,
if we don't set a limit on the vocabulary size, we can keep merging until every
unit is a *word* again, which is the case of using words as tokens.

**Why this makes sense?**

Though beginning with cruel statistics, BPE can effectively capture the
semantic relationship between subword units. For example, in the above
sentence, `tion` appeared in `construction`, `location`, and `...tion` has the
same semantic meaning of being a noun suffix. So, if there are sufficiently
many words that share the same suffix, BPE will eventually merge the suffix
into a single unit, which can be easily recognized by the LLM as a noun suffix.

As TinyLLaMA is trained on a large corpus of text, we are unable to reproduce
its tokenization process. We thereby reuse `autotokenizer` from HuggingFace to
tokenize the input text into tokens.

```python
tokenizer = AutoTokenizer.from_pretrained("TinyLlama/TinyLlama-1.1B-Chat-v1.0", cache_dir="./")
print("--- Test Tokenizer ---")
tokenized = tokenizer("Hello, World!")
tokens, mask = tokenized["input_ids"], tokenized["attention_mask"]
print(f"Hello World! ===> {tokens}")
```

## Embeddings: From Tokens to Vectors

In the language we use from day to day, we have letters, words. In the world of
AI models, however, we only have numbers and vectors. So, for LLMs to
understand and process language, we need to convert words into numbers. This is
where embeddings come in; it is essentially a function that maps tokens to vectors.
For example, we can have a function $$f$$ that maps the token `Love` to a numerical vector.

$$f(Love) = [0.2, 0.5, 0.7...]$$

Note that the embedding function is *not* simply giving every token a random
vector. Instead, it is designed to capture the semantic relationship between
tokens. The following figure illustrates this idea; the word `France` is mapped
to a vector that is close to the vector of `Paris`, while `Germany` is mapped
to a vector that is close to `Berlin`. This way, the LLM can understand the
semantic relationship between these words through their embeddings.

<div align="center"><img width="60%" src="https://upload.wikimedia.org/wikipedia/commons/thumb/f/fe/Word_embedding_illustration.svg/1920px-Word_embedding_illustration.svg.png"></div>

Since embeddings is simply a function that maps tokens to vectors, its implementation is straightforward.

```python
print("--- Test Embedding ---")
embedding = Embedding(weights.get_tensor("model.embed_tokens.weight"))
embedded = embedding[tokens]
print(f"Hello World! ===> {embedded.shape}")
```


## Attention: Where Magic Happens!
