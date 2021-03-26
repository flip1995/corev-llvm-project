# Demo

## `memcpy`

### Explicit

- Show and explain C-code (`memcpy.c`)
1. GCC
    - `memcpy` without extension (should just be a loop)
        - Show objdump
    - `memcpy` with `xcorevhwlp` extension
        - Explain command (options and binary name)
        - Show objdump
2. LLVM
    - `memcpy` with `xcorevhwlp` extension
        - Explain differences in command
        - Show objdump
        - Explain differences

### Implicit

- Show and explain C-code (`implicit-memcpy.c`)
1. GCC
    - implicit `memcpy` with `xcorev` extension
        - Mention the `xcorev` extension
        - Show objdump
        - **Don't** point out the expanded arch string. This is an
          implementation detail / something people expectto Just Work(TM),
          choose your reason for not mentioning it.
2. LLVM
    - implicit `memcpy` with `xcorev` extension
        - Explain why no `-O2`
        - Show objdump and point out differences

## `mac`

### Builtin

#### Fail

- Show and explain C-code (`mac-builtin.c`)
1. GCC
    - Show command and point out, that we don't compile with `xcorevmac`
      enabled.
    - Show error message
2. LLVM
    - Show behavior of LLVM in this case

#### Success

1. GCC
    - Point out, that we're now using `xcorevmac`
    - Show objdump
2. LLVM
    - Compile
    - Show objdump
    - Explain difference and why not `-O2`

### Implicit

- Show and explain C-code (`mac.c`)
1. GCC
    - Compile
    - Show objdump
2. LLVM
    - Compile
    - Show objdump
