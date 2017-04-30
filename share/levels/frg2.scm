(set-track-name "Deja Vu")
(set-author "Ulrik Enstad")

(set-start-position 250 250)
(map-is-transparent #t)

(diamond 232.5 232.5)
(restart-time 0)



(set-primary-color (pipe 226.0 225.4 -0.1 226.0 223.35 -0.1 0.5) 0.0 1.0 0.5)
(set-primary-color (pipe-connector 226.0 223.0 -0.1 0.6) 0.0 1.0 0.5)
(set-primary-color (pipe 226.0 223.0 -0.27 225.9 223.0 -3.1 0.5) 0.0 1.0 0.5)
(set-primary-color (pipe-connector 225.9 223.0 -3.1 0.6) 0.0 1.0 0.5)
(set-primary-color (pipe 225.9 223.0 -3.1 223.6 223.0 -3.1 0.5) 0.0 1.0 0.5)

(add-goal 210 218 #t "frg3")
(add-goal 210 217 #t "frg3")

(add-flag 232 232 30 0 0.5)
(add-flag 215 218 30 0 0.5)
(add-flag 215 217 50 0 0.5)

(cond
 ((= (difficulty) *easy*)
         (set-primary-color (set-acceleration (new-mr-black 219.5 223.5) 1.6) 0.5 0.5 0.5)
(set-primary-color (set-acceleration (new-mr-black 220.5 217.5) 1.6) 0.5 0.5 0.5)
(jump 0.0)
 )
 ((= (difficulty) *normal*)
         (set-primary-color (set-acceleration (new-mr-black 219.5 223.5) 1.6) 0.5 0.5 0.5)
(set-primary-color (set-acceleration (new-mr-black 220.5 217.5) 1.6) 0.5 0.5 0.5)
(jump 0.0)
 )
 ((= (difficulty) *hard*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 220.5 217.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 219.5 223.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
)

(cond
 ((= (difficulty) *easy*)
         (start-time 80)
 )
 ((= (difficulty) *normal*)
         (start-time 60)
 )
 ((= (difficulty) *hard*)
         (start-time 40)
 )
)
