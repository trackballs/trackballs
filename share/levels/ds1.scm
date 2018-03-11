;;; Track: ds1

(set-track-name (_ "Principia"))
(set-author "0x7fa00000")
(start-time 180)
(load "ds_lib.scm")
(set-start-position 220 220)
(add-goal 220 223 #t "ds2")
(add-flag 248 250 50 #t 0.1)

(define ff (forcefield 219 222.5 0 0.0 1.0 0.0 1.0 *ff-bounce*))

(define roger #f)

(define witch (switch 223 212
  (lambda () (set-onoff ff #t))
  (lambda () (set-onoff ff #f))))

(set-primary-color ff 0. 0. 1. 1.0)
(set-primary-color witch 0. 0. 1. 1.0)

(smart-trigger 224.5 224.5 3.
  (lambda ()
    (set-horizon roger 10.))
  (lambda () #f))

(define reset
  (lambda ()
    (if (boolean? roger)
         (begin (set! roger (new-mr-black 224.6 224.6))
            (on-event *death* roger (lambda (sub obj) (set! roger #f))))
         (set-position roger 224.6 224.6 0.25))
    (cond ((eq? (difficulty) *easy*) (set-modtime roger *mod-small* -1.))
          ((eq? (difficulty) *normal*) #f)
          ((eq? (difficulty) *hard*) (set-modtime roger *mod-spike* -1.)))
    (set-acceleration roger 4.0)
    (set-horizon roger 2.0)
    (set-primary-color roger 0.5 0. 0. 1.0)
    (set-secondary-color roger 1. 0. 0. 1.0)

    (add-flag 219 226 50 #t 0.1)
    (add-flag 216 220 100 #t 0.1)
    (add-flag 220 209 200 #t 0.1)

    (set-onoff ff #t)
    (set-onoff witch #t)))
