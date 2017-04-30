;;; level child5
;;; This is an easy and short level for young children.
;;; This level was created for my little goddaughter.


(day)
(rain 0.2)
(fog  0.1)

(map-is-transparent #t)

(set-track-name "Wonderland")
(set-author "Attila Boros")

;; Start
(set-start-position 240.5 248.5)
(cond
 ((= (difficulty) *easy*)   (start-time 300))
 ((= (difficulty) *normal*) (start-time 180))
 ((= (difficulty) *hard*)   (start-time  90))
)

;; Goal
(add-goal 248 241 #f "child6")

;; Do not jump
(jump 0.1)


(set-cell-wall-colors 0 0 255 255 0 .9 .7 .1 .3)
(set-cell-wall-colors 0 0 255 255 1 .9 .7 .1 .3)
(set-cell-wall-colors 0 0 255 255 2 .9 .7 .1 .3)
(set-cell-wall-colors 0 0 255 255 3 .9 .7 .1 .3)

(add-cyclic-platform 252 247 252 247 -2.0 2.0 1.0 0.5)

(define pipe1(pipe 240.8 252.5 2.5 244.2 252.5 2.5 0.5))
(set-primary-color pipe1 .3 .7 .9 .4)

(define pipe2(pipe 244.8 248.5 2.5 248.2 248.5 2.5 0.5))
(set-primary-color pipe2 .3 .7 .9 .4)

(define pipe3(pipe 248.8 252.5 2.5 252.2 252.5 2.5 0.5))
(set-primary-color pipe3 .3 .7 .9 .4)

(trigger 240.5 249.5 0.5 (lambda() (set-primary-color pipe1 .3 .7 .9 .0)))
(trigger 240.5 249.5 0.5 (lambda() (set-primary-color pipe2 .3 .7 .9 .0)))
(trigger 240.5 249.5 0.5 (lambda() (set-primary-color pipe3 .3 .7 .9 .0)))

(trigger 240.5 252.5 0.5 (lambda() (set-primary-color pipe1 .3 .7 .9 .0)))
(trigger 240.5 251.5 0.5 (lambda() (set-primary-color pipe1 .3 .7 .9 .5)))

(trigger 244.5 248.5 0.5 (lambda() (set-primary-color pipe2 .3 .7 .9 .0)))
(trigger 244.5 249.5 0.5 (lambda() (set-primary-color pipe2 .3 .7 .9 .5)))

(trigger 248.5 252.5 0.5 (lambda() (set-primary-color pipe3 .3 .7 .9 .0)))
(trigger 248.5 251.5 0.5 (lambda() (set-primary-color pipe3 .3 .7 .9 .5)))

