# List of test programs
test_programs=($(find . -type f -name 'test_*.c' | sed 's|^\./||; s/\.c$//'))

# Flag to check if async mode is enabled
async_mode=false
if [ "$1" = "--async" ]; then
    async_mode=true
    echo "Running tests in async mode..."
fi

# Loop through each test program and run the corresponding test
for program in "${test_programs[@]}"; do
    echo "Started test for $program..."
    if [ $async_mode = true ]; then
        ./test.sh "$program" --keep > /dev/null 2>&1 &
    else
        ./test.sh "$program"
    fi
done

if [ $async_mode = true ]; then
    wait
    echo "Tests completed."
    for program in $test_programs; do
        echo "Output of $program:"
        awk '/Booting from Hard Disk..xv6.../ {flag=1} flag' $program.out
        if grep -q "Test Failed" $1.o; then
            printf "\n\033[1;31mTest Failed\n\n\033[0m"
        elif grep -q "Test Passed" $1.o; then
            printf "\n\033[1;32mTest Passed\n\n\033[0m"
        fi
        rm $1.o
        echo "----------------------------"
    done
fi
