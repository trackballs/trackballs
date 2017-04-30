;;; level child3
;;; This is an easy and short level for young children.
;;; This level was created for my little goddaughter.


(day)
(fog 0.3)

(set-track-name "Pinky")
(set-author "Attila Boros")

;; Start
(set-start-position 254.5 252.5)
(start-time 300) 
(cond
 ((= (difficulty) *easy*)   (start-time 300))
 ((= (difficulty) *normal*) (start-time 180))
 ((= (difficulty) *hard*)   (start-time 120))
)

;; Goal
(add-goal 243 252 #f "child4")

;; Do not jump
(jump 0.1)


(add-cyclic-platform 245 251 245 251 -2.0 -0.5 2.0 0.6)
(add-cyclic-platform 245 245 245 245 -3.5 -2.0 0.5 0.5)

(add-flag 250 248 50 1 0.1) 
(add-flag 250 249 50 1 0.1) 
(add-flag 250 250 50 1 0.1) 

