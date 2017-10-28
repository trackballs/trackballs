;;; boot.scm

(define last-level #f)
(define return-level #f)

;;**************************************
;; Utility functions
;; You can use these to simplify doing
;; more complicated map things
;;**************************************

(define (nth n l)
  (if (= 0 n) (car l) (nth (- n 1) (cdr l))))

;; coords is a list of coordinates. eg ((247.5 246.5 1.5) (250.5 246.5 1.5) (251.5 247.5 1.5))
;; radius is the radius which will be used for all pipes
;; connectors is a boolean if we should use connectors
;; result is a list of pipes and connectors
(define (multi-pipe coords radius connectors)
  (let ((p0 (nth 0 coords))
    (p1 (nth 1 coords)))
  (if (= (length coords) 2)
    (list (pipe (nth 0 p0) (nth 1 p0) (nth 2 p0) (nth 0 p1) (nth 1 p1) (nth 2 p1) radius))
    (cons (pipe (nth 0 p0) (nth 1 p0) (nth 2 p0) (nth 0 p1) (nth 1 p1) (nth 2 p1) radius)
        (if connectors 
          (cons (pipe-connector (nth 0 p1) (nth 1 p1) (nth 2 p1) radius)
            (multi-pipe (cdr coords) radius connectors))
          (multi-pipe (cdr coords) radius connectors))))))
          
;; arguments are like (trigger . . . .)
;; namely, position x y, radius of effect, and function to call
;; the thunk is executed only the first time the player enters the
;; radius and never again.
;; useful to avoid overcreating a fixed object (pipe, teleporter)
(define trigger-once
  (lambda (x y r thunk)
    (let ((first-time #t))
      (trigger x y r
        (lambda ()
          (if first-time
            (begin (set! first-time #f)
              (thunk))))))))
