# Elash Developement Roadmap

## 0. Project Setup

- [x] Initialize repository
- [x] Configure build system
- [x] Add LLVM C bindings as dependency
- [x] Create compiler entry point
- [x] Implement source file loader
- [x] Implement diagnostic system (line/column + spans)
- [ ] Setup automated tests


## 1. Lexer

- [x] Define token types
- [x] Implement identifier and keyword recognition
- [x] Implement numeric literals (int, float)
- [x] Implement string literals (with escapes)
- [x] Implement operator parsing (single and multi-character)
- [x] Implement comment handling
- [x] Attach position metadata to tokens
- [ ] Add lexer test suite

## 2. Preprocessor

- [x] Define preprocessor directive syntax
- [x] Implement preprocessing phase as a token-stream transform
- [ ] Implement file inclusion directive
- [ ] Resolve relative and module-based include paths
- [ ] Prevent recursive inclusion
- [ ] Implement function-like macros
- [ ] Implement macro expansion engine
- [ ] Handle nested macro expansion
- [ ] Preserve correct source locations for diagnostics after expansion
- [ ] Implement conditional compilation directives
- [ ] Evaluate constant expressions in conditionals
- [ ] Support predefined compiler macros
- [ ] Add preprocessor test suite (includes, macros, conditionals, edge cases)


## 3. Parser - AST

- [x] Define AST node structures
- [x] Implement expression parser with precedence handling
- [ ] Implement variable declarations
- [x] Implement function declarations
- [x] Implement blocks
- [ ] Implement control flow statements
- [x] Implement return statements
- [x] Implement top-level/module parsing
- [x] Attach source spans to AST nodes
- [ ] Implement error recovery
- [ ] Add parser test suite


## 4. Semantic Analysis & ELHIR Generation (Binder)

- [x] Implement symbol table (scope)
- [x] Implement scope handling
- [ ] Implement built-in functions
- [x] Perform name resolution
- [x] Detect duplicate declarations
- [x] Detect undefined identifiers
- [x] Define internal type representation
- [ ] Implement expression type checking
- [ ] Validate function calls
- [ ] Validate return types
- [ ] Implement implicit and explicit conversions
- [ ] Lower AST nodes to ELHIR (Elash High-level Intermediate Representation)
- [ ] Add semantic & ELHIR test suite


## 5. ELHIR -> ELMIR Lowering

- [x] Define ELMIR (Elash Mid-level Intermediate Representation) structure
- [ ] Implement Control Flow Graph (CFG) construction
- [ ] Lower structured control flow (if, loops) into basic blocks and jumps
- [x] Flatten nested expressions into linear ELMIR instructions
- [x] Implement temporary variable generation
- [ ] Perform early ELMIR optimizations (constant folding, dead code elimination)


## 6. ELMIR -> LLVM IR Lowering

- [ ] Initialize LLVM context, module, and IR builder
- [ ] Define mapping from language types to LLVM types
- [ ] Lower primitive types
- [ ] Lower function signatures
- [ ] Create LLVM function definitions
- [ ] Map ELMIR basic blocks to LLVM basic blocks
- [ ] Lower local variables and temporaries using alloca in entry block
- [ ] Lower ELMIR instructions to LLVM IR (arithmetic, comparisons, jumps)
- [ ] Lower function calls
- [ ] Lower return instructions
- [ ] Handle global variables
- [ ] Verify LLVM module
- [ ] Emit LLVM IR (.ll)
- [ ] Emit object file


## 7. Linking

- [ ] Configure LLVM target triple
- [ ] Create target machine
- [ ] Emit object files
- [ ] Invoke system linker
- [ ] Produce final executable


## 8. Optimizations (LLVM Passes)

- [ ] Configure pass manager
- [ ] Define optimization levels (O0, O1, O2)
- [ ] Run optimization pipeline
- [ ] Verify module after passes
- [ ] Benchmark optimized vs non-optimized builds


## 9. CLI

- [ ] Implement command parsing
- [ ] Implement useful flags
- [ ] Implement optimization flags
- [ ] Implement colored diagnostics
- [ ] Return proper exit codes


## 10. Modules & Multi-file Compilation

- [ ] Implement import resolution
- [ ] Support multi-file parsing
- [ ] Share symbol table across modules
- [ ] Implement symbol visibility rules
- [ ] Detect circular imports
- [ ] Merge modules into single LLVM module before emission


## 11. Standard Library
- [ ] `io` module
- [ ] `fs` module
- [ ] `math` module
- [ ] `collections` module
- [ ] `time` module
- [ ] `rand` module
- [ ] `thread` module
- [ ] `net` module


## 12. Stability & Hardening

- [ ] End-to-end compilation tests
- [ ] Invalid program test suite
- [ ] Large file compilation test
- [ ] Memory usage checks
- [ ] Regression tests
