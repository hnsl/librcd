/* CC-BY-SA 3.0 from https://stackoverflow.com/a/809465/68204 */
/* See the COPYING file distributed with this project for more information. */

/** A compile time assertion check.
 *
 *  Validate at compile time that the predicate is true without
 *  generating code. This can be used at any point in a source file
 *  where typedef is legal.
 *
 *  On success, compilation proceeds normally.
 *
 *  On failure, attempts to typedef an array type of negative size. The
 *  offending line will look like
 *      typedef assertion_failed_file_h_42[-1]
 *  where file is the content of the second parameter which should
 *  typically be related in some obvious way to the containing file
 *  name, 42 is the line number in the file on which the assertion
 *  appears, and -1 is the result of a calculation based on the
 *  predicate failing.
 *
 *  \param predicate The predicate to test. It must evaluate to
 *  something that can be coerced to a normal C boolean.
 *
 *  \param file A sequence of legal identifier characters that should
 *  uniquely identify the source file in which this condition appears.
 */

#ifndef CASSERT_H
#define CASSERT_H

#define CASSERT(predicate) \
    _CASSERT_INNER(predicate, __LINE__)

#define _CASSERT_PASTE(a, b) a##b

#define _CASSERT_INNER(predicate, line) \
    struct _CASSERT_PASTE(__cassert_, line){char _[2*!!(predicate)-1];}

#endif /* CASSERT_H */
