(set-track-name (_ "Azure Fortress"))
(set-author "Ulrik Enstad")
(set-start-position 249 249)
(rain 0.7)

(jump 0.0)
(restart-time 0)

(pipe 225.6 217.5 0.6 225.6 215.5 0.4 0.08)
(pipe-connector 225.6 215.5 0.4 0.08)
(pipe 225.6 215.5 0.4 225.6 212.5 -1.1 0.08)

(set-primary-color (add-spike 227 208 .3 0.00) 0.5 0.5 0.5)
(set-primary-color (add-spike 226 208 .3 0.00) 0.5 0.5 0.5)
(set-primary-color (add-spike 225 208 .3 0.00) 0.5 0.5 0.5)
(set-primary-color (add-spike 224 208 .3 0.00) 0.5 0.5 0.5)

(add-modpill 227 187 *mod-frozen* 3 3)
(add-modpill 226 187 *mod-frozen* 3 3)
(add-modpill 225 187 *mod-frozen* 3 3)
(add-modpill 224 187 *mod-frozen* 3 3)

(add-goal 220 180 #t "frg9")
(add-goal 220 179 #t "frg9")

(pipe 226.7 191.5 -2.6 226.7 189.5 -2.6 0.08)
(pipe 227.3 191.5 -2.6 227.3 189.5 -2.6 0.08)

(diamond 225.5 205.5)

(add-flag 234 241 30 #f 0.5)
(add-flag 232 222 30 #f 1.5)
(add-flag 227 216 30 #f 0.5)
(add-flag 224 183 30 #f 0.5)
(add-flag 227 183 30 #f 0.5)

(cond
 ((= (difficulty) *easy*)
         (start-time 120)
 )
 ((= (difficulty) *normal*)
         (start-time 80)
 )
 ((= (difficulty) *hard*)
         (start-time 70)
 )
)

(cond
 ((= (difficulty) *easy*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 236 235) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
 ((= (difficulty) *normal*)
         (set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 236 235) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)
 )
 ((= (difficulty) *hard*)
(set-primary-color (set-acceleration (set-modtime
(new-mr-black 236 235) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)
 )
)
