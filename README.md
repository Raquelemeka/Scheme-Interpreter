# Scheme-Interpreter

A simple Scheme interpreter implemented in C, supporting basic evaluation of Scheme expressions.

## Features

- **Tokenizer**: Converts input Scheme code into tokens
- **Parser**: Builds a parse tree from tokens
- **Interpreter**: Evaluates Scheme expressions
- **Memory Management**: Includes a tracking allocator (`talloc`) for automatic memory cleanup
- **Linked List**: Custom implementation used throughout the interpreter

## Files

- `main.c`: Entry point of the interpreter
- `tokenizer.[ch]`: Tokenizes input Scheme code
- `parser.[ch]`: Parses tokens into an abstract syntax tree
- `interpreter.[ch]`: Evaluates Scheme expressions
- `linkedlist.[ch]`: Custom linked list implementation
- `talloc.[ch]`: Tracking memory allocator
- `schemeval.h`: Common type definitions for Scheme values

## Building

The project includes a `justfile` for building. To build:

```bash
just
```

## Usage

Run the interpreter:

```bash
./scheme
```

The interpreter will read Scheme expressions from stdin and evaluate them.

## Memory Management

The interpreter uses a tracking allocator (`talloc`) that:
- Keeps track of all allocated memory
- Provides automatic cleanup via `tfree`
- Includes `texit` for clean program termination

## Example

```scheme
> (+ 1 2 3)
6
> (define x 5)
> (* x 2)
10
```

## Implementation Notes

- The interpreter uses a recursive evaluation model
- Scheme values are represented as a tagged union (defined in `schemeval.h`)
- The linked list implementation is specialized for Scheme's cons cells

## License

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
 
