/*************************** asmlib.h ***************************************
* Author:        Agner Fog
* Date created:  2003-12-12
* Last modified: 2012-03-10
* Project:       asmlib.zip
* Source URL:    www.agner.org/optimize
*
* Description:
* Header file for the asmlib function library.
* This library is available in many versions for different platforms.
* See asmlib-instructions.pdf for details.
*
* (c) Copyright 2003 - 2012 by Agner Fog.
* GNU General Public License http://www.gnu.org/licenses/gpl.html
*****************************************************************************/


#ifndef ASMLIB_H
#define ASMLIB_H

#include "sse.h"

/***********************************************************************
Function prototypes, memory and string functions
***********************************************************************/
void * A_memcpy (void * dest, const void * src, size_t count); // Copy count bytes from src to dest
void * A_memmove(void * dest, const void * src, size_t count); // Same as memcpy, allows overlap between src and dest
void * A_memset (void * dest, int c, size_t count);            // Set count bytes in dest to (char)c
size_t GetMemcpyCacheLimit(void);                              // Data blocks bigger than this will be copied uncached by memcpy and memmove
void   SetMemcpyCacheLimit(size_t);                            // Change limit in GetMemcpyCacheLimit
size_t GetMemsetCacheLimit(void);                              // Data blocks bigger than this will be stored uncached by memset
void   SetMemsetCacheLimit(size_t);                            // Change limit in GetMemsetCacheLimit
char * A_strcat (char * dest, const char * src);               // Concatenate strings dest and src. Store result in dest
char * A_strcpy (char * dest, const char * src);               // Copy string src to dest
size_t A_strlen (const char * str);                            // Get length of zero-terminated string
int    A_strcmp (const char * a, const char * b);              // Compare strings. Case sensitive
int    A_stricmp (const char *string1, const char *string2);   // Compare strings. Case insensitive for A-Z only
char * A_strstr (char * haystack, const char * needle);        // Search for substring in string
void   A_strtolower(char * string);                            // Convert string to lower case for A-Z only
void   A_strtoupper(char * string);                            // Convert string to upper case for a-z only
size_t A_substring(char * dest, const char * source, size_t pos, size_t len); // Copy a substring for source into dest
size_t A_strspn (const char * str, const char * set);          // Find span of characters that belong to set
size_t A_strcspn(const char * str, const char * set);          // Find span of characters that don't belong to set
size_t strCountInSet(const char * str, const char * set);      // Count characters that belong to set
size_t strcount_UTF8(const char * str);                        // Counts the number of characters in a UTF-8 encoded string


/***********************************************************************
Function prototypes, miscellaneous functions
***********************************************************************/
uint32_t A_popcount(uint32_t x);                               // Count 1-bits in 32-bit integer
int    RoundD (double x);                                      // Round to nearest or even
int    RoundF (float  x);                                      // Round to nearest or even
int    InstructionSet(void);                                   // Tell which instruction set is supported
char * ProcessorName(void);                                    // ASCIIZ text describing microprocessor
void   CpuType(int * vendor, int * family, int * model);       // Get CPU vendor, family and model
size_t DataCacheSize(int level);                               // Get size of data cache
void   A_DebugBreak(void);                                     // Makes a debug breakpoint
uint64_t ReadTSC(void);                                     // Read microprocessor internal clock (64 bits)
void cpuid_ex (int abcd[4], int eax, int ecx);                 // call CPUID instruction
static inline void cpuid_abcd (int abcd[4], int eax) {
   cpuid_ex(abcd, eax, 0);}

/***********************************************************************
Function prototypes, integer division functions
***********************************************************************/

void setdivisori32(int buffer[2], int d);                      // Set divisor for repeated division
int dividefixedi32(const int buffer[2], int x);                // Fast division with previously set divisor
void setdivisoru32(uint32_t buffer[2], uint32_t d);            // Set divisor for repeated division
uint32_t dividefixedu32(const uint32_t buffer[2], uint32_t x); // Fast division with previously set divisor

// Integer vector division functions. These functions divide an integer vector by a scalar:

// Set divisor for repeated integer vector division
void setdivisorV8i16(__m128i buf[2], int16_t d);               // Set divisor for repeated division
void setdivisorV8u16(__m128i buf[2], uint16_t d);              // Set divisor for repeated division
void setdivisorV4i32(__m128i buf[2], int32_t d);               // Set divisor for repeated division
void setdivisorV4u32(__m128i buf[2], uint32_t d);              // Set divisor for repeated division

// Fast division of vector by previously set divisor
__m128i dividefixedV8i16(const __m128i buf[2], __m128i x);     // Fast division with previously set divisor
__m128i dividefixedV8u16(const __m128i buf[2], __m128i x);     // Fast division with previously set divisor
__m128i dividefixedV4i32(const __m128i buf[2], __m128i x);     // Fast division with previously set divisor
__m128i dividefixedV4u32(const __m128i buf[2], __m128i x);     // Fast division with previously set divisor

#endif // ASMLIB_H
