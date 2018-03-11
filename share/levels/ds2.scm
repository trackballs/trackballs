;;; Track ds2

(set-track-name (_ "Orbit"))
(set-author "0x7fa00000")
(start-time 200)
(set-start-position 223 223)
(load "ds_lib.scm")

(night)

(add-goal 221 195.3 #f "ds3")
(add-goal 220 195.3 #f "ds3")

(set-primary-color (add-flag 223 213 1000 #t 0.1) 0.0 0.0 0.0 1.0)

(define pitball #f)

(define reset
  (lambda ()
    (if (boolean? pitball)
        (begin
          (set! pitball (new-mr-black 223 213))
          (on-event *death* pitball (lambda (sub obj) (set! pitball #f)))))
    (set-position pitball 223 213 -3.0)
    (set-modtime pitball *mod-spike* -1.)
    (if (not (eq? (difficulty) *easy*))
        (set-modtime pitball *mod-large* -1.))
    (if (not (eq? (difficulty) *hard*))
        (begin 
          (set-acceleration pitball 5.0)
          (set-horizon pitball 5.0))
        (begin 
          (set-acceleration pitball 10.0)
          (set-horizon pitball 8.0)))
    (set-primary-color pitball 1.0 0. 0. 1.0)
    (set-secondary-color pitball 1. 1. 1. 1.0)
         
    (night)
    (set-cell-heights 220 192 221 192 5.5 5.5 5.5 5.5)
    (set-onoff green-gate (not (eq? (difficulty) *easy*)))
    (set-onoff blue-gate (eq? (difficulty) *hard*))
    (set-onoff green-switch (not (eq? (difficulty) *easy*)))
    (set-onoff blue-switch (eq? (difficulty) *hard*))))

(define green-gate (forcefield 222 204 -2 2.0 0.0 0.0 3.0 *ff-bounce*))
(define blue-gate (forcefield 222 203  -2 2.0 0.0 0.0 3.0 *ff-bounce*))

(define rev (switch 218 194
  (lambda () #f)
  (lambda () (set-cell-heights 220 192 221 192 4 4 4 4))))

(define green-switch (switch 229 213
  (lambda () (set-onoff green-gate #t))
  (lambda () (set-onoff green-gate #f))))

(define blue-switch (switch 217 213
  (lambda () (set-onoff blue-gate #t))
  (lambda () (set-onoff blue-gate #f))))

(set-primary-color green-switch 0 1 0. 1.0)
(set-primary-color green-gate 0.0 1 0. 1.0)
(set-primary-color blue-switch 0 0. 1. 1.0)
(set-primary-color blue-gate 0 0. 1. 1.0)

(add-flag 211 183 50 #t 0.2)
(add-flag 222 183 50 #t 0.2)
(add-flag 211 194 50 #t 0.2)
