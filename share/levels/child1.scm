;;; level child1
;;; This is an easy and short level for young children.
;;; This level was created for my little goddaughter.


(day)

(set-track-name "Maze")
(set-author "Attila Boros")

;; Start
(set-start-position 249.5 245.5)
(cond
 ((= (difficulty) *easy*)   (start-time 240))
 ((= (difficulty) *normal*) (start-time 120))
 ((= (difficulty) *hard*)   (start-time  60))
)

;; Goal
(add-goal 238 247 #t "child2")

;; Do not jump
(jump 0.1)

(add-flag 254 242 100 1 0.1) 
(add-flag 238 254 100 1 0.1) 
(add-flag 238 242 100 1 0.1) 
(add-flag 254 254 100 1 0.1) 

