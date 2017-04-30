(set-track-name "Warmup")
(set-author "Ulrik Enstad")
(set-start-position 250.5 250.5)
(map-is-transparent #t)
(add-goal 230 227 #f "frg2")
(add-goal 231 227 #f "frg2")
(restart-time 0)

(pipe 239.0 239.0 1.15 239.0 238.9 -0.5 0.4)
(pipe-connector 239.0 238.9 -0.5 0.4)
(pipe 239.0 238.9 -0.5 239.0 238.0 -1.4 0.4)
(pipe-connector 239.0 238.0 -1.4 0.4)
(pipe 239.0 238.0 -1.4 239.0 236.6 -1.4 0.4)

(add-flag 239 236 30 0 1.5)

(jump 0.0)

(cond
 ((= (difficulty) *easy*)
         (start-time 60)
 )
 ((= (difficulty) *normal*)
         (start-time 30)
 )
 ((= (difficulty) *hard*)
         (start-time 20)
 )
)


