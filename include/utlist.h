/*
Copyright (c) 2007-2012, Troy D. Hanson   http://uthash.sourceforge.net
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTLIST_H
#define UTLIST_H

/******************************************************************************
 * singly linked list macros (non-circular) (custom pointer name)             *
 *****************************************************************************/
#define LL_NAMED_PREPEND(head,add,prev,next) \
do { \
  (add)->next = head; \
  head = add; \
} while (0)

#define LL_NAMED_CONCAT(head1,head2,prev,next) \
do { \
  __typeof(head1) _tmp; \
  if (head1) { \
    _tmp = head1; \
    while (_tmp->next) { _tmp = _tmp->next; } \
    _tmp->next=(head2); \
  } else { \
    (head1)=(head2); \
  } \
} while (0)

#define LL_NAMED_APPEND(head,add,prev,next) \
do { \
  __typeof(head) _tmp; \
  (add)->next=0; \
  if (head) { \
    _tmp = head; \
    while (_tmp->next) { _tmp = _tmp->next; } \
    _tmp->next=(add); \
  } else { \
    (head)=(add); \
  } \
} while (0)

#define LL_NAMED_DELETE(head,del,prev,next) \
do { \
  __typeof(head) _tmp; \
  if ((head) == (del)) { \
    (head)=(head)->next; \
  } else { \
    _tmp = head; \
    while (_tmp->next && (_tmp->next != (del))) { \
      _tmp = _tmp->next; \
    } \
    if (_tmp->next) { \
      _tmp->next = ((del)->next); \
    } \
  } \
} while (0)

#define LL_NAMED_FOREACH(head,el,prev,next) \
    for(el=head;el;el=(el)->next)

#define LL_NAMED_FOREACH_SAFE(head,el,tmp,prev,next) \
  for((el)=(head);(el) && (tmp = (el)->next, 1); (el) = tmp)

#define LL_NAMED_SEARCH_SCALAR(head,out,field,val,prev,next) \
do { \
    LL_NAMED_FOREACH(head,out) { \
      if ((out)->field == (val)) break; \
    } \
} while(0)

#define LL_NAMED_SEARCH(head,out,elt,cmp,prev,next) \
do { \
    LL_NAMED_FOREACH(head,out) { \
      if ((cmp(out,elt))==0) break; \
    } \
} while(0)

/******************************************************************************
 * doubly linked list macros (non-circular) (custom pointer name)             *
 *****************************************************************************/
#define DL_NAMED_PREPEND(head,add,prev,next) \
do { \
 (add)->next = head; \
 if (head) { \
   (add)->prev = (head)->prev; \
   (head)->prev = (add); \
 } else { \
   (add)->prev = (add); \
 } \
 (head) = (add); \
} while (0)

#define DL_NAMED_APPEND(head,add,prev,next) \
do { \
  if (head) { \
      (add)->prev = (head)->prev; \
      (head)->prev->next = (add); \
      (head)->prev = (add); \
      (add)->next = 0; \
  } else { \
      (head)=(add); \
      (head)->prev = (head); \
      (head)->next = 0; \
  } \
} while (0)

#define DL_NAMED_CONCAT(head1,head2,prev,next) \
do { \
  __typeof(head1) _tmp; \
  if (head2) { \
    if (head1) { \
        _tmp = (head2)->prev; \
        (head2)->prev = (head1)->prev; \
        (head1)->prev->next = (head2); \
        (head1)->prev = _tmp; \
    } else { \
        (head1)=(head2); \
    } \
  } \
} while (0)

#define DL_NAMED_DELETE(head,del,prev,next) \
do { \
  assert((del)->prev != 0); \
  if ((del)->prev == (del)) { \
      (head)=0; \
  } else if ((del)==(head)) { \
      (del)->next->prev = (del)->prev; \
      (head) = (del)->next; \
  } else { \
      (del)->prev->next = (del)->next; \
      if ((del)->next) { \
          (del)->next->prev = (del)->prev; \
      } else { \
          (head)->prev = (del)->prev; \
      } \
  } \
} while (0)


#define DL_NAMED_FOREACH(head,el,prev,next) \
  for(el=head;el;el=(el)->next)

/* this version is safe for deleting the elements during iteration */
#define DL_NAMED_FOREACH_SAFE(head,el,tmp,prev,next) \
  for((el)=(head);(el) && (tmp = (el)->next, 1); (el) = tmp)

/* these are identical to their singly-linked list counterparts */
#define DL_SEARCH_SCALAR LL_SEARCH_SCALAR
#define DL_SEARCH LL_SEARCH

/******************************************************************************
 * circular doubly linked list macros (custom pointer name)                   *
 *****************************************************************************/
