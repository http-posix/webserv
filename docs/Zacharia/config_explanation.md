# Configuration Parser & Tokenizer — Explained

This document explains how the configuration subsystem in `src/config/` works.
It covers two cooperating parts:

- **Tokenizer** (`src/config/tokenizer/`) — turns the raw config file text into a
  stream of tokens.
- **Parser** (`src/config/parser/`) — reads those tokens and builds a structured
  `Config` object tree that the rest of the application can use.

The config format is intentionally *nginx-like*: blocks delimited by `{`/`}`,
directives made of keywords and values, and `#` comment lines.

---

## 1. High-level overview

The end-to-end flow is:

```
config file (text)
      │
      ▼
  readFile()        ── read entire file into a string
      │
      ▼
  removeComments()  ── strip every "# ... \n" line
      │
      ▼
  ConfigTokenizer   ── split the text into a stream of ConfigToken
      │
      ▼
  ConfigParser      ── recursively descend, matching tokens into directives,
      │                building ServerConfig / LocationConfig objects
      ▼
  Config (object tree)  ── returned via getConfig()
```

The parser is a **recursive-descent, single-pass** reader. It walks the token
stream strictly left-to-right and never seeks backwards. It keeps exactly one
token of lookahead (the "current" token) plus a non-consuming peek when needed.

---

## 2. The Tokenizer

Files: `src/config/tokenizer/tokenizer.hpp`, `src/config/tokenizer/tokenizer.cpp`.

### 2.1 Token representation

A token is a simple struct (`tokenizer.hpp:4`):

```cpp
struct ConfigToken
{
    enum TokenType
    {
        Keyword,      // reserved config words
        Identifier,   // any non-keyword word (or fallback for stray chars)
        Symbol,       // single structural char: '{', '}', ':'
        Number,       // a run of digits (kept as text, not yet an int)
        String,       // a quoted literal (quotes stripped)
        EndOfFile     // no more input
    } type;
    std::string value;   // the textual content of the token
};
```

### 2.2 Tokenizer class

`ConfigTokenizer` (`tokenizer.hpp:18`) is a **stateful cursor** over the raw
input string:

| Member | Purpose |
|---|---|
| `src_` | the full raw config text |
| `pos_` | current read index into `src_` |
| `next()` | consume and return the next token, advancing `pos_` |
| `checkNext()` | return the next token *without* advancing `pos_` (lookahead) |
| `isKeyword(s)` | classify a word as a reserved keyword or not |

### 2.3 Tokenization steps (`next()`, `tokenizer.cpp:25`)

For each call, in order:

1. **Skip whitespace** — consume spaces, tabs, newlines.
2. **End of file** — if the cursor is past the end, return `{EndOfFile, ""}`.
   This is the sentinel that ends parsing.
3. **Word (Keyword / Identifier)** — if the char is alphabetic, keep scanning
   while characters are alphanumeric, `_`, or `/` (the slash lets path-like
   values such as `/api/v1` be captured as one token). The result is checked by
   `isKeyword()`; it becomes a `Keyword` token if recognized, else `Identifier`.
4. **Number** — a run of digits becomes a `Number` token. It is kept as text;
   conversion to an integer is deferred to the parser.
5. **String** — a `"..."` or `'...'` literal becomes a `String` token with the
   quotes removed. There is **no escape-sequence handling**.
6. **Symbol** — a single char that is `{`, `}`, or `:` becomes a `Symbol`.
7. **Fallback** — any other single character becomes an `Identifier`. This is a
   safety net (notably `;` falls through here, since it is not a designated
   symbol).

### 2.4 Recognized keywords (`isKeyword`, `tokenizer.cpp:19`)

Twelve reserved words are classified as `Keyword`:

`server`, `location`, `listen`, `error_page`, `client_max_body_size`,
`methods`, `return`, `root`, `autoindex`, `index`, `upload_enable`,
`upload_store`.

### 2.5 Lookahead (`checkNext()`, `tokenizer.cpp:79`)

`checkNext()` runs the *same* scanning logic as `next()` but on a temporary copy
of the cursor, so the real `pos_` is untouched. The parser uses it to inspect the
upcoming token before deciding how to consume it. (Note: `next()` and `checkNext()`
duplicate the scanning code — a candidate for refactoring into one shared
helper.)

### 2.6 Design notes

- **No string escapes** — a backslash before a quote is taken literally; the
  matching closing quote still ends the string.
- **Decimal-only integers** — no negatives, decimals, or hex. A leading `-`
  would tokenize as an `Identifier`; a `.` would split a number into two tokens.
- **Implicit statement terminators** — only `{` `}` `:` are symbols. Statements
  are whitespace/semicolon separated rather than brace-terminated.

---

## 3. The Parser

Files: `src/config/parser/parser.hpp`, `src/config/parser/parser.cpp`.

### 3.1 What it produces

The parser emits a **hierarchy of plain structs** (no separate AST):

```cpp
struct Config {                         // top level
    std::vector<ServerConfig> servers;
};

struct ServerConfig {                   // one "server { ... }" block
    std::string hostname;
    std::vector<uint16_t> listen_port;
    std::unordered_map<int, std::string> error_pages;
    std::string root;
    std::string index;
    std::vector<LocationConfig> locations;
    size_t client_max_body_size = 0;
};

struct LocationConfig {                 // one "location { ... }" block
    std::string uri_path;
    std::vector<std::string> allowed_methods;
    std::pair<int, std::string> redirections;
    std::string root;
    std::string index;
    bool autoindex = false;
    bool upload_enable = false;
    std::string upload_location;
};
```

