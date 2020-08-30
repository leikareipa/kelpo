echo "NOTE: Build warnings are suppressed."

for buildScript in build_renderer_*.sh
do
    echo "Executing script $buildScript..."
    ./"$buildScript" 2>&1 | grep "error"

done

echo "Done."
