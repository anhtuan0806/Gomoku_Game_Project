# C/C++ Code Style Guide (Systems & Data-Oriented)

**Goal:** To ensure high-performance, memory-transparent, and maintainable code by strictly separating data structures from logic, enforcing explicit initialization, and ensuring structural clarity.

---

## 1. Naming Conventions

| Entity                  | Rule                             | Example                                              |
| :---------------------- | :------------------------------- | :--------------------------------------------------- |
| **Files**               | `PascalCase`                     | `AnhBanh.cpp`, `MemoryPool.hpp`                      |
| **Namespaces**          | `PascalCase`                     | `CoreLogic`, `MathUtils`, `SecurityLayer`            |
| **Structs**             | `PascalCase`                     | `struct UserSession`, `struct DataPacket`            |
| **Type Aliases**        | `PascalCase`                     | `using Byte = uint8_t;`, `using Vector3 = float[3];` |
| **Functions**           | `camelCase` (Verb-first)         | `calculateTotal()`, `initBuffer()`                   |
| **Global Variables**    | `g` + `camelCase`                | `gIsGameOver`, `gBoardMatrix`                        |
| **File-Scope (Static)** | `s` + `camelCase`                | `sFrameRate`, `sLocalCounter`                        |
| **Variables / Members** | `camelCase`                      | `sampleCount`, `bufferPtr`, `isActive`               |
| **Booleans**            | Interrogative (`is`/`has`/`can`) | `isWinner`, `hasMarked`, `canUndo`                   |
| **Constants / Macros**  | `UPPER_SNAKE_CASE`               | `MAX_CAPACITY`, `BOARD_SIZE`[cite: 1]                |

> **Note on Clarity:** Names for variables and functions must be descriptive. For boolean types, always use a question format (e.g., `isProcessed`) to clarify conditional logic. Avoid cryptic abbreviations.

---

## 2. Data & Logic Structure (The "No Class" Rule)

- **FORBIDDEN: The `class` keyword.** All entities must be defined using `struct`.
- **Struct Rules:**
  - Contains **only** data members (attributes).
  - **NO** member functions, constructors, or destructors allowed inside the struct.
  - **NO** access specifiers (`public`, `private`, `protected`). All members are implicitly public.
- **Logic Management:** All logic must be implemented as **Free Functions** within a namespace. These functions should take the struct as a parameter (usually via reference or pointer).

---

## 3. Declarations & Control Flow

### Variable Declarations

- **One Per Line:** Each variable must be declared on its own line to improve readability and version control diffs.
- **Mandatory Initialization:** Every variable **must** be assigned a default value at the time of declaration to prevent undefined behavior from uninitialized memory.
- **Scope Prefixes:** Use `g` for variables with global visibility and `s` for static variables limited to file scope.

### Control Structures

- **Mandatory Braces:** Curly braces `{}` are required for all control blocks (e.g., `if`, `else`, `for`, `while`, `do`), even if the block contains only a single statement[cite: 1].

---

## 4. Namespace & Type Aliasing

- **BANNED: `using namespace`:** Never use `using namespace std;` in the global scope or within header files. Use explicit prefixes (e.g., `std::vector`).
- **Type Aliasing:** Use the `using` keyword instead of `typedef` for defining complex types or fixed-width integers (from `<cstdint>`).

---

## 5. Tooling & Formatting

- **Indentation:** 4 spaces, no tabs.
- **Line Length:** 88–100 characters.
- **Memory Safety:** Use `Valgrind` or `AddressSanitizer` to ensure that any manually allocated resources are properly freed via "destroy" functions.
