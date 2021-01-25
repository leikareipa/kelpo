echo "NOTE: BUILD WARNINGS ARE SUPPRESSED."

for buildScript in build_renderer_*.sh
do
    echo "Executing $buildScript..."
    ./"$buildScript" 2>&1 | grep "error"

done

echo "Done."
