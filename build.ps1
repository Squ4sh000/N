gcc -o n.exe src/main.c src/lexer.c src/parser.c src/interpreter.c src/utils.c -Isrc
if ($?) {
    Write-Host "Build successful: n.exe" -ForegroundColor Green
} else {
    Write-Host "Build failed" -ForegroundColor Red
}
