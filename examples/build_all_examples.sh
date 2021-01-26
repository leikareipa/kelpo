# Builds all Kelpo render examples that're residing immediately under this
# directory.

buildScriptName="build_win32_mingw441.sh"

echo "NOTE: BUILD WARNINGS ARE SUPPRESSED."

for exampleDirectory in $(find ./ -mindepth 1 -maxdepth 1 -type d); do
    if [ -f "$exampleDirectory/$buildScriptName" ]; then
        echo "Building $exampleDirectory..."
        (cd "$exampleDirectory" && ./"$buildScriptName" 2>&1 | grep -e "error" -e "undefined reference")
    fi
done

echo "Done. Any asset files will need to be copied manually."
