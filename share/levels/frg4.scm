(set-track-name "Blue")
(set-author "Ulrik Enstad")
(set-start-position 250.5 250.5)
(map-is-transparent #t)
(add-goal 217 212 #f "frg5")
(add-goal 216 212 #f "frg5")

(fog)
(fog-color 0.0 0.5 0.8)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 235.5 239.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
(jump 0.0)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 241.5 235.5) 0.0 0.3 1.0) 0.0 0.5 0.0)6.5) 3.0)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 236.5 232.5) 0.0 0.3 1.0) 0.0 0.5 0.0)6.5) 3.0)

(set-primary-color (pipe 237.4 219.5 -1.0 235.5 219.5 -1.0 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe-connector 235.5 219.5 -1.0 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe 235.5 219.5 -1.0 234.5 219.5 -2.5 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe-connector 234.5 219.5 -2.5 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe 234.5 219.5 -2.5 232.6 219.5 -2.5 0.4) 0.3 1.0 0.0)


(set-primary-color (pipe-connector 245.0 238.1 0.3 0.08) 0.0 0.0 1.0)
(set-primary-color (pipe 245.0 238.1 0.3 253.0 238.1 0.3 0.08) 0.0 0.0 1.0)
(set-primary-color (pipe-connector 253.0 238.1 0.3 0.08) 0.0 0.0 1.0)

(add-flag 232 219 20 0 0.5)
(add-flag 221 219 30 0 1.5)

(cond
 ((= (difficulty) *easy*)
         (start-time 60)
 )
 ((= (difficulty) *normal*)
         (start-time 40)
 )
 ((= (difficulty) *hard*)
         (start-time 25)
 )
)

(restart-time 0)
