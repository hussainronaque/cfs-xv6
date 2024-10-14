if [ -z "$1" ]; then
  echo "Usage: ./run_test.sh <test_program>"
  exit 1
fi

if [ "$2" = "--keep" ]; then
    echo "Persisting xv6 build and output file..."
fi

out_file="$1.out"

if [ "$(uname)" = "Darwin" ]; then
    prefix="TOOLPREFIX=i386-elf-"
else
    prefix=""
fi

echo -e "\n\n$1\n" | make $prefix qemu-nox-test > $out_file 2>&1 &

# Capture the process ID (PID) of the last background process
QEMU_PID=$!

timeout=120
interval=1
elapsed=0

while [ $elapsed -lt $timeout ]; do
    # Check if the output file contains "Test completed"
    if grep -iqE "Test completed|panic|ALL TESTS PASSED" "$out_file"; then
        break
    fi
    sleep $interval
    elapsed=$((elapsed + interval))
done

# Kill the QEMU process after 10 seconds
kill $QEMU_PID
[ "$2" = "--keep" ] || make $prefix clean;

awk '/cpu0: starting 0/ {flag=1} flag' $out_file

# Check the output for "Test Passed"
if grep -iq "Test Failed" $out_file; then
    printf "\n\033[1;31mTest Failed\n\n\033[0m"
    [ "$2" = "--keep" ] || rm $out_file
    exit 1
elif grep -iqE "Tests? Passed" $out_file; then
    printf "\n\033[1;32mTest Passed\n\n\033[0m"
    [ "$2" = "--keep" ] || rm $out_file
    exit 0
else
    printf "\n\033[1;33mAssertions not found\n\n\033[0m"
    [ "$2" = "--keep" ] || rm $out_file
    exit 1
fi
