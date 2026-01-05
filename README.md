# Parsing Arguments for C Programs
This header-only library is intended mostly for parsing arrays of arguments.

- [ ] Needs fuzz testing
- [ ] Test with C++

## Implicit Behavior and Assumptions
- Given argument array needs to be NULL terminated
- All registered variable pointers are initialized with zero, if not overwritten by an argument
- Some structures that are added with this header are
- All array elements need to be comma-seperated

> [!WARNING]
> Positional arguments are not supported.

## Structures
- `Argument` is the main structure to define expected arguments with
- `DummyArray`, `StringArray`, `IntArray`, `LongArray`, `FloatArray` and `DoubleArray` are helper structures to save parsed arrays

## Functions
- `printArgumentHelp` will print help for the `Argument`-array
- `processArguments` will parse the program's argv according to the given `Argument`-array
- `freeArguments` will free all allocated memory created by array allocation
- functions with the `pcinternal_`-prefix are helpers

