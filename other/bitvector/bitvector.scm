#!r6rs

#|
  ByteVector bit operations library
  author: Vaccari Fabio, fabio.vaccari@gmail.com
  date: 20111117 created
  date: 20111118 library form, version 1
  date: 20111124 added a copy version of set and unset
|#

(library (vf utils bvbit (1))
         
         (export bytemask 
                 bvbit-set bvbit-unset
                 bvbit-set? bvbit-unset?
                 bvbit-set! bvbit-unset! 
                 bvbit-for-each bvbit-map)
         
         (import (rnrs base (6))
                 (rnrs bytevectors (6))
                 (rnrs arithmetic bitwise (6)))
         
         ;;; masks of a byte
         (define bytemask #vu8(1 2 4 8 16 32 64 128))
         
         ;;; set the bit at position 'k'
         ;;; return the result as a new instance
         (define (bvbit-set bytevector k)
           (let ((kmask (bytevector-copy bytevector))) 
             (bvbit-set! kmask k)
             kmask))
         
         ;;; unset the bit at position 'k'
         ;;; return the result as a new instance
         (define (bvbit-unset bytevector k)
           (let ((kmask (bytevector-copy bytevector))) 
             (bvbit-unset! kmask k)
             kmask))
         
         ;;; return #t if the vector at position 'k' is set
         ;;; otherwise return #f
         (define (bvbit-set? bytevector k)
           (let ((segment (div k 8))
                 (offset (mod k 8)))
             (> (bitwise-and (bytevector-u8-ref bytevector segment)
                             (bytevector-u8-ref bytemask offset))
                0)))
         
         ;;; return #t if the vector at position 'k' is not set
         ;;; otherwise return #f
         (define (bvbit-unset? bytevector k)
           (not (bvbit-set? bytevector k)))
         
         ;;; set the bit at position 'k'
         (define (bvbit-set! bytevector k)
           (let* ((segment (div k 8))
                  (offset (mod k 8))
                  (octet (bitwise-ior (bytevector-u8-ref bytevector segment)
                                      (bytevector-u8-ref bytemask offset))))
             (bytevector-u8-set! bytevector segment octet)))
         
         ;;; unset the bit at position 'k'
         (define (bvbit-unset! bytevector k)
           (let* ((segment (div k 8))
                  (offset (mod k 8))
                  (octet (bitwise-and (bytevector-u8-ref bytevector segment)
                                      (bitwise-not (bytevector-u8-ref bytemask offset)))))
             (bytevector-u8-set! bytevector segment octet)))
         
         ;;; local, return a list of elements fom a list of vectors
         ;;; each element is the one at position 'k'
         ;;; of a corresponding vector in 'lst'
         (define (param-list k lst)
           (let ((head (car lst))
                 (tail (cdr lst)))
             (if (null? tail) (cons (vector-ref head k) '())
                 (cons (vector-ref head k) (param-list k tail)))))
         
         ;;; apply 'proc' for each position in 'bytevector'
         ;;; procedure arguments are taken in this way:
         ;;; the first argumen from the first vector in the list
         ;;; the second argument from the second vector in the list
         ;;; ad so on
         (define (bvbit-for-each proc bytevector . vector-list)
           (define (bvbit-iter count len)
             (if (< count len) 
                 (begin
                   (if (bvbit-set? bytevector count)
                       (apply proc (param-list count vector-list)))
                   (bvbit-iter (+ 1 count) len))))
           (bvbit-iter 0 (vector-length (car vector-list))))
         
         ;;; apply 'proc' for each position in 'bytevector'
         ;;; procedure arguments are taken in this way:
         ;;; the first argumen from the first vector in the list
         ;;; the second argument from the second vector in the list
         ;;; ad so on
         ;;; return a list containing the result of each application
         (define (bvbit-map proc bytevector . vector-list)
           (define (bvbit-iter count len)
             (if (< count len)
                 (if (bvbit-set? bytevector count) 
                     (cons (apply proc (param-list count vector-list))
                           (bvbit-iter (+ 1 count) len))
                     (bvbit-iter (+ 1 count) len))
                 '()))
           (bvbit-iter 0 (vector-length (car vector-list)))))