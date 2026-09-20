# Elash v0.2.0 Roadmap

## Language Features
### Statements
- [x] Init-statements in `if` and `while`
- [ ] Add `for (init; cond; post)` loop
- [x] Initializers as `return` values
- [ ] Warn when the result or an expression without side effects is ignored
- [ ] Support `if case` and `while case` for optionals

### Declarations
- [ ] Support unnamed function params in declarations
- [x] Multiple declarators in a single declaration (comma separated, e.g. `int x, y;`)
- [x] Forward-declared typedefs / opaque types (e.g. `typedef Name;`), better incomplete types semantics

### Type system
- [ ] ~~Enum types~~ *(moved to 0.3.0)*
- [ ] ~~Union types~~ *(moved to 0.3.0)*
- [x] Optional types
- [ ] Read-only types
- [ ] Write-only types

### Expressions
- [x] Bitcast operator (`bitcast`)

- [x] Optional fallback operator (`??`)
- [x] Optional map operator (`?>`)
- [x] Optional unwrap operator (`!`)
- [x] Optional member operator (`?.`)

- [x] Make string literals untyped
- [x] Support passing types as function arguments (for builtins)
- [x] Support escape sequences in string and char literals

### Builtins
- [x] Support passing array types directly in len() function
- [x] Add `sizeof` function
- [x] Add `alignof` function
- [ ] ~~Add `offsetof` function~~ *(moved to 0.3.0 or later)*

### Driver/CLI
- [x] `-I[src/sys] <name>=<path>` flag
- [x] `--no-corelib` and `--no-stdlib` flags
- [ ] ~~Predefined modes (`release`, `debug`)~~ *(moved to 0.3.0)*
- [ ] ~~Invoke system linker~~ *(moved to 0.3.0)*
- [ ] ~~Produce final executable~~ *(moved to 0.3.0)*
- [ ] Don't report warnings from system headers

## Preprocessor
- [x] Implement file inclusion directive
- [x] Resolve scoped and local include paths
- [x] Max include depth limit
- [ ] Implement embed directive
- [x] Solve source spans issues with #include
- [x] Support preprocessor variables and constants
- [x] Support preprocessor functions
- [ ] Implement macro expansion engine
- [ ] Implement macro expansion result rescan
- [ ] Preserve correct source locations after expansion
- [x] Implement conditional compilation directives
- [ ] Implement loops (`#while`, `#for`)
- [ ] Support predefined preprocessor functions and macros
- [ ] ~~Builtin constants (`ELC_MODE`, `ELC_VERSION`, `ELC_OPTLVL`)~~ *(moved to 0.3.0)*

## Library
### Corelib
- [ ] ~~`str` module~~ *(moved to 0.3.0)*

### Stdlib
- [ ] ~~`io` module~~ *(moved to 0.3.0)*
- [ ] ~~`mem` module~~ *(moved to 0.3.0)*

## Lowering
- [ ] ~~Perform early MIR optimizations~~ *(moved to 0.3.0)*
- [ ] ~~Attach DWARF debug info~~ *(moved to 0.3.0)*
- [ ] ~~Attach source spans to MIR structures~~ *(moved to 0.3.0)*

### Optimizations
#### LLVM passes
- [x] Setup optimizations infrastructure
- [x] Configure pass manager
- [x] Define optimization levels (O0, O1, O2)
- [x] Run optimization pipeline
- [x] Verify module after passes

#### Frontend
- [x] Lower `T&?` and `T[&]?` to just `T*` in MIR

## Documentation
### Compiler internals
- [x] Lexer docs
- [ ] ~~Preproc docs~~ *(moved to 0.3.0 or later)*
- [ ] ~~Parser docs~~ *(moved to 0.3.0 or later)*
- [ ] ~~Binder docs~~ *(moved to 0.3.0 or later)*
- [ ] ~~Lowerer docs~~ *(moved to 0.3.0 or later)*
- [x] Source doc docs

### The Language
- [ ] Setup initial language documentation site
- [ ] Document elash basics

## Stability & Hardening
- [x] Support multiple source files in e2e cases
- [ ] Source doc test suite
- [x] Preproc test suite
- [x] Parser test suite
- [x] Unparser test suite
- [ ] ~~Binder test suite~~ *(moved to 0.3.0 or later)*
- [ ] ~~Lowerer test suite~~ *(moved to 0.3.0 or later)*
- [x] Fuzz tests

## Benchmarking & Profiling
- [ ] End-to-end benchmark suite
- [x] Timers for individual pipeline stages
- [x] Add `--time-report[=file]` flag to enable timers and display results
- [x] Support human readable and machine readable (`jsonl`) output format via
