# Simple Constant Propagation (LLVM Plug‑in)

A minimal out‑of‑tree **LLVM new‑PM** pass that performs two textbook optimisations:

1. **Arithmetic constant folding**

   * `x + 0 → x`, `x * 1 → x`, etc.
2. **Redundant ϕ‑node elimination**

   * Removes a ϕ when *all* incoming values are identical.

It’s purposely tiny for self teaching:

* hook into the LLVM **PassBuilder** plug‑in interface,
* use `llvm::PatternMatch`, and
* manipulate SSA safely (`replaceAllUsesWith`, `eraseFromParent`).

---

## Directory layout

```text
.
├── CMakeLists.txt          
├── SimpleConstProp.cpp      # the entire optimisation pass
├── build/                   # out‑of‑source CMake/Ninja artefacts (git‑ignored)
└── build.sh                 # convenience wrapper for one‑command builds
```

---

## Prerequisites

| Tool             | Notes                                                                                                                      |
| ---------------- | -------------------------------------------------------------------------------------------------------------------------- |
| **LLVM ≥ 17**    | Easiest path is Homebrew on macOS: `brew install llvm`. The plug‑in and the `opt` tool **must be the same major version**. |
| **CMake ≥ 3.16** | Used to detect LLVM and generate Ninja files.                                                                              |
| **Ninja**        | Faster builds. `brew install ninja`.                                                                                       |

Tested on:

```
macOS 15.4  (Apple M‑series)
LLVM 20.1.5 (Homebrew)
CMake 3.29 • Ninja 1.11
```

---

## Build & install (out‑of‑tree)

```bash
# 0) Clone the repo and enter it
$ git clone https://github.com/HCPhy/redundant-phi.git
$ cd redundant-phi

# 1) Configure the project
$ ./build.sh                # runs the cmake command below
# ↳ internally:
# rm -rf build/*
# cd build

# cmake -G Ninja \
#   -DLLVM_DIR=$(/opt/homebrew/opt/llvm/bin/llvm-config --cmakedir) \
#   -DCMAKE_BUILD_TYPE=Release ..

# ninja SimpleConstProp       

# 2) The plug‑in now lives at
$ ls -l build/SimpleConstProp.dylib  # (Linux: .so)
```

---

## Quick smoke test

```bash
# 1. Create a tiny C snippet with trivial folds
cat > test.c <<'EOF'
int foo(int x, int c) {
  int y = x + 0;      // ← add with zero
  if (c)
    y = y * 1;        // ← mul with one
  else
    y = y * 1;
  return y;
}
EOF

# 2. Lower to LLVM IR **without optnone** so passes can run
clang -O0 -Xclang -disable-O0-optnone -emit-llvm -S test.c -o test.ll

# 3. Promote allocas to SSA form; you'll see phi-nodes instead of loads/stores
opt -passes='mem2reg' -S test.ll -o mem2reg.ll

# 4a. Run the SimpleConstProp plug-in on the raw IR
/opt/homebrew/opt/llvm/bin/opt \
    -load-pass-plugin ./build/SimpleConstProp.dylib \
    -passes='simple-constprop' \
    -S test.ll -o out_raw.ll

# 4b. Run the plug-in on the SSA IR produced by mem2reg

/opt/homebrew/opt/llvm/bin/opt \
    -load-pass-plugin ./build/SimpleConstProp.dylib \
    -passes='simple-constprop' \
    -S mem2reg.ll -o out_ssa.ll



# 5. Inspect the diffs – the redundant add/mul instructions should be gone
diff -u test.ll     out_raw.ll | less
diff -u mem2reg.ll  out_ssa.ll | less


```

A similar diff appears if you compile with `-O1` (LLVM will introduce SSA & ϕs for you).


---

## License

This code is released under the Apache‑2.0 License with LLVM exception, to be compatible with upstream LLVM tooling.  See `LICENSE` for full terms.

---
