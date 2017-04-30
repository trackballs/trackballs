(set-track-name "Well done!")
(set-author "Ulrik Enstad")
(start-time 40)

(cond
 ((= (difficulty) *easy*)
         (add-goal 217 217 #f "")
 )
 ((= (difficulty) *normal*)
         (add-goal 217 217 #f "")
 )
 ((= (difficulty) *hard*)
         (add-goal 217 217 #f "frgb")
 )
)

(set-start-position 220.5 217.5)

(set-primary-color (add-flag 217 207 1000 1 0.1) 66 99 00)
(set-primary-color (add-flag 227 207 1000 1 0.1) 66 99 00)
(set-primary-color (add-flag 227 217 1000 1 0.1) 66 99 00)
(set-primary-color (add-flag 227 227 1000 1 0.1) 66 99 00)
(set-primary-color (add-flag 217 227 1000 1 0.1) 66 99 00)
(set-primary-color (add-flag 207 227 1000 1 0.1) 66 99 00)
(set-primary-color (add-flag 207 217 1000 1 0.1) 66 99 00)
(set-primary-color (add-flag 207 207 1000 1 0.1) 66 99 00)

(set-primary-color (sign "amazing! You won the entire game!" 1 20 -1 217 217) 0.5 0.0 0.5)

(jump 0.0)
