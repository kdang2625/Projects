# XOR Bitstream Analysis

## Overview
A Python utility that sequentially XORs 32 binary streams, validates the input data, reports the number of ones and zeros after each stage, performs a final consistency check, and writes the resulting bitstream to an output file.

The directory also contains statistical-test output used to evaluate the bitstream data.

## Technologies
- Python 3
- Binary / XOR logic
- File I/O
- Input validation
- Statistical-result analysis

## Project Files
```text
Lab4submission/
├── XORcode.py
├── output.txt
├── xor_result.txt
└── finalAnalysisReport.txt
```

### `XORcode.py`
The script:
1. Loads binary streams from `output.txt`
2. Verifies that exactly 32 streams were provided
3. Verifies equal stream lengths
4. Sequentially XORs the streams
5. Reports `1` and `0` counts after each stage
6. Compares Stream 32 with the XOR result of Streams 1–31
7. Writes the final result to `xor_result.txt`

### `finalAnalysisReport.txt`
Contains statistical-test output including frequency, block-frequency, cumulative-sums, runs, longest-run, rank, FFT, and template-based results.

## Running the Project
No third-party packages are required.

```bash
python3 XORcode.py
```

The script expects `output.txt` in the same directory.

## Expected Input
- 32 non-empty binary streams
- Equal length for all streams
- Project data structured around 20,000-bit streams

## Output
The final XOR stream is saved to:
```text
xor_result.txt
```

## Engineering Concepts Demonstrated
- Python scripting
- Data validation
- Sequential data processing
- Binary/XOR operations
- File parsing
- Debug-oriented output
- Automated verification

## What I Learned
This project emphasized validation and step-by-step verification so errors in the input or processing can be identified instead of treating the XOR operation as a black box.
