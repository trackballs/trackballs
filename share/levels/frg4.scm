(set-track-name (_ "Blue"))
(set-author "Ulrik Enstad")
(set-start-position 250 250)
(add-goal 217 212 #f "frg5")
(add-goal 216 212 #f "frg5")

(fog)
(fog-color 0.0 0.5 0.8)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 235 239) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
(jump 0.0)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 241 235) 0.0 0.3 1.0) 0.0 0.5 0.0)6.5) 3.0)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 236 232) 0.0 0.3 1.0) 0.0 0.5 0.0)6.5) 3.0)

(set-primary-color (pipe 236.9 219.0 -1.0 235.0 219.0 -1.0 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe-connector 235.0 219.0 -1.0 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe 235.0 219.0 -1.0 234.0 219.0 -2.5 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe-connector 234.0 219.0 -2.5 0.4) 0.3 1.0 0.0)
(set-primary-color (pipe 234.0 219.0 -2.5 232.1 219.0 -2.5 0.4) 0.3 1.0 0.0)


(set-primary-color (pipe-connector 244.5 237.6 0.3 0.08) 0.0 0.0 1.0)
(set-primary-color (pipe 244.5 237.6 0.3 252.5 237.6 0.3 0.08) 0.0 0.0 1.0)
(set-primary-color (pipe-connector 252.5 237.6 0.3 0.08) 0.0 0.0 1.0)

(add-flag 232 219 20 #f 0.5)
(add-flag 221 219 30 #f 1.5)

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
