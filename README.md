# class_db

A simple, **in-memory database implementation in C**, using a basic **Binary Search Tree (BST)** as an index (stubbed as a B-Tree API for future upgrades).
This project serves as a minimal educational example of a database with basic CRUD operations (Create, Read, Update, Delete) via a text-based SQL-like command interface.

> **Note:** Persistence to disk is not yet implemented. The database starts empty each run, and all data is lost on exit.

---

## 🤩 Features

* **Data Structure:** Uses a BST for indexing rows by ID (primary key).
  Each row is a simple struct:

  ```c
  struct Row {
      int32_t id;
      char name[64];
  };
  ```

* **Commands Supported:**

  | Command                                   | Description                                       |
  | ----------------------------------------- | ------------------------------------------------- |
  | `INSERT INTO table VALUES ('id','name');` | Insert a new row (rejects duplicates on ID).      |
  | `SELECT * FROM table WHERE id=*;`         | Select and print a row by ID.                     |
  | `SELECT * FROM table WHERE name='name';`  | Select and print rows by name (case-insensitive). |
  | `SELECT * FROM table;`                    | Print all rows.                                   |
  | `DELETE FROM table WHERE id=*;`           | Delete a row by ID.                               |
  | `.exit`                                   | Quit the program.                                 |

* **Error Handling:** Basic checks for duplicates, not-found rows, and parse errors.

* **Testing:** Includes a basic unit test (`tests/test_basic.c`) for insert/select/delete.

* **Build System:** Makefile with sanitizer options (`AddressSanitizer`, `UBSan`) and test targets.

---

## 🗂️ Project Structure

```
.
├── include/
│   ├── btree.h        # B-Tree (BST) API declarations
│   └── db.h           # Database Table and Row definitions
├── src/
│   ├── btree.c        # B-Tree (BST) implementation
│   ├── db.c           # Table operations (open, close, insert, select, delete)
│   ├── main.c         # Main entry point and REPL loop
│   └── parser.c       # Command parser for INSERT/SELECT/DELETE
├── tests/
│   └── test_basic.c   # Basic unit tests
├── Makefile           # Build rules
└── README.md          # Project documentation
```

---

## ⚙️ Building

Requires **GCC** (or any compatible C compiler).
No external dependencies.

### Build the main binary:

```bash
make
```

### Build with sanitizers (debug mode):

```bash
make SAN=1
```

### Clean build artifacts:

```bash
make clean
```

The output binary will be:

```
./class_db
```

---

## 💻 Running

Run the program (optionally specify a filename — ignored for now):

```bash
./class_db [optional_filename]
```

This starts an interactive REPL:

```
class_db ready. Type commands like:
  INSERT INTO table VALUES ('id','name')
  SELECT * FROM table WHERE id=*
  SELECT * FROM table
  DELETE FROM table WHERE id=*
  .exit
>
```

---

## 🧠 Example Session

```
> INSERT INTO table VALUES (1,'alice');
OK
> INSERT INTO table VALUES (2,'bob');
OK
> SELECT * FROM table WHERE id=1;
1,'alice'
> SELECT * FROM table WHERE name='bob';
2,'bob'
> SELECT * FROM table;
1,'alice'
2,'bob'
> DELETE FROM table WHERE id=1;
DELETED
> SELECT * FROM table;
2,'bob'
> .exit
```

### Notes

* Values can be **quoted** (`'value'`) or **unquoted** (`plain words/numbers`).
* Duplicate IDs are rejected (`ERROR: duplicate primary key`).
* `SELECT` by name performs a **full scan** (O(n)).
* Commands are **case-insensitive** (`select`, `SELECT`, and `Select` all work).

---

## 🧪 Testing

Build and run the included tests:

```bash
make test
```

This compiles and executes `tests/test_basic`, which verifies:

* Insert
* Select
* Delete
  operations using C `assert()` statements.

---

## 📝 License

This project is open-source and distributed under the **MIT License**.
