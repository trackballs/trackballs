;;; Track ds3

(set-track-name (_ "Tunnel"))
(set-author "0x7fa00000")
(start-time 180)
(load "ds_lib.scm")

(fog)
(fog-color 0. 0. 0.)

(set-start-position 239 250.3)

(add-goal 250 169.3 #f "")
(add-flag 248 250 50 #t 0.1)

(define respawn-list '())

(define add-respawning
  (lambda (x y func)
    (let ((elem #f))
      (set! respawn-list (cons
        (lambda () (if (eq? elem #f)
                      (let ((next (func x y)))
                        (on-event *death* next (lambda (sub obj) (set! elem #f)))
                        (set! elem next))
                      (set-position elem x y)))
        respawn-list)))))

(define make-mini (lambda (x y)
  (set-modtime (set-modtime (new-mr-black x y) *mod-spike* -1.) *mod-small* -1)))
(define make-cactus (lambda (x y) (add-cactus x y 0.5)))

(add-respawning 238.5 240 make-mini)
(add-respawning 237.5 240 make-mini)
(add-respawning 239.5 240 make-mini)
(add-respawning 237.5 236 make-mini)

(jump 0.3)


(add-respawning 240 236 make-cactus)

;; passageway
(pipe 241 213 -4.2 247 213 -4.2 0.6)
(pipe 241 217 -4.2 247 217 -4.2 0.6)

(add-modpill 239 213 *mod-small* 10 2)
(add-modpill 239 208 *mod-small* 5 2)

(cond ((= (difficulty) *easy*)
        (add-respawning 239.5 207
          (lambda (x y)
            (set-modtime (new-mr-black x y) *mod-spike* -1.))))
      ((= (difficulty) *normal*)
        (add-respawning 239.5 207
          (lambda (x y)
            (set-modtime (set-modtime (new-mr-black x y) *mod-spike* -1.) *mod-large* -1))))
      ((= (difficulty) *hard*)
        (add-respawning 238.5 207
          (lambda (x y)
            (set-acceleration (set-primary-color (set-modtime (new-mr-black x y) *mod-spike* -1.) 1. 0. 1. 1.0) 10)))))

;; Height gates
(pipe 242 209 -4.0 236 209 -4.0 0.1)
(pipe 242 209.2 -4.0 236 209.2 -4.0 0.1)
(pipe 242 204 -4.0 236 204 -4.0 0.1)
(pipe 242 204.2 -4.0 236 204.2 -4.0 0.1)

(add-bird 241 171 241 187 0.4 1.5)
(add-bird 239 171 239 187 0.4 1.5)
(add-bird 237 171 237 187 0.4 1.5)

(define speed
  (cond ((= (difficulty) *easy*) 0.3)
        ((= (difficulty) *normal*) 0.6)
        ((= (difficulty) *hard*) 1.0)))

;; x y speed phase
(add-sidespike 241 200 (* 0.3 speed) 0.5 2)
(add-sidespike 237 200 (* 0.3 speed) 0.5 1)
(add-spike 241 195 (* 0.3 speed) 0.1)
(add-spike 240 195 (* 0.3 speed) 0.2)
(add-spike 239 195 (* 0.3 speed) 0.3)
(add-spike 238 195 (* 0.3 speed) 0.4)
(add-spike 237 195 (* 0.3 speed) 0.5)

(if (= (difficulty) *hard*)
  (begin
    (add-spike 239.5 187 (* 2.0 speed) 0.5)
    (add-spike 239 187 (* 2.0 speed) 0.5)
    (add-spike 238.5 187 (* 2.0 speed) 0.5)))

(if (not (= (difficulty) *easy*))
  (begin
    (add-sidespike 241 186 (* 0.6 speed) 0.0 2)
    (add-sidespike 237 186 (* 0.6 speed) 0.0 1)
    (add-sidespike 241 185 (* 0.6 speed) 0.5 2)
    (add-sidespike 237 185 (* 0.6 speed) 0.5 1)))

(add-spike 240 174 (* 1.0 speed) 0.5)
(add-spike 238 174 (* 1.0 speed) 0.5)

(add-flag 251 167 50 #t 0.1)
(add-flag 250 167 50 #t 0.1)
(add-flag 249 167 50 #t 0.1)
(add-flag 251 211 50 #t 0.1)

(add-flag 238 237 50 #t 0.1)
(set-primary-color (add-flag 237 216 50 #t 0.1) 1.0 1.0 0.0 1.0)

(add-flag 239 174 50 #t 0.1)
(add-flag 241 174 50 #t 0.1)
(add-flag 237 174 50 #t 0.1)

(add-flag 239 189 50 #t 0.1)

(define reset
  (lambda ()
    (map (lambda (f) (f)) respawn-list)))
