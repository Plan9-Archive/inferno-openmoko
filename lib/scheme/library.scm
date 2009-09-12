;;;
;;; Equivalence operations
;;;
(define equal?
  (lambda (x y)
    (define listeq
      (lambda (x y)
        (if (null? x)
            (if (null? y) #t #f)
            (if (equal? (car x) (car y)) (equal? (cdr x) (cdr y)) #f))))
    (if (not (pair? x))
        (if (not (pair? y)) (eqv? x y) #f)
        (if (not (pair? y))
            #f
            (listeq x y)))))

;;;
;;; Boolean operations
;;;
(define not (lambda (b) (if (eq? b #f) #t #f)))
(define boolean? (lambda (x) (if (eqv? x #f) #t (if (eqv? x #t) #t #f))))

;;;
;;; Numerical operations
;;;
(define zero? (lambda (n) (= n 0)))
(define positive? (lambda (n) (> n 0)))
(define negative? (lambda (n) (< n 0)))
(define odd? (lambda (n) (not (= (remainder n 2) 0))))
(define even? (lambda (n) (= (remainder n 2) 0)))
(define abs (lambda (n) (if (negative? n) (- n) n)))

(define binop-list
  (lambda (f l)
    (if (null? l)
        '()
        (if (null? (cdr l))
            (car l)
            (f (car l) (binop-list f (cdr l)))))))
(define max (lambda l (binop-list (lambda (x1 x2) (if (> x1 x2) x1 x2)) l)))
(define min (lambda l (binop-list (lambda (x1 x2) (if (< x1 x2) x1 x2)) l)))
(define gcd
  (lambda l
    (define gcd2
      (lambda (n m)
         (define r (abs (modulo n m)))
         (if (= r 0) m (gcd2 m r))))
    (if (null? l)
        0
        (if (null? (cdr l))
            (car l)
            (gcd2 (car l) (apply gcd (cdr l)))))))
(define lcm
  (lambda l
    (define lcm2 (lambda (n m) (abs (quotient (* n m) (gcd n m)))))
    (if (null? l)
        1
        (if (null? (cdr l))
            (car l)
            (lcm2 (car l) (apply lcm (cdr l)))))))

;;;; "ratize.scm" Convert number to rational number
;;; Adapted from SLIB, maintained by Aubrey Jaffer
(define rationalize
  (lambda (x e)
    (define sr
      (lambda (x y)
        (define fx (floor x))
        (define fy (floor y))
        (if (>= fx x)
            fx
            (if (= fx fy)
                (+ fx (/ (sr (/ (- y fy)) (/ (- x fx)))))
                (+ 1 fx)))))
    (define simplest
      (lambda (a b)
        (if (< b a)
            (simplest b a)
            (if (>= a b)
                a
                (if (positive? a)
                    (sr a b)
                    (if (negative? b) (- (sr (- b) (- a))) 0))))))
    (simplest (- x e) (+ x e))))
    
;(define (rational:simplest x y)
;  (define (sr x y) (let ((fx (floor x)) (fy (floor y)))
;                    (cond ((not (< fx x)) fx)
;                          ((= fx fy) (+ fx (/ (sr (/ (- y fy)) (/ (- x fx))))))
;                          (else (+ 1 fx)))))
;  (cond ((< y x) (rational:simplest y x))
;        ((not (< x y)) (if (rational? x) x (slib:error)))
;        ((positive? x) (sr x y))
;        ((negative? y) (- (sr (- y) (- x))))
;        (else (if (and (exact? x) (exact? y)) 0 0.0))))
;(define (rationalize x e) (rational:simplest (- x e) (+ x e)))

;;;
;;; List operations
;;;
(define caar (lambda (l) (car (car l))))
(define cadr (lambda (l) (car (cdr l))))
(define cdar (lambda (l) (cdr (car l))))
(define cddr (lambda (l) (cdr (cdr l))))
(define caaar (lambda (l) (car (car (car l)))))
(define caadr (lambda (l) (car (car (cdr l)))))
(define cadar (lambda (l) (car (cdr (car l)))))
(define caddr (lambda (l) (car (cdr (cdr l)))))
(define cdaar (lambda (l) (cdr (car (car l)))))
(define cdadr (lambda (l) (cdr (car (cdr l)))))
(define cddar (lambda (l) (cdr (cdr (car l)))))
(define cdddr (lambda (l) (cdr (cdr (cdr l)))))
(define caaaar (lambda (l) (car (car (car (car l))))))
(define caaadr (lambda (l) (car (car (car (cdr l))))))
(define caadar (lambda (l) (car (car (cdr (car l))))))
(define caaddr (lambda (l) (car (car (cdr (cdr l))))))
(define cadaar (lambda (l) (car (cdr (car (car l))))))
(define cadadr (lambda (l) (car (cdr (car (cdr l))))))
(define caddar (lambda (l) (car (cdr (cdr (car l))))))
(define cadddr (lambda (l) (car (cdr (cdr (cdr l))))))
(define cdaaar (lambda (l) (cdr (car (car (car l))))))
(define cdaadr (lambda (l) (cdr (car (car (cdr l))))))
(define cdadar (lambda (l) (cdr (car (cdr (car l))))))
(define cdaddr (lambda (l) (cdr (car (cdr (cdr l))))))
(define cddaar (lambda (l) (cdr (cdr (car (car l))))))
(define cddadr (lambda (l) (cdr (cdr (car (cdr l))))))
(define cdddar (lambda (l) (cdr (cdr (cdr (car l))))))
(define cddddr (lambda (l) (cdr (cdr (cdr (cdr l))))))

(define null? (lambda (l) (eq? () l)))
(define list?
  (lambda (l)
    (define list?2
      (lambda (l s)
        (if (eq? l s)
            #f
            (if (null? l)
                #t
                (if (pair? l)
                    (list?2 (cdr l) s)
                    #f)))))
    (if (null? l) #t (list?2 (cdr l) l))))
(define length
  (lambda (l)
    (if (null? l)
        0
        (+ 1 (length (cdr l))))))
(define append
  (lambda l
    (binop-list
      (lambda (l1 l2)
        (if (null? l1)
            l2
            (cons (car l1) (append (cdr l1) l2))))
      l)))
(define reverse
  (lambda (l)
    (if (null? l)
        ()
        (append (reverse (cdr l)) (cons (car l) '())))))
(define list-tail
  (lambda (x k)
    (if (zero? k)
        x
        (list-tail (cdr x) (- k 1)))))
(define list-ref
  (lambda (x k)
    (if (= k 0)
        (car x)
        (list-ref (cdr x) (- k 1)))))
(define memq
  (lambda (x l)
    (if (null? l) #f
        (if (eq? x (car l)) l
            (memq x (cdr l))))))
(define memv
  (lambda (x l)
    (if (null? l) #f
        (if (eqv? x (car l)) l
            (memv x (cdr l))))))
(define member
  (lambda (x l)
    (if (null? l) #f
        (if (equal? x (car l)) l
            (member x (cdr l))))))
(define assq
  (lambda (x l)
    (if (null? l) #f
        (if (eq? x (caar l))
            (car l)
            (assq x (cdr l))))))
(define assv
  (lambda (x l)
    (if (null? l) #f
        (if (eqv? x (caar l))
            (car l)
            (assq x (cdr l))))))
(define assoc
  (lambda (x l)
    (if (null? l) #f
        (if (equal? x (caar l))
            (car l)
            (assq x (cdr l))))))

;;;
;;; Character operations
;;;
(define char-lower-case?
  (lambda (c)
     (if (char>=? c #\a)
         (if (char<=? c #\z) #t #f)
         #f)))
(define char-upper-case?
  (lambda (c)
    (if (char>=? c #\A)
        (if (char<=? c #\Z) #t #f)
        #f)))
(define char-alphabetic?
  (lambda (c)
    (if (char-lower-case? c) #t
        (if (char-upper-case? c) #t #f)
        #f)))
(define char-numeric?
  (lambda (c)
    (if (char>=? c #\0)
        (if (char<=? c #\9) #t #f)
        #f)))
(define char-whitespace?
  (lambda (c)
    (if (char=? c #\space) #t
        (if (char=? c #\tab) #t
            (if (char=? c \#newline) #t
                (if (char=? c #\formfeed) #t
                    (if (char=? c #\return) #t
                        #f) #f) #f) #f) #f)))
(define char-upcase
  (lambda (c)
    (define diff (- (char->integer #\A) (char->integer #\a)))
    (if (char-lower-case? c)
        (integer->char (+ (char->integer c) diff))
        c)))
(define char-downcase
  (lambda (c)
    (define diff (- (char->integer #\A) (char->integer #\a)))
    (if (char-upper-case? c)
        (integer->char (- (char->integer c) diff))
        c)))
(define charop-ci (lambda (f c1 c2) (f (char-upcase c1) (char-upcase c2))))
(define char-ci=? (lambda (c1 c2) (charop-ci char=? c1 c2)))
(define char-ci<? (lambda (c1 c2) (charop-ci char<? c1 c2)))
(define char-ci>? (lambda (c1 c2) (charop-ci char>? c1 c2)))
(define char-ci<=? (lambda (c1 c2) (charop-ci char<=? c1 c2)))
(define char-ci>=? (lambda (c1 c2) (charop-ci char>=? c1 c2)))

;;;
;;; String Operations
;;;
(define string (lambda l (list->string l)))
(define list->string
  (lambda (l)
    (define load-string
      (lambda (s k l)
        (if (not (null? l)) (string-set! s k (car l)))
        (if (not (null? l)) (load-string s (+ k 1) (cdr l)))
        s))
    (load-string (make-string (length l)) 0 l)))
(define string->list
  (lambda (s)
    (define unpack-string
      (lambda (k n s)
        (if (= k n) '() (cons (string-ref s k) (unpack-string (+ k 1) n s)))))
    (unpack-string 0 (string-length s) s)))

;;;
;;; Vector Operations
;;;
(define vector->list
  (lambda (v)
    (define vliter
      (lambda (v k n)
        (if (= k n)
            '()
            (cons (vector-ref v k) (vliter v (+ k 1) n)))))
    (vliter v 0 (vector-length v))))
(define list->vector
  (lambda (l)
    (define dolv
      (lambda (v k l)
        (vector-set! v k (car l))
        (lviter v (+ k 1) (cdr l))))
    (define lviter
      (lambda (v k l)
        (if (null? l)
            v
            (dolv v k l))))
    (lviter (make-vector (length l)) 0 l)))
(define vector (lambda l (list->vector l)))
(define vector-fill!
  (lambda (v f)
    (define n (vector-length v))
    (define vfiter
      (lambda (v f k)
        (if (= k 0)
            (vector-set! v 0 f)
            ((lambda (v f k)
              (vector-set! v k f)
              (vfiter v f (- k 1))) v f k))))
    (vfiter v f (- n 1))))

;;;
;;; Control features
;;;
(define map
  (lambda (p . l)
    (define cars
      (lambda (l)
        (if (null? l) '() (cons (caar l) (cars (cdr l))))))
    (define cdrs
      (lambda (l)
        (if (null? l) '() (cons (cdar l) (cdrs (cdr l))))))
    (if (null? (car l))
        '()
        (cons (apply p (cars l)) (apply map p (cdrs l))))))

;;; map already does things in the order required by for-each
(define for-each
  (lambda (p . l)
    (apply map p l)))

;;;
;;; I/O Operations
;;;
(define call-with-input-file
  (lambda (s p)
    (define port (open-input-file s))
    (p port)
    (close-input-port port)))
(define call-with-output-file
  (lambda (s p)
    (define port (open-output-file s))
    (p port)
    (close-output-port port)))
(define newline
  (lambda p
    (if (null? p)
        (write-char #\newline)
        (write-char #\newline (car p)))))
