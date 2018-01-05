# A Library for Using and Manipulating Sets #

This is my implementation of a set, the most fundamental discrete structure.
It maintains the same properties of a mathematical set; it is unordered, and
does not contain the same instance of a datum more than once. You will see an
example of this behavior in the `set.c` test file. In the header file `set.h`,
I have also bothered to implement all of the basic set operations: union,
intersection, and subtraction. I did not implement the complement operation.
Union and Intersection are created here as variadic functions, and thus can
operate on more than two sets at a time, with a minimum of one. It may be
important to note that the null pointer is not accepted as the null set.
To represent the null set, create a new set struct with no members. A handful
of other set operations are included for convenience, such as `set_issubset`
and `set_isequal`, which return boolean values. My plan is to use this library
on a series of discrete mathematics programs, but we will see if that intention
ever comes to fruition.
