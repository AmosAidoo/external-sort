# External Sort

This is an implementation of external merge sort algorithm which is used to sort huge amounts of data that do not fit in main memory.

## Description

Sorting is so fundamental that your favorite programming language likely has in-built functions to sort a list of values.
Things get interesting when the data that we have to sort is so huge that its non-trivial to call your language's sort function
on the the data. An example will be trying to sort a 10GB file containing unsigned integers on a machine with 4GB of RAM. In this
case we will have to split this 10GB file into multiple chuncks and sort them. After that, then we combine these chucks into the final
sorted out put.
The technique of copying chucks of huge data from disk into memory, operating on them and writing the results back to disk are quite
common in computer science. For a field like database development, it is at the heart of it so knowledge about these topics is very
useful. In [TUM's](https://www.tum.de/) Database Systems on Modern CPU Architectures course, [the first programming exercise](https://db.in.tum.de/teaching/ss15/moderndbs/TUM/Assignment1.pdf?lang=en) is to implement a function that can sort a large (5GB) file so this project is my attempt to solve it as I study about database development.

## Getting Started

### Dependencies

* cmake version 3.15+
* gcc or clang

### Installing

* 

### Running tests

```
cd build
ctest --test-dir tests --verbose
```

### Executing program

* 
```
```

## Acknowledgments

* [Datenbanksysteme und moderne CPU-Architekturen, 2015](https://db.in.tum.de/teaching/ss15/moderndbs/?lang=en)
* [Datenbanksysteme und moderne CPU-Architekturen, 2021](https://db.in.tum.de/teaching/ss21/moderndbs/?lang=en)
* [Wikipedia: External sorting](https://en.wikipedia.org/wiki/External_sorting)
