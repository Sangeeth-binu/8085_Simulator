# 8085 Assembler Tokenizer Documentation

## Overview

This tokenizer is designed to parse 8085 assembly language source code and convert it into a stream of tokens for further processing by an assembler. It handles various token types including numbers in multiple bases, identifiers, strings, and special characters.

## Features

- **Multiple Number Bases**: Binary (B), Octal (O/Q), Decimal (D), and Hexadecimal (H)
- **String Literals**: Single-quoted string parsing with proper null termination
- **Identifiers**: Assembly mnemonics, labels, and register names
- **Comments**: Semi-colon comment handling with automatic line tracking
- **Memory Safety**: Buffer overflow protection and bounds checking
- **Error Handling**: Comprehensive error reporting with line numbers

## Token Types

```c
enum {
    TOKEN_EOF = EOF,           // End of file (-1)
    TOKEN_NEWLINE = '\n',      // Line separator (10)
    TOKEN_STRING = 3,          // String literal
    TOKEN_IDENTIFIER = 2,      // Labels, mnemonics, registers
    TOKEN_NUMBER = 1,          // Numeric values
    TOKEN_COMMA = ',',         // Parameter separator (44)
    TOKEN_COLON = ':'          // Label delimiter (58)
    TOKEN_LABEL =4
};
```

## Supported Number Formats

| Format | Suffix | Example | Decimal Value |
|--------|--------|---------|---------------|
| Binary | B | `1010B` | 10 |
| Octal | O or Q | `77O` | 63 |
| Decimal | D or none | `255` or `255D` | 255 |
| Hexadecimal | H | `0FFH` | 255 |

## API Reference

### Core Functions

#### `int read_token()`
Reads and returns the next token from the input stream.

**Returns:**
- Token type constant (see enum above)
- Sets global variables `token_buffer`, `token_length`, and `token_number`

**Global Variables Set:**
- `token_buffer[64]`: Contains the raw token characters
- `token_length`: Length of token in buffer
- `token_number`: Numeric value for NUMBER tokens
- `line_number`: Current line being processed

#### `void print_token_info(int token_type)`
Debug function that prints detailed information about a token.

**Parameters:**
- `token_type`: Token type returned by `read_token()`

### Utility Functions

#### `int read_byte()`
Reads a single character from the input file.

#### `void unread_byte()`
Moves file pointer back one position (for lookahead).

#### `bool is_ident(uint8_t c)`
Checks if character is valid for identifiers (alphanumeric or underscore).

#### `void check_token_buffer_size()`
Validates buffer bounds to prevent overflow.

#### `void init_token_buffer()`
Initializes token buffer to all zeros.

## Usage Example

```c
#include "tokenizer.h"

int main() {
    assembly = fopen("program.asm", "r");
    if (!assembly) {
        printf("Error opening file\n");
        return 1;
    }
    
    int token_type;
    int count = 0;
    
    do {
        token_type = read_token();
        printf("Token %d: ", ++count);
        print_token_info(token_type);
    } while (token_type != TOKEN_EOF);
    
    fclose(assembly);
    return 0;
}
```

## Input Format

### Identifiers
- Must start with a letter
- Can contain letters, numbers, and underscores
- Automatically converted to uppercase
- Examples: `MOV`, `LOOP1`, `DATA_ADDR`

### Numbers
- Must start with a digit
- Base determined by suffix character
- Validated for correct digits per base
- Examples: `1010B`, `377O`, `255`, `0ABCH`

### Strings
- Enclosed in single quotes
- Closing quote not included in token content
- Null-terminated automatically
- Example: `'HELLO'` → `HELLO\0`

### Comments
- Start with semicolon (`;`)
- Continue to end of line
- Automatically skipped during tokenization

## Error Handling

The tokenizer provides comprehensive error checking:

### Buffer Overflow Protection
- Prevents writing beyond 64-byte token buffer
- Exits with error message if exceeded

### Number Base Validation
- **Binary**: Only accepts `0` and `1`
- **Octal**: Only accepts `0-7`
- **Hexadecimal**: Only accepts `0-9` and `A-F`

### File Handling
- Checks for successful file opening
- Proper file closure on errors

## Memory Safety

- **Buffer Size**: Fixed 64-byte token buffer with overflow protection
- **Bounds Checking**: All array accesses validated before use
- **Safe Parsing**: Character validation before buffer operations

## Limitations

1. **Token Length**: Maximum 63 characters per token
2. **String Length**: Maximum 63 characters per string literal
3. **Number Range**: Limited by `int` type (typically 32-bit)
4. **File Size**: No explicit limit, but processes sequentially

## Implementation Notes

### Character Handling
- All input converted to uppercase for case-insensitive parsing
- ASCII character comparisons used throughout
- Proper handling of whitespace and control characters

### State Management
- Line numbers automatically tracked
- End-of-line detection and processing
- Comment state handling


## Testing

Create a test file `test.asm`:

```assembly
        ORG 8000H           ; Program origin
START:  MOV A, B            ; Move B to A  
        ADD 0FFH            ; Add hex number
        STA RESULT          ; Store result
        MVI B, 'X'          ; Load character
        JMP START           ; Infinite loop
RESULT: DB 0                ; Data storage
        END
```

Expected token sequence:
1. `ORG` (IDENTIFIER)
2. `32768` (NUMBER - 8000H converted)
3. `START:` (LABEL)
4. `MOV` (IDENTIFIER)
5. `A` (IDENTIFIER)
6. `,` (COMMA)
7. `B` (IDENTIFIER)
8. And so on...

## Future Enhancements

- **Floating Point**: Support for decimal numbers
- **Escape Sequences**: String escape character handling
- **Preprocessor**: Include file and macro support
- **Error Recovery**: Continue parsing after errors
- **Token Buffering**: Lookahead capability for complex parsing