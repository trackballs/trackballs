(set-track-name "Azure Fortress")
(set-author "Ulrik Enstad")
(set-start-position 249.5 249.5)
(rain 0.7)

(jump 0.0)
(restart-time 0)

(pipe 226.1 218.0 0.6 226.1 216.0 0.4 0.08)
(pipe-connector 226.1 216.0 0.4 0.08)
(pipe 226.1 216.0 0.4 226.1 213.0 -1.1 0.08)

(set-primary-color (add-spike 227.5 208.5 .3 0.00) 0.5 0.5 0.5)
(set-primary-color (add-spike 226.5 208.5 .3 0.00) 0.5 0.5 0.5)
(set-primary-color (add-spike 225.5 208.5 .3 0.00) 0.5 0.5 0.5)
(set-primary-color (add-spike 224.5 208.5 .3 0.00) 0.5 0.5 0.5)

(add-modpill 227.5 187.5 *mod-frozen* 3 3)
(add-modpill 226.5 187.5 *mod-frozen* 3 3)
(add-modpill 225.5 187.5 *mod-frozen* 3 3)
(add-modpill 224.5 187.5 *mod-frozen* 3 3)

(add-goal 220 180 #t "frg9")
(add-goal 220 179 #t "frg9")

(pipe 227.2 192.0 -2.6 227.2 190.0 -2.6 0.08)
(pipe 227.8 192.0 -2.6 227.8 190.0 -2.6 0.08)

(diamond 226 206)

(add-flag 234 241 30 0 0.5)
(add-flag 232 222 30 0 1.5)
(add-flag 227 216 30 0 0.5)
(add-flag 224 183 30 0 0.5)
(add-flag 227 183 30 0 0.5)

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
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 236.5 235.5) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
 ((= (difficulty) *normal*)
         (set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 236.5 235.5) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)
 )
 ((= (difficulty) *hard*)
(set-primary-color (set-acceleration (set-modtime
(new-mr-black 236.5 235.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)
 )
)
