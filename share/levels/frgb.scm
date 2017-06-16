(set-track-name (_ "Chess Champion"))
(set-author "Ulrik Enstad")
(cond
 ((= (difficulty) *easy*)
         (start-time 120)
 )
 ((= (difficulty) *normal*)
         (start-time 100)
 )
 ((= (difficulty) *hard*)
         (start-time 80)
 )
)

(set-start-position 250 249)

(jump 0.0)
(restart-time 0)

(trigger 247.5 235.5 1.0 (lambda ()
(set-cell-heights 243 231 244 232 3.9 3.9 3.9 3.9))
)

(trigger 245.5 237.5 1.0 (lambda ()
(set-cell-heights 243 231 244 232 3.0 3.0 3.0 3.0))
)

;railing

(pipe-connector 250.5 236.5 4.7 0.08)
(pipe 250.5 236.5 4.7 248.5 236.5 4.7 0.08)
(pipe-connector 248.5 236.5 4.7 0.08)
(pipe 248.5 236.5 4.7 248.5 232.5 4.7 0.08)
(pipe-connector 248.5 232.5 4.7 0.08)

(pipe 244.4 219.7 2.8 240.5 219.7 2.8 0.08)
(pipe 244.4 220.3 2.8 240.5 220.3 2.8 0.08)

(set-primary-color (set-secondary-color (add-spike 242.4 230.7 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.4 231.1 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.4 231.5 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.4 231.9 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.4 232.3 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)

(cond
 ((= (difficulty) *easy*)
         (set-primary-color (set-acceleration (new-mr-black 248 243) 1.6) 0.5 0.5 0.5)
 )
 ((= (difficulty) *normal*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 248 243) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
 ((= (difficulty) *hard*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 248 243) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
)

(cond
 ((= (difficulty) *easy*)
         (set-primary-color (set-acceleration (new-mr-black 241.5 210.5) 1.6) 0.5 0.5 0.5)
 )
 ((= (difficulty) *normal*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 241 210) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
 ((= (difficulty) *hard*)
         (set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 241 210) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)
 )
)





(add-cyclic-platform 246 209 246 213 1.7 4.2 1. .5)

(set-wind (pipe 246.1 211.0 2.2 247.0 211.0 2.2 0.4) 5.0 0.0)
(set-wind (pipe-connector 247.0 211.0 2.2 0.4) 5.0 0.0)
(set-wind (pipe 247.0 211.0 2.2 248.0 210.0 2.2 0.4) 5.0 0.0)
(set-wind (pipe-connector 248.0 210.0 2.2 0.4) 5.0 0.0)
(set-wind (pipe 248.0 210.0 2.2 248.0 208.1 1.2 0.4) 5.0 0.0)

(set-primary-color (set-wind (pipe 244.0 204.5 1.1 244.0 205.4 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe-connector 244.0 205.4 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe 244.0 205.4 1.1 244.0 205.3 -8.0 0.5) 20 0) 1.0 0.65 1.0)

(set-primary-color (set-wind (pipe 241.0 204.5 1.1 241.0 205.4 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe-connector 241.0 205.4 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe 241.0 205.4 1.1 241.0 205.3 -8.0 0.5) 20 0) 1.0 0.65 1.0)

(set-primary-color (pipe-connector 246.5 204.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 244.5 204.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 246.5 204.5 0.9 244.5 204.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 241.5 204.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 243.5 204.5 0.9 241.5 204.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 238.5 202.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 240.5 204.5 0.9 238.5 204.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 238.5 204.5 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 238.5 204.5 0.9 238.5 202.5 0.9 0.08) 1.0 0.65 1.0)

(diamond 239 220)

(trigger 240 185 0.5 (lambda ()
(set-cell-heights 240 184 240 184 -0.2 -0.2 -0.2 -0.2)))

(trigger 240 184 0.5 (lambda ()
(set-cell-heights 240 183 240 183 -0.2 -0.2 -0.2 -0.2)))

(trigger 240 183 0.5 (lambda ()
(set-cell-heights 240 182 240 182 -0.2 -0.2 -0.2 -0.2)
(set-cell-heights 240 184 240 184 -8.0 -8.0 -8.0 -8.0)))

(trigger 240 182 0.5 (lambda ()
(set-cell-heights 240 181 240 181 -0.2 -0.2 -0.2 -0.2)
(set-cell-heights 240 183 240 183 -8.0 -8.0 -8.0 -8.0)))

(trigger 240 181 0.5 (lambda ()
(set-cell-heights 240 180 240 180 -0.2 -0.2 -0.2 -0.2)
(set-cell-heights 240 182 240 182 -8.0 -8.0 -8.0 -8.0)))

(trigger 240 180 0.5 (lambda ()
(set-cell-heights 240 181 240 181 -8.0 -8.0 -8.0 -8.0)))

(trigger 240 179 0.5 (lambda ()
(set-cell-heights 240 180 240 180 -8.0 -8.0 -8.0 -8.0)))

(set-primary-color (add-goal 235 179 #t "") 1.0 1.0 1.0 1.0)
(set-primary-color (add-goal 235 178 #t "") 1.0 1.0 1.0 1.0)

(pipe-connector 247.5 190.5 -1.5 0.08)
(pipe 247.5 190.5 -1.5 247.5 184.5 -1.5 0.08)
(pipe-connector 247.5 184.5 -1.5 0.08)
(pipe 247.5 184.5 -1.5 245.5 184.5 -1.5 0.08)
(pipe-connector 245.5 184.5 -1.5 0.08)

