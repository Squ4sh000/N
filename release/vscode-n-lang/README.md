# N Language Support

[![Version](https://img.shields.io/visual-studio-marketplace/v/Squ4sh000.n-lang)](https://marketplace.visualstudio.com/items?itemName=Squ4sh000.n-lang)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Official VS Code extension for the **N Language**, a modern, lightweight, and expressive scripting language.

## Features

- **Syntax Highlighting**: Rich coloring for keywords, strings, numbers, and comments.
- **Run with Ease**: Right-click any `.ns` file and select **"Run N Script"** to execute it instantly in the integrated terminal.
- **Smart Detection**: Automatically finds the `n.exe` interpreter in your `PATH` or standard installation directory.
- **Auto-Save**: Automatically saves your code before running, ensuring you're always testing the latest version.
- **Terminal Management**: Reuses a single "N Script" terminal to keep your workspace clean.

## Requirements

You must have the **N Language Interpreter (`n.exe`)** installed on your system.
1. Download the N Language installer from the official repository.
2. Run the installer to set up the environment variables automatically.

## Extension Settings

This extension contributes the following settings:

* `n-lang.executablePath`: Custom path to the `n.exe` interpreter if it's not in your system PATH.

## Release Notes

### 1.0.0
- Initial release of N Language support.
- Support for `?` error operator, native arrays, and `for-in` loops.
- Integrated run command with right-click menu.

---
**Developed by Squ4sh000**
