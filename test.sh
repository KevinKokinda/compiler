#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

run_test() {
    local test_file="$1"
    local base_name=$(basename "$test_file" .sl)
    
    echo "Testing $base_name..."
    
    ./compiler "$test_file" "output/${base_name}.asm"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to compile ${base_name}${NC}"
        return 1
    fi
    
    nasm -f elf64 "output/${base_name}.asm" -o "output/${base_name}.o"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to assemble ${base_name}${NC}"
        return 1
    fi
    
    gcc "output/${base_name}.o" -o "output/${base_name}"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to link ${base_name}${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Successfully built ${base_name}${NC}"
    
    return 0
}

mkdir -p output

for test_file in tests/*.sl; do
    run_test "$test_file"
    echo
done