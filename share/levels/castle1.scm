;;; level castle1
;;; This level was designed for experienced trackballs players.
;;; Created by Attila Boros. 2006


(day)
(fog 0.5)

(set-track-name "Blue Castle")
(set-author "Attila Boros")

;; Start
(set-start-position 232.5 238.5)

(cond
 ((= (difficulty) *easy*)   
    (start-time 240)
 )
 ((= (difficulty) *normal*) 
    (start-time 180) 
 )
 ((= (difficulty) *hard*)   
    (start-time 120)
 )
)

;; Goal
(add-goal 247 246 #t "castle2")

;; Do not jump
(jump 0.1)

(add-flag 254 254 20 1 0.1) 
(add-flag 229 229 20 1 0.1) 

(add-flag 230 240 10 1 0.1) 
(add-flag 230 243 10 1 0.1) 
(add-flag 233 240 10 1 0.1) 
(add-flag 233 243 10 1 0.1) 

(set-cell-wall-colors 0 0 255 255 0 .3 .3 .9)
(set-cell-wall-colors 0 0 255 255 1 .3 .3 .9)
(set-cell-wall-colors 0 0 255 255 2 .3 .3 .9)
(set-cell-wall-colors 0 0 255 255 3 .3 .3 .9)

;;Secret treasure
(trigger 241 242 1.0 (lambda() (set-cell-heights 240 241 241 242 -1 -1 -1 -1)))
(add-flag 240 241 50 1 0.1) 
(add-flag 240 242 50 1 0.1) 
(add-flag 241 241 50 1 0.1) 
(add-flag 241 242 50 1 0.1) 

