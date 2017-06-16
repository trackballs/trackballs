(set-track-name (_ "Warmup"))
(set-author "Ulrik Enstad")
(set-start-position 250 250)
(add-goal 230 227 #f "frg2")
(add-goal 231 227 #f "frg2")
(restart-time 0)

(pipe 238.5 238.5 1.15 238.5 238.4 -0.5 0.4)
(pipe-connector 238.5 238.4 -0.5 0.4)
(pipe 238.5 238.4 -0.5 238.5 237.5 -1.4 0.4)
(pipe-connector 238.5 237.5 -1.4 0.4)
(pipe 238.5 237.5 -1.4 238.5 236.1 -1.4 0.4)

(add-flag 239 236 30 #f 1.5)

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


