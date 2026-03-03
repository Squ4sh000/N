const vscode = require('vscode');
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
    let outputTerminal = null;

    let disposable = vscode.commands.registerCommand('n-lang.runFile', async function (uri) {
        const editor = vscode.window.activeTextEditor;
        
        // Use uri if triggered from context menu, otherwise active editor
        let filePath = uri ? uri.fsPath : (editor ? editor.document.fileName : null);

        if (!filePath) {
            vscode.window.showErrorMessage('No N script file to run.');
            return;
        }

        if (path.extname(filePath) !== '.ns') {
            vscode.window.showErrorMessage('Not an N script file (.ns).');
            return;
        }

        // Auto-save the file before running
        if (editor && editor.document.fileName === filePath) {
            await editor.document.save();
        }

        // Configuration and path detection
        const config = vscode.workspace.getConfiguration('n-lang');
        let nPath = config.get('executablePath') || 'n';

        // Robust path detection for Windows
        if (nPath === 'n' && process.platform === 'win32') {
            const commonPaths = [
                path.join(process.env['ProgramFiles'] || 'C:\\Program Files', 'NLanguage', 'n.exe'),
                path.join(process.env['ProgramFiles(x86)'] || 'C:\\Program Files (x86)', 'NLanguage', 'n.exe'),
                path.join(process.env['LocalAppData'] || '', 'Programs', 'NLanguage', 'n.exe')
            ];

            for (const p of commonPaths) {
                if (fs.existsSync(p)) {
                    nPath = p;
                    break;
                }
            }
        }

        // Create or reuse terminal
        if (!outputTerminal || outputTerminal.exitStatus !== undefined) {
            outputTerminal = vscode.window.createTerminal('N Script');
        }

        outputTerminal.show();
        // Use & for PowerShell to run path with spaces
        outputTerminal.sendText(`& "${nPath}" "${filePath}"`);
    });

    context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};
