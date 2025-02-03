if git status | grep -w "modified" > /dev/null; then
    if ls -la | grep -w ".vscode" > /dev/null; then
        rm -f .vscode
        echo .vscode found and removed successfully;
    fi
    git add .; git commit -m "Updated"; git push;
    echo \e[1;32m File successfully git push;
fi