
(set-track-name (_ "Jump'n Roll"))
(set-author "Ulrik Enstad")
(set-start-position 248 250)
(night)

(jump 0.0)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 242 233) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)

(add-cyclic-platform 235 229 236 234 1.8 3.8 1. .5)
(add-cyclic-platform 233 229 234 234 1.8 3.8 5. .3)
(add-cyclic-platform 231 229 232 234 1.8 3.8 3. .6)

(set-primary-color (pipe 219.5 225.9 2.3 219.5 219.1 1.8 0.4) 0.0 1.0 0.5)

(set-acceleration (set-primary-color (set-modtime (new-mr-black 206.5 211.5) *mod-spike* -1.) 0.8 0.0 0.0) 2.3)

(diamond 219.5 228.5)

(add-goal 205 206 #t "frg8")
(add-goal 205 205 #t "frg8")

(add-flag 248 238 30 #f 0.5)
(add-flag 220 219 30 #f 0.5)
(add-flag 206 216 30 #f 1.5)

(set-primary-color (set-secondary-color (set-acceleration (new-mr-black 212 216) 3.5) 0.0 0.3 1.0) 0.0 0.5 0.0)

(cond
 ((= (difficulty) *easy*)
         (start-time 90)
 )
 ((= (difficulty) *normal*)
         (start-time 75)
 )
 ((= (difficulty) *hard*)
         (start-time 65)
 )
)

(restart-time 0)

