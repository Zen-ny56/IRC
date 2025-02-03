if git status | grep -w "modified" > /dev/null; then
    if ls -la | grep -w ".vscode" > /dev/null; then
        find . -name "*.vscode" -delete
        echo .vscode found and removed successfully;
    fi
    git add .; git commit -m "Updated"; git push;
    sleep 3;
    clear;
    echo ****** File successfully git push ******;
fi