Containment: a `Config` has one or more `ServerConfig`s; each `ServerConfig` has
zero or more `LocationConfig`s.

### 3.2 Parser state

`ConfigParser` (`parser.hpp:71`) keeps:

| Member | Role |
|---|---|
| `config_` | the `Config` tree being built |
| `file_string_` | raw file text after comment removal |
| `tokenizer_` | the `ConfigTokenizer` instance |
| `pos_` | the **current token** (one-token lookahead buffer) |

### 3.3 Parsing flow (`parseFromString`, `parser.cpp:329`)

1. **Read file** — `readFile()` opens the file and reads it all into
   `file_string_`; throws on failure or empty input.
2. **Remove comments** — `removeComments()` erases every `# ...` line *before*
   tokenization.
3. **Prime tokenizer** — construct the `ConfigTokenizer` from the cleaned text
   and read the first token into `pos_`.
4. **Top level** — loop until `EndOfFile`. Each iteration must see the `server`
   keyword followed by `{`, then calls `createServerConfig()`. Anything else is
   an error. At least one server block is required.
5. **Retrieve** — `getConfig()` returns `const Config&`.

### 3.4 Server blocks — `createServerConfig()` (`parser.cpp:142`)

A loop runs while the current token is not `}`. Each directive is dispatched by
the token's string `value`:

| Directive | Behavior |
|---|---|
| `listen` | expect a `Number`, validate port 1–65535, push to `listen_port`, expect `;` |
| `hostname` | collect tokens up to `;` and join into one string |
| `root` | collect tokens up to `;` and join |
| `index` | collect tokens up to `;` and join |
| `client_max_body_size` | expect a `Number`, then an optional unit (`k`/`kb`/`m`/`mb`/`g`/`gb`) via peek, expect `;` |
| `error_page` | expect a `Number` status (400–599), then path up to `;` |
| `location` | collect tokens up to `{` as the URI, then `createLocationConfig()` |
| *(other)* | throw `ConfigException` |

After the loop it requires at least one `listen` port, consumes the closing `}`,
and pushes the finished `ServerConfig` into `config_.servers`.

### 3.5 Location blocks — `createLocationConfig()` (`parser.cpp:244`)

Same dispatch-on-token pattern, looping until `}`:

| Directive | Behavior |
|---|---|
| `methods` | collect tokens up to `;`, drop commas, validate each is `GET`/`POST`/`DELETE` |
| `return` | expect a `Number` status (300–399), then URL up to `;` → `pair<int,string>` |
| `root` | collect up to `;` and join |
| `index` | collect up to `;` and join |
| `autoindex` | expect `on`/`off`, set boolean, expect `;` |
| `upload_enable` | expect `on`/`off`, set boolean, expect `;` |
| `upload_store` | collect up to `;` and join |
| *(other)* | throw `ConfigException` |

### 3.6 Helper functions (`parser.cpp`)

| Function | Purpose |
|---|---|
| `collectUntil(stop)` | advance and gather token values until a token equals `stop`, then consume `stop` |
| `expect(value)` | assert `pos_.value == value`, then advance |
| `join(values)` | concatenate a vector of strings (no separator) |
| `parseInt(value)` | `std::stol` with full-string-consumed validation |
| `parseSize(value)` | `std::stoul` with full-string validation |
| `applyUnit(value, unit)` | multiply by 1024 / 1024² / 1024³ for k/m/g units (case-insensitive) |

---

## 4. Supported grammar (summary)

```
config       ::= server_block+

server_block ::= 'server' '{' server_directive* '}'
server_dir   ::= 'listen' NUMBER ';'
              |  'hostname' VALUE ';'
              |  'root' PATH ';'
              |  'index' FILE ';'
              |  'client_max_body_size' NUMBER [UNIT] ';'
              |  'error_page' NUMBER PATH ';'
              |  'location' URI '{' location_directive* '}'

location_dir ::= 'methods' METHOD (',' METHOD)* ';'
              |  'return' NUMBER URL ';'
              |  'root' PATH ';'
              |  'index' FILE ';'
              |  'autoindex' ('on' | 'off') ';'
              |  'upload_enable' ('on' | 'off') ';'
              |  'upload_store' PATH ';'

METHOD ::= 'GET' | 'POST' | 'DELETE'
UNIT   ::= 'k' | 'kb' | 'm' | 'mb' | 'g' | 'gb'   (case-insensitive)
# lines starting with '#' are comments
```

---

## 5. How the two pieces interact

- The parser **owns** a `ConfigTokenizer` and a `ConfigToken pos_` holding the
  current token.
- It pulls tokens with `next()` (advancing) and reads `pos_.type` / `pos_.value`
  to decide what to do.
- `expect()` consumes a specific expected token; `collectUntil()` gathers a run
  of tokens up to a delimiter; `checkNext()` peeks without consuming (used, e.g.,
  in `error_page` to confirm the status is not followed by another number).
- The tokenizer is a dumb character scanner; all *meaning* (ranges, units,
  required fields, validation) lives in the parser.

---

## 6. Error handling

All malformed input throws `ConfigException` (a `std::runtime_error` subclass).
Examples enforced by the parser:

- Unterminated block (EOF before `}`).
- `listen` port out of range or missing.
- `error_page` status not in 400–599; `return` status not in 300–399.
- `methods` containing anything other than GET/POST/DELETE.
- Unknown size unit, or a non-numeric value where a number is required.
- Unexpected token at the top level or inside a block.
- Empty config file, or no `server` blocks.
