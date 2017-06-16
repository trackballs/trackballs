;;; level castle2
;;; This level was designed for experienced trackballs players.
;;; Created by Attila Boros. 2006


(night)

(set-track-name (_ "Night Watch"))
(set-author "Attila Boros")

;; Start
(set-start-position 249 250)

(define speed 0.2)
(cond
 ((= (difficulty) *easy*)   
    (start-time 600) (set! speed 0.2)
 )
 ((= (difficulty) *normal*) 
    (start-time 180) (set! speed 0.25)
 )
 ((= (difficulty) *hard*)   
    (start-time 120) (set! speed 0.3)
 )
)

;; Goal
(add-goal 229 244 #t "castle3")

;; Do not jump
(jump 0.1)

(define pipe1(pipe 241 233 -7.5 241 235 -7.5 1.0))
(set-primary-color pipe1 .9 .9 .9)
(define pipe2(pipe 233 227 -7.5 235 227 -7.5 1.0))
(set-primary-color pipe2 .9 .9 .9)
(define pipe3(pipe 226 232 -7.5 226 234 -7.5 1.0))
(set-primary-color pipe3 .9 .9 .9)

(set-cell-wall-colors 0 0 255 255 0 .9 .6 .3)
(set-cell-wall-colors 0 0 255 255 1 .9 .6 .3)
(set-cell-wall-colors 0 0 255 255 2 .9 .6 .3)
(set-cell-wall-colors 0 0 255 255 3 .9 .6 .3)

(add-spike 225 224 speed 0.9)
(add-spike 226 224 speed 0.8)
(add-spike 226 225 speed 0.7)
(add-spike 227 225 speed 0.6)
(add-spike 227 226 speed 0.5)
(add-spike 228 226 speed 0.4)
(add-spike 228 227 speed 0.3)
(add-spike 229 227 speed 0.2)
(add-spike 229 228 speed 0.1)

(add-sidespike 242 248 speed 0.9 4)
(add-sidespike 241 248 speed 0.7 4)
(add-sidespike 240 248 speed 0.5 4)
(add-sidespike 239 248 speed 0.3 4)
(add-sidespike 238 248 speed 0.1 4)

(add-sidespike 238 247 speed 0.9 4)
(add-sidespike 239 247 speed 0.7 4)
(add-sidespike 240 247 speed 0.5 4)
(add-sidespike 241 247 speed 0.3 4)
(add-sidespike 242 247 speed 0.1 4)

(add-spike 242 246 0.4 0.1)
(add-spike 242 245 0.4 0.2)
(add-spike 242 244 0.4 0.3)
(add-spike 244 246 0.4 0.1)
(add-spike 244 245 0.4 0.2)
(add-spike 244 244 0.4 0.3)
(add-spike 244 247 0.4 0.0)

;;Secret Teleport
(trigger 236 220 0.5 (lambda() (add-teleport 236 220 240 234 0.3)))