#define CDL_NAMED_PREPEND(head,add,prev,next) \
do { \
 if (head) { \
   (add)->prev = (head)->prev; \
   (add)->next = (head); \
   (head)->prev = (add); \
   (add)->prev->next = (add); \
 } else { \
   (add)->prev = (add); \
   (add)->next = (add); \
 } \
(head)=(add); \
} while (0)

#define CDL_NAMED_DELETE(head,del,prev,next) \
do { \
  if ( ((head)==(del)) && ((head)->next == (head))) { \
      (head) = 0L; \
  } else { \
     (del)->next->prev = (del)->prev; \
     (del)->prev->next = (del)->next; \
     if ((del) == (head)) (head)=(del)->next; \
  } \
} while (0)

#define CDL_NAMED_FOREACH(head,el,prev,next) \
    for(el=head;el;el=((el)->next==head ? 0L : (el)->next))

#define CDL_NAMED_FOREACH_SAFE(head,el,tmp1,tmp2,prev,next) \
  for((el)=(head), ((tmp1)=(head)?((head)->prev):0); \
      (el) && ((tmp2)=(el)->next, 1); \
      ((el) = (((el)==(tmp1)) ? 0L : (tmp2))))

#define CDL_NAMED_SEARCH_SCALAR(head,out,field,val,prev,next) \
do { \
    CDL_NAMED_FOREACH(head,out) { \
      if ((out)->field == (val)) break; \
    } \
} while(0)

#define CDL_NAMED_SEARCH(head,out,elt,cmp,prev,next) \
do { \
    CDL_FOREACH(head,out) { \
      if ((cmp(out,elt))==0) break; \
    } \
} while(0)

/******************************************************************************
 * singly linked list macros (non-circular)                                   *
 *****************************************************************************/
#define LL_PREPEND(head,add) \
    LL_NAMED_PREPEND(head,add,prev,next)

#define LL_CONCAT(head1,head2) \
    LL_NAMED_CONCAT(head1,head2,prev,next)

#define LL_APPEND(head,add) \
    LL_NAMED_APPEND(head,add,prev,next)

#define LL_DELETE(head,del) \
    LL_NAMED_DELETE(head,del,prev,next)

#define LL_FOREACH(head,el) \
    LL_NAMED_FOREACH(head,el,prev,next)

#define LL_FOREACH_SAFE(head,el,tmp) \
    LL_NAMED_FOREACH_SAFE(head,el,tmp,prev,next)

#define LL_SEARCH_SCALAR(head,out,field,val) \
    LL_NAMED_SEARCH_SCALAR(head,out,field,val,prev,next)

#define LL_SEARCH(head,out,elt,cmp) \
    LL_NAMED_SEARCH(head,out,elt,cmp,prev,next)

/******************************************************************************
 * doubly linked list macros (non-circular)                                   *
 *****************************************************************************/
#define DL_PREPEND(head,add) \
    DL_NAMED_PREPEND(head,add,prev,next)

#define DL_APPEND(head,add) \
    DL_NAMED_APPEND(head,add,prev,next)

#define DL_CONCAT(head1,head2) \
    DL_NAMED_CONCAT(head1,head2,prev,next)

#define DL_DELETE(head,del) \
    DL_NAMED_DELETE(head,del,prev,next)

#define DL_FOREACH(head,el) \
    DL_NAMED_FOREACH(head,el,prev,next)

/* this version is safe for deleting the elements during iteration */
#define DL_FOREACH_SAFE(head,el,tmp) \
    DL_NAMED_FOREACH_SAFE(head,el,tmp,prev,next)

/* these are identical to their singly-linked list counterparts */
#define DL_SEARCH_SCALAR LL_SEARCH_SCALAR
#define DL_SEARCH LL_SEARCH

/******************************************************************************
 * circular doubly linked list macros                                         *
 *****************************************************************************/
#define CDL_PREPEND(head,add) \
    CDL_NAMED_PREPEND(head,add,prev,next)

#define CDL_DELETE(head,del) \
    CDL_NAMED_DELETE(head,del,prev,next)

#define CDL_FOREACH(head,el) \
    CDL_NAMED_FOREACH(head,el,prev,next)

#define CDL_FOREACH_SAFE(head,el,tmp1,tmp2) \
    CDL_NAMED_FOREACH_SAFE(head,el,tmp1,tmp2,prev,next)

#define CDL_SEARCH_SCALAR(head,out,field,val) \
    CDL_NAMED_SEARCH_SCALAR(head,out,field,val,prev,next)

#define CDL_SEARCH(head,out,elt,cmp) \
    CDL_NAMED_SEARCH(head,out,elt,cmp,prev,next)

#endif /* UTLIST_H */
