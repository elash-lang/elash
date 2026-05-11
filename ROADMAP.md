# Elash Roadmap

## Features To Implement to the 0.1.0 release

### Functions & Variables
- [x] Function definitions
- [ ] Function declarations (extern)
- [ ] Variable definitions
- [ ] Variable declarations (extern)

### Control Flow
- [ ] If statement
- [ ] While statement

### Type system
- [x] Basic primitive types
- [x] Pointer types
- [x] Function types
- [ ] Array types

### Expressions
- [x] Binary expressions (+, -, *, /, ==, ...)
- [x] Unary expressions (-, !, ~, ...)
- [x] Function calls
- [ ] Pointer dereference operator
- [ ] Increment and decrement operators
- [ ] Variable assignment operator
- [ ] Compound assignment operators (+=, -=, ...)

## Developement Roadmap

### 0. Project Setup

- [x] Initialize repository
- [x] Configure build system
- [x] Add LLVM C bindings as dependency
- [x] Create compiler entry point
- [x] Implement source file loader
- [x] Implement diagnostic system (line/column + spans)
- [x] Setup automated tests


### 1. Lexer

- [x] Define token types
- [x] Implement identifier and keyword recognition
- [x] Implement numeric literals (int, float)
- [x] Implement string literals (with escapes)
- [x] Implement operator parsing (single and multi-character)
- [x] Implement comment handling
- [x] Attach position metadata to tokens
- [x] Add lexer test suite

### 2. Preprocessor

- [x] Define preprocessor directive syntax
- [x] Implement preprocessing phase as a token-stream transform
- [ ] Implement file inclusion directive
- [ ] Resolve relative and module-based include paths
- [ ] Prevent recursive inclusion
- [ ] ~~Implement function-like macros~~ *(moved to 0.2.0)*
- [ ] ~~Implement macro expansion engine~~ *(moved to 0.2.0)*
- [ ] ~~Handle nested macro expansion~~ *(moved to 0.2.0)*
- [ ] ~~Preserve correct source locations for diagnostics after expansion~~ *(moved to 0.2.0)*
- [ ] ~~Implement conditional compilation directives~~ *(moved to 0.2.0)*
- [ ] ~~Evaluate constant expressions in conditionals~~ *(moved to 0.2.0)*
- [ ] ~~Support predefined compiler macros~~ *(moved to 0.2.0)*
- [ ] Add preprocessor test suite (includes, macros, conditionals, edge cases)


### 3. Parser - AST

- [x] Define AST node structures
- [x] Implement expression parser with precedence handling
- [ ] Implement variable definitions
- [x] Implement function definitions
- [ ] Implement variable declarations (extern)
- [ ] Implement function declarations (extern)
- [x] Implement blocks
- [ ] Implement control flow statements
- [x] Implement return statements
- [x] Implement top-level/module parsing
- [x] Attach source spans to AST nodes
- [ ] Implement error recovery
- [ ] Add parser test suite


### 4. Semantic Analysis & ELHIR Generation (Binder)

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


### 5. ELHIR -> ELMIR Lowering

- [x] Define ELMIR (Elash Mid-level Intermediate Representation) structure
- [ ] Implement Control Flow Graph (CFG) construction
- [ ] Lower structured control flow (if, loops) into basic blocks and jumps
- [x] Flatten nested expressions into linear ELMIR instructions
- [x] Implement temporary variable generation
- [ ] Perform early ELMIR optimizations (constant folding, dead code elimination)


### 6. ELMIR -> LLVM IR Lowering

- [x] Initialize LLVM context, module, and IR builder
- [x] Define mapping from language types to LLVM types
- [x] Lower primitive types
- [x] Lower function signatures
- [x] Create LLVM function definitions
- [x] Map ELMIR basic blocks to LLVM basic blocks
- [ ] Lower local variables and temporaries using alloca in entry block
- [ ] Lower ELMIR instructions to LLVM IR (arithmetic, comparisons, jumps)
- [x] Lower function calls
- [x] Lower return instructions
- [x] Handle global variables
- [ ] Verify LLVM module
- [ ] Emit LLVM IR (.ll)
- [ ] Emit object file


### 7. Linking

- [x] Configure LLVM target triple
- [x] Create target machine
- [x] Emit object files
- [ ] Invoke system linker
- [ ] Produce final executable


### 8. Optimizations (LLVM Passes)

- [ ] ~~Configure pass manager~~ *(moved to 0.2.0)*
- [ ] ~~Define optimization levels (O0, O1, O2)~~ *(moved to 0.2.0)*
- [ ] ~~Run optimization pipeline~~ *(moved to 0.2.0)*
- [ ] ~~Verify module after passes~~ *(moved to 0.2.0)*
- [ ] ~~Benchmark optimized vs non-optimized builds~~ *(moved to 0.2.0)*


### 9. CLI

- [ ] Implement command parsing
- [ ] Implement useful flags
- [ ] Implement optimization flags
- [ ] Implement colored diagnostics
- [ ] Return proper exit codes


### 10. Modules & Multi-file Compilation

- [ ] ~~Implement import resolution~~ *(moved to 0.2.0 or later)*
- [ ] ~~Support multi-file parsing~~ *(moved to 0.2.0 or later)*
- [ ] ~~Share symbol table across modules~~ *(moved to 0.2.0 or later)*
- [ ] ~~Implement symbol visibility rules~~ *(moved to 0.2.0 or later)*
- [ ] ~~Detect circular imports~~ *(moved to 0.2.0 or later)*
- [ ] ~~Merge modules into single LLVM module before emission~~ *(moved to 0.2.0 or later)*


### 11. Standard Library
- [ ] ~~`io` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`fs` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`math` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`collections` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`time` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`rand` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`thread` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`net` module~~ *(moved to 0.2.0 or later)*


### 12. Stability & Hardening

- [ ] End-to-end compilation tests
- [ ] Invalid program test suite
- [ ] Large file compilation test
- [ ] Memory usage checks
- [ ] Regression tests
