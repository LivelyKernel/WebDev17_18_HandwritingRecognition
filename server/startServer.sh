until LD_LIBRARY_PATH=/home/nhoffmann/WebDev17_18_HandwritingRecognition/WritePadWrapper/build python3 main.py; do
    echo "Server 'writepad' crashed with exit code $?.  Respawning.." >&2
    sleep 1
done
