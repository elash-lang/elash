# Elash Roadmap

## Features To Implement to the 0.1.0 release

### Functions & Variables
- [x] Function definitions
- [x] Function declarations (extern)
- [x] Variable definitions
- [x] Variable declarations (extern)
- [ ] Symbolic aliases

### Control Flow
- [x] If statement
- [x] While loops
- [ ] ~~For loops~~ *(moved to 0.2.0)*

### Type system
- [ ] Strong aliases (typedef)
- [ ] Structs syntactic sugar
- [x] Basic primitive types
- [x] Floating-point types
- [x] Pointer types
- [x] Function types
- [x] Slice types
- [x] Raw slice types
- [x] Array types
- [x] Struct types
- [x] Tuple types
- [ ] ~~Enum types~~ *(moved to 0.2.0)*
- [x] Type casting

### Expressions
- [x] Binary expressions (+, -, *, /, ==, ...)
- [x] Unary expressions (-, !, ~, ...)
- [x] Untyped literals
- [x] Function calls
- [x] Pointer dereference operator
- [x] Array subscript operator
- [x] Member access operator
- [x] Increment and decrement operators
- [x] Variable assignment operator
- [x] Compound assignment operators (+=, -=, ...)
- [ ] String literals
- [ ] Struct initializers

### Compiler CLI
- [x] diagnostics engine
- [x] lower, check, compile, build and inspect commands
- [x] --output/-o flag
- [x] --opt=\<N\>/-O\<N\> flag
- [x] --dump-tokens, --dump-pp-tokens and --dump-ast flags
- [x] --dump-hir, --dump-mir and --dump-lir flags
- [x] --color=never/auto/always flag
- [ ] ~~-I\<scope\>:\<path\> flag~~ *(moved to 0.2.0)*

## Development Roadmap

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
- [ ] ~~Implement file inclusion directive~~ *(moved to 0.2.0)*
- [ ] ~~Resolve relative and module-based include paths~~ *(moved to 0.2.0)*
- [ ] ~~Prevent recursive inclusion~~ *(moved to 0.2.0)*
- [ ] ~~Implement function-like macros~~ *(moved to 0.2.0)*
- [ ] ~~Implement macro expansion engine~~ *(moved to 0.2.0)*
- [ ] ~~Handle nested macro expansion~~ *(moved to 0.2.0)*
- [ ] ~~Preserve correct source locations for diagnostics after expansion~~ *(moved to 0.2.0)*
- [ ] ~~Implement conditional compilation directives~~ *(moved to 0.2.0)*
- [ ] ~~Evaluate constant expressions in conditionals~~ *(moved to 0.2.0)*
- [ ] ~~Support predefined compiler macros~~ *(moved to 0.2.0)*
- [ ] ~~Add preprocessor test suite (includes, macros, conditionals, edge cases)~~ *(moved to 0.2.0)*


### 3. Parser - AST

- [x] Define AST node structures
- [x] Implement expression parser with precedence handling
- [x] Implement variable definitions
- [x] Implement function definitions
- [x] Implement variable declarations (extern)
- [x] Implement function declarations (extern)
- [x] Implement blocks
- [x] Implement control flow statements
- [x] Implement return statements
- [x] Implement top-level/module parsing
- [x] Attach source spans to AST nodes
- [x] Implement error recovery
- [ ] ~~Add parser test suite~~ *(moved to 0.2.0 or later)*


### 4. Semantic Analysis & ELHIR Generation (Binder)

- [x] Implement symbol table (scope)
- [x] Implement scope handling
- [x] Implement built-in functions
- [x] Perform name resolution
- [x] Detect duplicate declarations
- [x] Detect undefined identifiers
- [x] Define internal type representation
- [x] Implement expression type checking
- [x] Validate function calls
- [x] Validate return types
- [x] Implement constant folding
- [ ] Check whether function returns in all execution paths
- [x] Implement implicit and explicit conversions
- [x] Lower AST nodes to ELHIR (Elash High-level Intermediate Representation)
- [ ] ~~Add semantic & ELHIR test suite~~ *(moved to 0.2.0 or later)*
- [x] Attach source spans to HIR nodes

### 5. ELHIR -> ELMIR Lowering

- [x] Define ELMIR (Elash Mid-level Intermediate Representation) structure
- [x] Lower structured control flow (if, loops) into basic blocks and jumps
- [x] Flatten nested expressions into linear ELMIR instructions
- [x] Implement temporary variable generation
- [ ] Perform early ELMIR optimizations (e.g., dead code elimination)
- [ ] ~~Attach source spans to MIR structures~~ *(moved to 0.2.0 or later)*

### 6. ELMIR -> LLVM IR Lowering

- [x] Initialize LLVM context, module, and IR builder
- [x] Define mapping from language types to LLVM types
- [x] Lower primitive types
- [x] Lower function signatures
- [x] Create LLVM function definitions
- [x] Map ELMIR basic blocks to LLVM basic blocks
- [x] Lower local variables and temporaries using alloca in entry block
- [x] Lower ELMIR instructions to LLVM IR (arithmetic, comparisons, jumps)
- [x] Lower function calls
- [x] Lower return instructions
- [x] Handle global variables
- [x] Verify LLVM module
- [x] Emit LLVM IR (.ll)
- [x] Emit object file
- [ ] ~~Attach DWARF debug info~~ *(moved to 0.2.0 or later)*

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

- [x] Implement command parsing
- [x] Implement useful flags
- [ ] Implement optimization flags
- [x] Implement colored diagnostics
- [x] Return proper exit codes

### 10. Standard Library
- [ ] ~~`io` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`fs` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`math` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`collections` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`time` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`rand` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`thread` module~~ *(moved to 0.2.0 or later)*
- [ ] ~~`net` module~~ *(moved to 0.2.0 or later)*


### 11. Stability & Hardening

- [x] End-to-end compilation tests
- [x] Invalid program test suite
- [ ] ~~Fuzz tests~~ *(moved to 0.2.0)*
