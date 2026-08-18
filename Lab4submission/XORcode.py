#!/usr/bin/env python3
"""
xor_streams_full_debug.py

Comprehensive debug + Sequential XOR of 32 bitstreams (20,000 bits each).

Steps:
1. Load & normalize streams
2. Verify count and uniform length
3. Step-by-step XOR with per-step counts
4. After final XOR, check if Stream #32 matches the pre-XOR running result
5. Final summary and save
"""
import sys
from collections import Counter

INPUT_FILE   = 'output.txt'
OUTPUT_FILE  = 'xor_result.txt'
EXPECTED_NUM = 32


def read_streams(path):
    with open(path, 'r') as f:
        lines = [
            line.strip().replace('\r','').replace('\t','')
            for line in f
            if line.strip()
        ]
    if len(lines) != EXPECTED_NUM:
        sys.exit(f"ERROR: expected {EXPECTED_NUM} streams, but found {len(lines)}")
    print(f"Loaded {len(lines)} streams from '{path}'")
    return lines


def verify_lengths(streams):
    length = len(streams[0])
    if any(len(s) != length for s in streams):
        sys.exit("ERROR: Not all streams have equal length.")
    print(f"Verified all streams are {length} bits long")
    return length


def xor_two(a, b):
    """Bitwise XOR of two equal-length bitstrings."""
    return ''.join('1' if a[i] != b[i] else '0' for i in range(len(a)))


def main():
    # Read and verify
    streams = read_streams(INPUT_FILE)
    length = verify_lengths(streams)

    # Initial XOR of streams 1 & 2
    print("\n--- Step 1: XOR Stream #1 & #2 ---")
    result = xor_two(streams[0], streams[1])
    ones = result.count('1'); zeros = result.count('0')
    print(f"After XOR #1⊕#2: ones={ones}, zeros={zeros}")

    # Sequential XOR for streams 3..32
    for idx in range(2, EXPECTED_NUM):
        step = idx
        print(f"\n--- Step {step}: XOR running result with Stream #{idx+1} ---")
        prev_ones, prev_zeros = ones, zeros
        # Before final stream, capture result to compare
        if idx == EXPECTED_NUM - 1:
            prev_result = result
        # XOR
        result = xor_two(result, streams[idx])
        ones = result.count('1'); zeros = result.count('0')
        print(f"Before step {step}: ones={prev_ones}, zeros={prev_zeros}")
        print(f"After  step {step}: ones={ones}, zeros={zeros}")

    # Check if Stream #32 matches pre-XOR running result
    print("\n--- Verification: Stream #32 vs prior result ---")
    if streams[-1] == prev_result:
        print("Stream #32 exactly matches the XOR of streams 1–31.")
    else:
        print("Stream #32 differs from the XOR of streams 1–31.")

    # Final summary
    print("\n=== FINAL RESULT ===")
    print(f"Bit-length    : {length}")
    print(f"Number of 1's : {ones}")
    print(f"Number of 0's : {zeros}")

    # Write final result
    with open(OUTPUT_FILE, 'w') as f:
        f.write(result)
    print(f"\nFinal XOR bitstream written to: {OUTPUT_FILE}")

if __name__ == '__main__':
    main()
