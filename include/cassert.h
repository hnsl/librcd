/* CC-BY-SA 3.0, modified from https://stackoverflow.com/a/809465/68204 */
/* See the COPYING file distributed with this project for more information. */

#ifndef CASSERT_H
#define CASSERT_H

/// A compile time assertion check.
///
/// Validate at compile time that the predicate is true without
/// generating code. This can be used at any point in a source file
/// where typedef is legal.
///
/// On success, compilation proceeds normally.
///
/// On failure, attempts to typedef an array type of negative size. The
/// offending line will look like:
///
/// typedef _cassert_1_42{char _[2*!!(0)-1];};
///
/// where 1 is a unique counter, and 42 is the line number in the file on
/// which the assertion appears.
#define CASSERT(predicate) \
    _CASSERT_INNER(predicate, __COUNTER__, __LINE__)

#define _CASSERT_PASTE(a, b, c, d) a##b##c##d

#define _CASSERT_INNER(predicate, counter, line) \
    struct _CASSERT_PASTE(__cassert_, counter, _, line){char _[2*!!(predicate)-1];}

#endif /* CASSERT_H */
