# Elash v0.2.0 Roadmap

## Language Features
### Statements
- [ ] Init-statements in `if` and `while`
- [ ] Post-iteration statement in `while` loop
- [x] Initializers as `return` values

### Type system
- [ ] Enum types
- [ ] ~~Union types~~ *(moved to 0.3.0)*
- [ ] Optional types
- [ ] Read-only types
- [ ] Write-only types

### Operators
- [ ] Sizeof operator (`sizeof`)
- [ ] Alignof operator (`alignof`)
- [ ] Offsetof operator (`offsetof`)

- [ ] Bitcast operator (`bitcast`)

- [ ] Optional fallback operator (`??`)
- [ ] Optional map operator (`?:`)
- [ ] Optional unwrap operator (`!`)
- [ ] Optional member operator (`?.`)

### Driver/CLI
- [x] `-I[src/sys] <name>=<path>` flag
- [x] `--no-corelib` and `--no-stdlib` flags
- [ ] Predefined modes (`release`, `debug`)
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
- [ ] Support preprocessor functions
- [ ] Implement macro expansion engine
- [ ] Implement macro expansion result rescan
- [ ] Preserve correct source locations after expansion
- [x] Implement conditional compilation directives
- [ ] Implement loops (`#while`, `#for`)
- [ ] Support predefined preprocessor functions and macros
- [ ] Builtin constants (`ELC_MODE`, `ELC_VERSION`, `ELC_OPTLVL`)

## Library
### Corelib
- [ ] `str` module

### Stdlib
- [ ] `io` module
- [ ] `mem` module

## Lowering
- [ ] Perform early MIR optimizations
- [ ] Attach DWARF debug info
- [ ] Attach source spans to MIR structures

### Optimizations
#### LLVM passes
- [x] Setup optimizations infrastructure
- [x] Configure pass manager
- [x] Define optimization levels (O0, O1, O2)
- [x] Run optimization pipeline
- [x] Verify module after passes

#### Frontend
- [ ] Lower `T&?` and `T[&]?` to just `T*` in MIR
- [ ] Omit null checks in release mode

## Documentation
### Compiler internals
- [x] Lexer docs
- [ ] Preproc docs
- [ ] Parser docs
- [ ] Binder docs
- [ ] Lowerer docs
- [ ] Srcdoc docs

### The Language
- [ ] Setup initial language documentation site
- [ ] Document elash basics

## Stability & Hardening
- [ ] Support multiple source files in e2e cases
- [ ] Preproc test suite
- [x] Parser test suite
- [x] Unparser test suite
- [ ] Binder test suite
- [ ] Lowerer test suite
- [x] Fuzz tests
