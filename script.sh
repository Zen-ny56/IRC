if git status | grep -w "modified" > /dev/null; then
    if ls -la | grep -w ".vscode" > /dev/null; then
        rm -f .vscode
        echo .vscode found and removed successfully;
    fi
    git add .; git commit -m "Updated"; git push;
    echo \033[32m File successfully git push \033[0m;
fi