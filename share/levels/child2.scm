;;; level child2
;;; This is an easy and short level for young children.
;;; This level was created for my little goddaughter.


(day)
(snow 0.5)

(set-track-name "Christmas")
(set-author "Attila Boros")

;; Start
(set-start-position 253.5 250.5)
(cond
 ((= (difficulty) *easy*)   (start-time 180))
 ((= (difficulty) *normal*) (start-time 120))
 ((= (difficulty) *hard*)   (start-time  60))
)

;; Goal
(add-goal 250 246 #t "child3")

;; Do not jump
(jump 0.1)


(add-cyclic-platform 248 250 248 250 -4.0 -2.5 2.0 0.5)

(add-flag 248 252 50 1 0.1) 
(add-flag 248 246 50 1 0.1) 

