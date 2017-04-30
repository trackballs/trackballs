(set-track-name "Chess Champion")
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

(set-start-position 250.5 249.5)
(map-is-transparent #t)

(jump 0.0)
(restart-time 0)

(trigger 248.0 236.0 1.0 (lambda ()
(set-cell-heights 243 231 244 232 3.9 3.9 3.9 3.9))
)

(trigger 246.0 238.0 1.0 (lambda ()
(set-cell-heights 243 231 244 232 3.0 3.0 3.0 3.0))
)

;railing

(pipe-connector 251 237 4.7 0.08)
(pipe 251 237 4.7 249 237 4.7 0.08)
(pipe-connector 249 237 4.7 0.08)
(pipe 249 237 4.7 249 233 4.7 0.08)
(pipe-connector 249 233 4.7 0.08)

(pipe 244.9 220.2 2.8 241.0 220.2 2.8 0.08)
(pipe 244.9 220.8 2.8 241.0 220.8 2.8 0.08)

(set-primary-color (set-secondary-color (add-spike 242.9 231.1 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.9 231.5 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.9 232.0 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.9 232.5 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)
(set-primary-color (set-secondary-color (add-spike 242.9 232.9 .5 0.00) 0.5 0.5 0.5) 0.7 0.7 0.7)

(cond
 ((= (difficulty) *easy*)
         (set-primary-color (set-acceleration (new-mr-black 248.5 243.5) 1.6) 0.5 0.5 0.5)
 )
 ((= (difficulty) *normal*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 248.5 243.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
 ((= (difficulty) *hard*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 248.5 243.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
)

(cond
 ((= (difficulty) *easy*)
         (set-primary-color (set-acceleration (new-mr-black 241.5 210.5) 1.6) 0.5 0.5 0.5)
 )
 ((= (difficulty) *normal*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 241.5 210.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
 ((= (difficulty) *hard*)
         (set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 241.5 210.5) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)
 )
)





(add-cyclic-platform 246 209 246 213 1.7 4.2 1. .5)

(set-wind (pipe 246.6 211.5 2.2 247.5 211.5 2.2 0.4) 5.0 0.0)
(set-wind (pipe-connector 247.5 211.5 2.2 0.4) 5.0 0.0)
(set-wind (pipe 247.5 211.5 2.2 248.5 210.5 2.2 0.4) 5.0 0.0)
(set-wind (pipe-connector 248.5 210.5 2.2 0.4) 5.0 0.0)
(set-wind (pipe 248.5 210.5 2.2 248.5 208.6 1.2 0.4) 5.0 0.0)

(set-primary-color (set-wind (pipe 244.5 205.0 1.1 244.5 205.9 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe-connector 244.5 205.9 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe 244.5 205.9 1.1 244.5 205.8 -8.0 0.5) 20 0) 1.0 0.65 1.0)

(set-primary-color (set-wind (pipe 241.5 205.0 1.1 241.5 205.9 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe-connector 241.5 205.9 1.1 0.5) 20 0) 1.0 0.65 1.0)
(set-primary-color (set-wind (pipe 241.5 205.9 1.1 241.5 205.8 -8.0 0.5) 20 0) 1.0 0.65 1.0)

(set-primary-color (pipe-connector 247 205 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 245 205 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 247 205 0.9 245 205 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 242 205 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 244 205 0.9 242 205 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 239 203 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 241 205 0.9 239 205 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe-connector 239 205 0.9 0.08) 1.0 0.65 1.0)
(set-primary-color (pipe 239 205 0.9 239 203 0.9 0.08) 1.0 0.65 1.0)

(diamond 239.5 220.5)

(trigger 240.5 185.5 0.5 (lambda ()
(set-cell-heights 240 184 240 184 -0.2 -0.2 -0.2 -0.2)))

(trigger 240.5 184.5 0.5 (lambda ()
(set-cell-heights 240 183 240 183 -0.2 -0.2 -0.2 -0.2)))

(trigger 240.5 183.5 0.5 (lambda ()
(set-cell-heights 240 182 240 182 -0.2 -0.2 -0.2 -0.2)
(set-cell-heights 240 184 240 184 -8.0 -8.0 -8.0 -8.0)))

(trigger 240.5 182.5 0.5 (lambda ()
(set-cell-heights 240 181 240 181 -0.2 -0.2 -0.2 -0.2)
(set-cell-heights 240 183 240 183 -8.0 -8.0 -8.0 -8.0)))

(trigger 240.5 181.5 0.5 (lambda ()
(set-cell-heights 240 180 240 180 -0.2 -0.2 -0.2 -0.2)
(set-cell-heights 240 182 240 182 -8.0 -8.0 -8.0 -8.0)))

(trigger 240.5 180.5 0.5 (lambda ()
(set-cell-heights 240 181 240 181 -8.0 -8.0 -8.0 -8.0)))

(trigger 240.5 179.5 0.5 (lambda ()
(set-cell-heights 240 180 240 180 -8.0 -8.0 -8.0 -8.0)))

(set-primary-color (add-goal 235 179 #t "") 1.0 1.0 1.0 1.0)
(set-primary-color (add-goal 235 178 #t "") 1.0 1.0 1.0 1.0)

(pipe-connector 248 191 -1.5 0.08)
(pipe 248 191 -1.5 248 185 -1.5 0.08)
(pipe-connector 248 185 -1.5 0.08)
(pipe 248 185 -1.5 246 185 -1.5 0.08)
(pipe-connector 246 185 -1.5 0.08)

