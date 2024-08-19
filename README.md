# Malloc Implementation

## Description

This project involves a custom implementation of `malloc` and `free`, focusing on heap management. The implementation includes splitting and coalescing free blocks, and supports multiple heap management strategies: Next Fit, Worst Fit, Best Fit, and First Fit. The project was completed to understand and demonstrate core concepts of memory allocation and management in C.

## Building and Running the Code

The code compiles into four shared libraries and six test programs. To build the code, navigate to the top-level assignment directory and run:

```bash
make
```

To use the custom memory allocator and override the existing `malloc`, use `LD_PRELOAD`. For example, to run the `ffnf` test using the First Fit shared library:

```bash
env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf
```

To run the other heap management schemes, replace `libmalloc-ff.so` with the appropriate library:

- **Best-Fit**: `libmalloc-bf.so`
- **First-Fit**: `libmalloc-ff.so`
- **Next-Fit**: `libmalloc-nf.so`
- **Worst-Fit**: `libmalloc-wf.so`

## Program Requirements

The implementation includes the following features:

1. **Block Splitting and Coalescing**: Implements splitting of larger free blocks and coalescing of adjacent free blocks.

2. **Heap Management Strategies**:
   - Next Fit
   - Worst Fit
   - Best Fit (First Fit is already implemented)

3. **Counters**: Tracks various statistics, including the number of successful `malloc` and `free` calls, block reuse, new block requests, splits, coalesces, and other memory metrics. Example statistics output:

    ```
    mallocs: 8
    frees: 8
    reuses: 1
    grows: 5
    splits: 1
    coalesces: 1
    blocks: 5
    requested: 7298
    max heap: 4096
    ```

4. **Additional Implementations**: Includes `realloc` and `calloc`.

5. **Benchmarking**: The performance of the four allocator implementations was compared against the standard `malloc()`. Benchmarks evaluate:
   - Performance
   - Number of splits and heap growth
   - Heap fragmentation
   - Max heap size

6. **Report**: A comprehensive report was generated, including:
   - Executive summary
   - Description of algorithms
   - Test implementation
   - Test results for all five candidates (including standard `malloc`)
   - Explanation and interpretation of results
   - Conclusion

   The report is available as [./Report.pdf] file.

## Debugging

For debugging, particularly if you encounter segfaults or memory errors, use `gdb` with the custom malloc library. Example commands:

```bash
$ gdb ./tests/ffnf
...
(gdb) set exec-wrapper env LD_PRELOAD=./lib/libmalloc-ff.so
(gdb) run
...
(gdb) where
```

## Administrative Notes

- The assignment was completed in C and tested in the course GitHub Codespace.
- The project adheres to academic integrity guidelines. The implementation is 100% original work.

