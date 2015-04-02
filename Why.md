# Why an alternate implementation? #

Given that the Message Pack project contains a perfectly functional C implementation already, what is the rationale behind reimplementation and why should you consider using this version?

Originally this project arose out of a desire to use Message Pack in a language without existing bindings. Using a DLL seemed an obvious solution to avoid starting from scratch. Unfortunately the partial function inlining of the original implementation meant that half the functions did not export. I eventually found a way to trick the compiler, but was then thwarted by my target language's lack of support for function pointers.

After exploring some options, I decided to reimplement in C with the following design goals in mind:
  * Pure C: fast and versatile
  * Compiles without warnings (-Wall)
  * No external dependencies
  * Ability to inline or export to shared library (see [CompilingToDLL](CompilingToDLL.md))
  * Implement only "sbuffer" back-end to simplify packing procedure
  * Simplify unpacking and remove need for temporary objects
  * Copy data directly into buffer - zero copy makes no sense on single objects
  * Simple error checking
Check out [ImplementationDetails](ImplementationDetails.md) for a more in-depth explanation of the differences.

This implementation has all these features, and is binary-compatible with other bindings. Have a look at the comparison code and [speed tests](SpeedTests.md), and decide for yourself!