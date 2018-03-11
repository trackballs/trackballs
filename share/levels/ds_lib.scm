;;
;; Library of common routines and settings for the ds class levels
;;

(restart-time 0)

(define speed 
  (cond ((eq? (difficulty) *easy*) 0.1)
        ((eq? (difficulty) *normal*) 0.2)
        ((eq? (difficulty) *hard*) 0.3)))

(define make-pipe-link
  (lambda (x1 y1 x2 y2 z)
    (let* ((rx (- y1 y2))
           (ry (- x2 x1))
           (nrm (* 4 (sqrt (+ (* rx rx) (* ry ry)))))
           (dx (/ rx nrm))
           (dy (/ ry nrm)))
    (pipe (+ x1 dx) (+ y1 dy) (- z 0.1) (+ x2 dx) (+ y2 dy) (- z 0.1) 0.1)
    (pipe (- x1 dx) (- y1 dy) (- z 0.1) (- x2 dx) (- y2 dy) (- z 0.1) 0.1))))

(define marked (make-list (* 128 256) #f))

(define main-loop
  (lambda ()
    (let ((rx (get-position-x (player)))
          (ry (get-position-y (player))))
      (let* ((cx (inexact->exact (round rx)))
             (cy (inexact->exact (round ry)))
             (loc (+ (* 256 (- cx 128)) cy))
             (cur (if (and (>= loc 0) (< loc (* 128 256))) (list-ref marked loc) #f)))
        (cond ((or (< loc 0) (>= loc (* 128 256))
                   (not (boolean? cur))
                   (get-cell-flag cx cy *cell-kill*))
               #f)
              (cur #f)
              (else
                (list-set! marked loc (animator 1.0 0.1 speed 0.0 1.0 *animator-remove*
                    (lambda (j)
                      (cond ((> j 0.7)
                              (set-cell-colors cx cy cx cy *cell-center* 1 0 0 1)
                              (set-cell-flag cx cy cx cy *cell-kill* #t)
                              (let ((f (list-ref marked loc)))
                                (if (not (boolean? f))
                                    (begin (set-animator-position f 1.)
                                           (set-animator-direction f 1.))))
                              (list-set! marked loc #t))
                            ((< j 0.05) (list-set! marked loc #f))
                            (else (set-cell-colors cx cy cx cy
                              *cell-center* 1.0 (- 1 j) (- 1 j) 1.0))))))))))))

(animator 0.0 0.1 0.05 0.0 1.0 *animator-wrap*
  (lambda (h) (main-loop)))

;; req player existance in case of edit mode
(if (not (unspecified? (player)))
  (on-event *spawn* (player)
    (lambda (sub obj)
      (map
        (lambda (e)
          (if (boolean? e) #f
            (begin
              (set-animator-position e -1.)
              (set-animator-direction e -1.)
              #f)))
        marked)
      (set! marked (make-list (* 128 256) #f))
      (copy-cells 0 0 127 255 128 0 #f #f #f)
      (reset))))

(define reset (lambda x (error "Users of 'ds_lib' must override the function (reset)" #f)))

(copy-cells 128 0 255 255 0 0 #f #f #f)
